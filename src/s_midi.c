
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MIDI_NOTEOFF                    0x80
#define MIDI_NOTEON                     0x90
#define MIDI_POLYPRESSURE               0xa0
#define MIDI_CONTROLCHANGE              0xb0
#define MIDI_PROGRAMCHANGE              0xc0
#define MIDI_AFTERTOUCH                 0xd0
#define MIDI_PITCHBEND                  0xe0

#define MIDI_STARTSYSEX                 0xf0
#define MIDI_TIMECODE                   0xf1
#define MIDI_SONGPOS                    0xf2
#define MIDI_SONGSELECT                 0xf3
#define MIDI_RESERVED1                  0xf4
#define MIDI_RESERVED2                  0xf5
#define MIDI_TUNEREQUEST                0xf6
#define MIDI_ENDSYSEX                   0xf7
#define MIDI_CLOCK                      0xf8
#define MIDI_TICK                       0xf9
#define MIDI_START                      0xfa
#define MIDI_CONTINUE                   0xfb
#define MIDI_STOP                       0xfc
#define MIDI_ACTIVESENSE                0xfe
#define MIDI_RESET                      0xff

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MIDI_QUEUE_SIZE                 1024
#define MIDI_UNDEFINED_OFFSET           (-(DBL_MAX))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _midiqelem {
    double          q_time;
    int             q_portNumber;
    unsigned char   q_hasOneByte;
    unsigned char   q_byte1;
    unsigned char   q_byte2;
    unsigned char   q_byte3;
    } t_midiqelem;

typedef struct _midiparser {
    int mp_status;
    int mp_gotByte1;
    int mp_byte1;
    } t_midiparser;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int midi_api;
extern int sys_schedadvance;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_midiqelem  midi_outQueue[MIDI_QUEUE_SIZE];                         /* Shared. */
static int          midi_outHead;                                           /* Shared. */
static int          midi_outTail;                                           /* Shared. */

static t_midiqelem  midi_inQueue[MIDI_QUEUE_SIZE];                          /* Shared. */
static int          midi_inHead;                                            /* Shared. */
static int          midi_inTail;                                            /* Shared. */

