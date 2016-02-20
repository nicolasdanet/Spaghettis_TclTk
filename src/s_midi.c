
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

static t_midiparser midi_parser[MAXIMUM_MIDI_IN];                           /* Shared. */
    
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

void midi_synchronise (void)
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

static void midi_dispatchNext (void)
{
    int port = midi_inQueue[midi_inTail].q_portNumber;
    int byte = midi_inQueue[midi_inTail].q_byte1;
    t_midiparser *p = midi_parser + port;
        
    PD_ASSERT (midi_inQueue[midi_inTail].q_hasOneByte);
    PD_ASSERT (port >= 0 || port < MAXIMUM_MIDI_IN);
    
    if (byte >= MIDI_CLOCK) { inmidi_realtimein (port, byte); }
    else {
    //
    inmidi_byte (port, byte);
    
    if (byte & 0x80) {
    
        if (byte == MIDI_TUNEREQUEST || byte == MIDI_RESERVED1 || byte == MIDI_RESERVED2) {
            p->mp_status = 0;
        } else if (byte == MIDI_STARTSYSEX) {
            inmidi_sysex (port, byte); p->mp_status = byte;
        } else if (byte == MIDI_ENDSYSEX) {
            inmidi_sysex (port, byte); p->mp_status = 0;
        } else {
            p->mp_status = byte;
        }
        
        p->mp_gotByte1 = 0;
        
    } else {
    
        int command  = (p->mp_status >= MIDI_STARTSYSEX ? p->mp_status : (p->mp_status & 0xf0));
        int channel  = (p->mp_status & 0xf);
        int byte1    = p->mp_byte1;
        
        switch (command) {
        //
        case MIDI_NOTEOFF       :   if (p->mp_gotByte1) { 
                                        inmidi_noteon (port, channel, byte1, 0);
                                        p->mp_gotByte1 = 0; 
                                    } else {
                                        p->mp_byte1 = byte;
                                        p->mp_gotByte1 = 1;
                                    }
                                    break;
            
        case MIDI_NOTEON        :   if (p->mp_gotByte1) { 
                                        inmidi_noteon (port, channel, byte1, byte);
                                        p->mp_gotByte1 = 0; 
                                    } else {
                                        p->mp_byte1 = byte;
                                        p->mp_gotByte1 = 1;
                                    }
                                    break;
            
        case MIDI_POLYPRESSURE  :   if (p->mp_gotByte1) {
                                        inmidi_polypressure (port, channel, byte1, byte);
                                        p->mp_gotByte1 = 0;
                                    } else {
                                        p->mp_byte1 = byte;
                                        p->mp_gotByte1 = 1;
                                    }
                                    break;
                                    
        case MIDI_CONTROLCHANGE :   if (p->mp_gotByte1) {
                                        inmidi_controlchange (port, channel, byte1, byte);
                                        p->mp_gotByte1 = 0;
                                    } else {
                                        p->mp_byte1 = byte;
                                        p->mp_gotByte1 = 1;
                                    }
                                    break;
                                    
        case MIDI_PROGRAMCHANGE :   inmidi_programchange (port, channel, byte);
                                    break;
                                    
        case MIDI_AFTERTOUCH    :   inmidi_aftertouch (port, channel, byte);
                                    break;
                                    
        case MIDI_PITCHBEND     :   if (p->mp_gotByte1) {
                                        inmidi_pitchbend (port, channel, ((byte << 7) + byte1)); 
                                        p->mp_gotByte1 = 0;
                                    } else {
                                        p->mp_byte1 = byte;
                                        p->mp_gotByte1 = 1;
                                    }
                                    break;
                                    
        case MIDI_STARTSYSEX    :   inmidi_sysex (port, byte);
                                    break;
            
        default                 :   break;
        //
        }
    }
    //
    }
    
    midi_inTail = (midi_inTail + 1 == MIDI_QUEUE_SIZE ? 0 : midi_inTail + 1);
}

static void midi_pollIn (void)
{
    double logicalTime = MILLISECONDS_TO_SECONDS (scheduler_getMillisecondsSince (midi_logicalTimeAtStart));

    while (midi_inHead != midi_inTail) {
    //
    if (midi_inQueue[midi_inTail].q_time <= logicalTime) { midi_dispatchNext(); }
    else { 
        break;
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_pollInOut (void)
{
    if (API_WITH_ALSA && midi_api == API_ALSA) { sys_alsa_poll_midi(); }
    else {
        sys_poll_midi();
    }
    
    midi_pollOut();
    midi_pollIn();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_send (int port, int byte)
{
    midi_pushNextByte (port, byte);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* If FIFO is full dispatch or flush an element to make room. */

void midi_receive (int port, int byte)
{
    t_midiqelem *e = NULL;
    int newHead = (midi_inHead + 1 == MIDI_QUEUE_SIZE ? 0 : midi_inHead + 1);
    if (newHead == midi_inTail) { midi_dispatchNext(); }
    
    e = midi_inQueue + midi_inHead;
    
    e->q_portNumber = port;
    e->q_hasOneByte = 1;
    e->q_byte1      = byte;
    e->q_time       = midi_getTimeIn();
    
    midi_inHead = newHead;
    
    midi_pollIn();
}

void midi_broadcast (int port, int hasOneByte, int a, int b, int c)
{
    t_midiqelem *e = NULL;
    int newHead = (midi_outHead + 1 == MIDI_QUEUE_SIZE ? 0 : midi_outHead + 1);
    if (newHead == midi_outTail) { midi_pushNext(); }
    
    e = midi_outQueue + midi_outHead;
    
    e->q_portNumber = port;
    e->q_hasOneByte = hasOneByte;
    e->q_byte1      = a;
    e->q_byte2      = b;
    e->q_byte3      = c;
    e->q_time       = MILLISECONDS_TO_SECONDS (scheduler_getMillisecondsSince (midi_logicalTimeAtStart));
        
    midi_outHead = newHead;
    
    midi_pollOut();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void outmidi_noteOn (int port, int channel, int pitch, int velocity)
{
    pitch    = PD_CLAMP (pitch, 0, 127);
    velocity = PD_CLAMP (velocity, 0, 127);

    midi_broadcast (port, 0, MIDI_NOTEON + (channel & 0xf), pitch, velocity);
}

void outmidi_controlChange (int port, int channel, int control, int value)
{
    control = PD_CLAMP (control, 0, 127);
    value   = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_CONTROLCHANGE + (channel & 0xf), control, value);
}

void outmidi_programChange (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_PROGRAMCHANGE + (channel & 0xf), value, 0);
}

void outmidi_pitchBend (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 16383);     // 0x3fff 
    
    midi_broadcast (port, 0, MIDI_PITCHBEND + (channel & 0xf), (value & 127), ((value >> 7) & 127));
}

void outmidi_afterTouch (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_AFTERTOUCH + (channel & 0xf), value, 0);
}

void outmidi_polyPressure (int port, int channel, int pitch, int value)
{
    pitch = PD_CLAMP (pitch, 0, 127);
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_POLYPRESSURE + (channel & 0xf), pitch, value);
}

void outmidi_clock (int port)
{
    midi_broadcast (port, 1, MIDI_CLOCK, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