static double       midi_realTimeAtStart;                                   /* Shared. */
static double       midi_logicalTimeAtStart;                                /* Shared. */
static double       midi_dacOffset;                                         /* Shared. */
static double       midi_adcOffset;                                         /* Shared. */
static double       midi_dacNewOffset = MIDI_UNDEFINED_OFFSET;              /* Shared. */
static double       midi_adcNewOffset = MIDI_UNDEFINED_OFFSET;              /* Shared. */
static double       midi_needToUpdateTime;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_initialize (void)
{
    midi_realTimeAtStart    = sys_getRealTimeInSeconds();
    midi_logicalTimeAtStart = scheduler_getLogicalTime();
    midi_dacOffset          = 0.0;
    midi_adcOffset          = 0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_setOffsets (void)
{
    double realLapse    = sys_getRealTimeInSeconds();
    double logicalLapse = MILLISECONDS_TO_SECONDS (scheduler_getMillisecondsSince (midi_logicalTimeAtStart));
    
    double dacOffset    = logicalLapse - realLapse - MICROSECONDS_TO_SECONDS (sys_schedadvance);
    double adcOffset    = logicalLapse - realLapse;
    
    if (dacOffset > midi_dacNewOffset) { midi_dacNewOffset = dacOffset; }
    if (adcOffset > midi_adcNewOffset) { midi_adcNewOffset = adcOffset; }
        
    if (realLapse > midi_needToUpdateTime) {
    //
    midi_dacOffset    = midi_dacNewOffset;
    midi_adcOffset    = midi_adcNewOffset;
    midi_dacNewOffset = MIDI_UNDEFINED_OFFSET;
    midi_adcNewOffset = MIDI_UNDEFINED_OFFSET;
    
    midi_needToUpdateTime = realLapse + 1.0;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static double midi_getTimeIn (void)
{
    return (sys_getRealTimeInSeconds() + midi_adcOffset);
}

static double midi_getTimeOut (void)
{
    return (sys_getRealTimeInSeconds() + midi_dacOffset);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void midi_pushNextByte (int port, int a)
{
    if (API_WITH_ALSA && midi_api == API_ALSA) { sys_alsa_putmidibyte (port, a); }
    else {
        sys_putmidibyte (port, a);
    }
}

static void midi_pushNextMessage (int port, int a, int b, int c)
{
    if (API_WITH_ALSA && midi_api == API_ALSA) { sys_alsa_putmidimess (port, a, b, c); }
    else {
        sys_putmidimess (port, a, b, c);
    }
}

static void midi_pushNext (void)
{
    t_midiqelem *e = midi_outQueue + midi_outTail;
        
    if (e->q_hasOneByte) { midi_pushNextByte (e->q_portNumber, e->q_byte1); }
    else {
        midi_pushNextMessage (e->q_portNumber, e->q_byte1, e->q_byte2, e->q_byte3);
    }   
    
    midi_outTail = (midi_outTail + 1 == MIDI_QUEUE_SIZE ? 0 : midi_outTail + 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void midi_pollOut (void)
{
    double t = midi_getTimeOut();

    while (midi_outHead != midi_outTail) {
        if (midi_outQueue[midi_outTail].q_time <= t) { midi_pushNext(); }
        else { 
            break;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void outmidi_append (int portno, int onebyte, int a, int b, int c)
{
    t_midiqelem *midiqelem;
    int newhead = midi_outHead +1;
    if (newhead == MIDI_QUEUE_SIZE)
        newhead = 0;
            /* if FIFO is full flush an element to make room */
    if (newhead == midi_outTail)
        midi_pushNext();
    midi_outQueue[midi_outHead].q_portNumber = portno;
    midi_outQueue[midi_outHead].q_hasOneByte = onebyte;
    midi_outQueue[midi_outHead].q_byte1 = a;
    midi_outQueue[midi_outHead].q_byte2 = b;
    midi_outQueue[midi_outHead].q_byte3 = c;
    midi_outQueue[midi_outHead].q_time =
        .001 * scheduler_getMillisecondsSince(midi_logicalTimeAtStart);
    midi_outHead = newhead;
    midi_pollOut();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void outmidi_noteon(int portno, int channel, int pitch, int velo)
{
    if (pitch < 0) pitch = 0;
    else if (pitch > 127) pitch = 127;
    if (velo < 0) velo = 0;
    else if (velo > 127) velo = 127;
    outmidi_append(portno, 0, MIDI_NOTEON + (channel & 0xf), pitch, velo);
}

void outmidi_controlchange(int portno, int channel, int ctl, int value)
{
    if (ctl < 0) ctl = 0;
    else if (ctl > 127) ctl = 127;
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    outmidi_append(portno, 0, MIDI_CONTROLCHANGE + (channel & 0xf),
        ctl, value);
}

void outmidi_programchange(int portno, int channel, int value)
{
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    outmidi_append(portno, 0,
        MIDI_PROGRAMCHANGE + (channel & 0xf), value, 0);
}

void outmidi_pitchbend(int portno, int channel, int value)
{
    if (value < 0) value = 0;
    else if (value > 16383) value = 16383;
    outmidi_append(portno, 0, MIDI_PITCHBEND + (channel & 0xf),
        (value & 127), ((value>>7) & 127));
}

void outmidi_aftertouch(int portno, int channel, int value)
{
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    outmidi_append(portno, 0, MIDI_AFTERTOUCH + (channel & 0xf), value, 0);
}

void outmidi_polyaftertouch(int portno, int channel, int pitch, int value)
{
    if (pitch < 0) pitch = 0;
    else if (pitch > 127) pitch = 127;
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    outmidi_append(portno, 0, MIDI_POLYPRESSURE + (channel & 0xf),
        pitch, value);
}

void outmidi_mclk(int portno)
{
   outmidi_append(portno, 1, 0xf8, 0,0);
}

void outmidi_byte(int portno, int value)
{
#ifdef USEAPI_ALSA
  if (midi_api == API_ALSA)
    {
      sys_alsa_putmidibyte(portno, value);
    }
  else
#endif
    {
      sys_putmidibyte(portno, value);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void sys_dispatchnextmidiin( void)
{
    static t_midiparser parser[MAXIMUM_MIDI_IN], *parserp;
    int portno = midi_inQueue[midi_inTail].q_portNumber,
        byte = midi_inQueue[midi_inTail].q_byte1;
    if (!midi_inQueue[midi_inTail].q_hasOneByte) { PD_BUG; }
    if (portno < 0 || portno >= MAXIMUM_MIDI_IN) { PD_BUG; }
    parserp = parser + portno;
    
    if (byte >= 0xf8)
        inmidi_realtimein(portno, byte);
    else
    {
        inmidi_byte(portno, byte);
        if (byte & 0x80)
        {
            if (byte == MIDI_TUNEREQUEST || byte == MIDI_RESERVED1 ||
                byte == MIDI_RESERVED2)
                    parserp->mp_status = 0;
            else if (byte == MIDI_STARTSYSEX)
            {
                inmidi_sysex(portno, byte);
                parserp->mp_status = byte;
            }
            else if (byte == MIDI_ENDSYSEX)
            {
                inmidi_sysex(portno, byte);
                parserp->mp_status = 0;
            }
            else
            {
                parserp->mp_status = byte;
            }
            parserp->mp_gotByte1 = 0;
        }
        else
        {
            int cmd = (parserp->mp_status >= 0xf0 ? parserp->mp_status :
                (parserp->mp_status & 0xf0));
            int chan = (parserp->mp_status & 0xf);
            int byte1 = parserp->mp_byte1, gotbyte1 = parserp->mp_gotByte1;
            switch (cmd)
            {
            case MIDI_NOTEOFF:
                if (gotbyte1)
                    inmidi_noteon(portno, chan, byte1, 0),
                        parserp->mp_gotByte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotByte1 = 1;
                break;
            case MIDI_NOTEON:
                if (gotbyte1)
                    inmidi_noteon(portno, chan, byte1, byte),
                        parserp->mp_gotByte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotByte1 = 1;
                break;
            case MIDI_POLYPRESSURE:
                if (gotbyte1)
                    inmidi_polyaftertouch(portno, chan, byte1, byte),
                        parserp->mp_gotByte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotByte1 = 1;
                break;
            case MIDI_CONTROLCHANGE:
                if (gotbyte1)
                    inmidi_controlchange(portno, chan, byte1, byte),
                        parserp->mp_gotByte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotByte1 = 1;
                break;
            case MIDI_PROGRAMCHANGE:
                inmidi_programchange(portno, chan, byte);
                break;
            case MIDI_AFTERTOUCH:
                inmidi_aftertouch(portno, chan, byte);
                break;
            case MIDI_PITCHBEND:
                if (gotbyte1)
                    inmidi_pitchbend(portno, chan, ((byte << 7) + byte1)),
                        parserp->mp_gotByte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotByte1 = 1;
                break;
            case MIDI_STARTSYSEX:
                inmidi_sysex(portno, byte);
                break;
                
                /* other kinds of messages are just dropped here.  We'll
                need another status byte before we start letting MIDI in
                again (no running status across "system" messages). */
            case MIDI_TIMECODE:     /* 1 data byte*/
                break;
            case MIDI_SONGPOS:       /* 2 */
                break;
            case MIDI_SONGSELECT:    /* 1 */
                break;
            }
        }
    }  
    midi_inTail  = (midi_inTail + 1 == MIDI_QUEUE_SIZE ? 0 : midi_inTail + 1);
}

void sys_pollmidiinqueue( void)
{
    double logicaltime = .001 * scheduler_getMillisecondsSince(midi_logicalTimeAtStart);

    while (midi_inHead != midi_inTail)
    {
        if (midi_inQueue[midi_inTail].q_time <= logicaltime)
        {
            sys_dispatchnextmidiin();
        }
        else break;
    }
}

    /* this should be called from the system dependent MIDI code when a byte
    comes in, as a result of our calling sys_poll_midi.  We stick it on a
    timetag queue and dispatch it at the appropriate logical time. */


void sys_midibytein(int portno, int byte)
{
    static int warned = 0;
    t_midiqelem *midiqelem;
    int newhead = midi_inHead +1;
    if (newhead == MIDI_QUEUE_SIZE)
        newhead = 0;
            /* if FIFO is full flush an element to make room */
    if (newhead == midi_inTail)
    {
        if (!warned)
        {
            post("warning: MIDI timing FIFO overflowed");
            warned = 1;
        }
        sys_dispatchnextmidiin();
    }
    midi_inQueue[midi_inHead].q_portNumber = portno;
    midi_inQueue[midi_inHead].q_hasOneByte = 1;
    midi_inQueue[midi_inHead].q_byte1 = byte;
    midi_inQueue[midi_inHead].q_time = midi_getTimeIn();
    midi_inHead = newhead;
    sys_pollmidiinqueue();
}

void sys_pollmidiqueue( void)
{
#if 0
    static double lasttime;
    double newtime = sys_getRealTimeInSeconds();
    if (newtime - lasttime > 0.007)
        post("delay %d", (int)(1000 * (newtime - lasttime)));
    lasttime = newtime;
#endif
#ifdef USEAPI_ALSA
      if (midi_api == API_ALSA)
        sys_alsa_poll_midi();
      else
#endif /* ALSA */
    sys_poll_midi();    /* OS dependent poll for MIDI input */
    midi_pollOut();
    sys_pollmidiinqueue();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
