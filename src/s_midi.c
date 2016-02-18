
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

#define MIDI_QUEUE_SIZE     1024

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_midiqelem  midi_outQueue[MIDI_QUEUE_SIZE];                         /* Shared. */
static int          midi_outHead;                                           /* Shared. */
static int          midi_outTail;                                           /* Shared. */

static t_midiqelem  midi_inQueue[MIDI_QUEUE_SIZE];                          /* Shared. */
static int          midi_inHead;                                            /* Shared. */
static int          midi_inTail;                                            /* Shared. */

static int          midi_api                        = API_DEFAULT_MIDI;     /* Shared. */
static double       midi_systimeAtStart;                                    /* Shared. */
static double       midi_dacTimeMinusRealTime;                              /* Shared. */
static double       midi_adcTimeMinusRealTime;                              /* Shared. */
static double       midi_newDacTimeMinusRealTime    = -1e20;                /* Shared. */
static double       midi_newAdcTimeMinusRealTime    = -1e20;                /* Shared. */
static double       midi_needToUpdateTime;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_initialize (void)
{
    midi_systimeAtStart         = scheduler_getSystime();
    midi_dacTimeMinusRealTime   = 0.0;
    midi_adcTimeMinusRealTime   = 0.0;
}

    /* this is called from the OS dependent code from time to time when we
    think we know the delay (outbuftime) in seconds, at which the last-output
    audio sample will go out the door. */
    
void sys_setmiditimediff (double inbuftime, double outbuftime)
{
    double dactimeminusrealtime =
        .001 * scheduler_getMillisecondsSince(midi_systimeAtStart)
            - outbuftime - sys_getRealTime();
    double adctimeminusrealtime =
        .001 * scheduler_getMillisecondsSince(midi_systimeAtStart)
            + inbuftime - sys_getRealTime();
    if (dactimeminusrealtime > midi_newDacTimeMinusRealTime)
        midi_newDacTimeMinusRealTime = dactimeminusrealtime;
    if (adctimeminusrealtime > midi_newAdcTimeMinusRealTime)
        midi_newAdcTimeMinusRealTime = adctimeminusrealtime;
    if (sys_getRealTime() > midi_needToUpdateTime)
    {
        midi_dacTimeMinusRealTime = midi_newDacTimeMinusRealTime;
        midi_adcTimeMinusRealTime = midi_newAdcTimeMinusRealTime;
        midi_newDacTimeMinusRealTime = -1e20;
        midi_newAdcTimeMinusRealTime = -1e20;
        midi_needToUpdateTime = sys_getRealTime() + 1;
    }
}

    /* return the logical time of the DAC sample we believe is currently
    going out, based on how much "system time" has elapsed since the
    last time sys_setmiditimediff got called. */
static double sys_getmidioutrealtime( void)
{
    return (sys_getRealTime() + midi_dacTimeMinusRealTime);
}

static double sys_getmidiinrealtime( void)
{
    return (sys_getRealTime() + midi_adcTimeMinusRealTime);
}

static void sys_putnext( void)
{
    int portno = midi_outQueue[midi_outTail].q_portNumber;
#ifdef USEAPI_ALSA
    if (midi_api == API_ALSA)
      {
        if (midi_outQueue[midi_outTail].q_hasOneByte)
          sys_alsa_putmidibyte(portno, midi_outQueue[midi_outTail].q_byte1);
        else sys_alsa_putmidimess(portno, midi_outQueue[midi_outTail].q_byte1,
                             midi_outQueue[midi_outTail].q_byte2,
                             midi_outQueue[midi_outTail].q_byte3);
      }
    else
#endif /* ALSA */
      {
        if (midi_outQueue[midi_outTail].q_hasOneByte)
          sys_putmidibyte(portno, midi_outQueue[midi_outTail].q_byte1);
        else sys_putmidimess(portno, midi_outQueue[midi_outTail].q_byte1,
                             midi_outQueue[midi_outTail].q_byte2,
                             midi_outQueue[midi_outTail].q_byte3);
      }
    midi_outTail  = (midi_outTail + 1 == MIDI_QUEUE_SIZE ? 0 : midi_outTail + 1);
}

/*  #define TEST_DEJITTER */

void sys_pollmidioutqueue( void)
{
#ifdef TEST_DEJITTER
    static int db = 0;
#endif
    double midirealtime = sys_getmidioutrealtime();
#ifdef TEST_DEJITTER
    if (midi_outHead == midi_outTail)
        db = 0;
#endif
    while (midi_outHead != midi_outTail)
    {
#ifdef TEST_DEJITTER
        if (!db)
        {
            post("out: del %f, midiRT %f logicaltime %f, RT %f dacminusRT %f",
                (midi_outQueue[midi_outTail].q_time - midirealtime),
                    midirealtime, .001 * scheduler_getMillisecondsSince(midi_systimeAtStart),
                        sys_getRealTime(), midi_dacTimeMinusRealTime);
            db = 1;
        }
#endif
        if (midi_outQueue[midi_outTail].q_time <= midirealtime)
            sys_putnext();
        else break;
    }
}

static void sys_queuemidimess(int portno, int onebyte, int a, int b, int c)
{
    t_midiqelem *midiqelem;
    int newhead = midi_outHead +1;
    if (newhead == MIDI_QUEUE_SIZE)
        newhead = 0;
            /* if FIFO is full flush an element to make room */
    if (newhead == midi_outTail)
        sys_putnext();
    midi_outQueue[midi_outHead].q_portNumber = portno;
    midi_outQueue[midi_outHead].q_hasOneByte = onebyte;
    midi_outQueue[midi_outHead].q_byte1 = a;
    midi_outQueue[midi_outHead].q_byte2 = b;
    midi_outQueue[midi_outHead].q_byte3 = c;
    midi_outQueue[midi_outHead].q_time =
        .001 * scheduler_getMillisecondsSince(midi_systimeAtStart);
    midi_outHead = newhead;
    sys_pollmidioutqueue();
}

#define MIDI_NOTEON 144
#define MIDI_POLYAFTERTOUCH 160
#define MIDI_CONTROLCHANGE 176
#define MIDI_PROGRAMCHANGE 192
#define MIDI_AFTERTOUCH 208
#define MIDI_PITCHBEND 224

void outmidi_noteon(int portno, int channel, int pitch, int velo)
{
    if (pitch < 0) pitch = 0;
    else if (pitch > 127) pitch = 127;
    if (velo < 0) velo = 0;
    else if (velo > 127) velo = 127;
    sys_queuemidimess(portno, 0, MIDI_NOTEON + (channel & 0xf), pitch, velo);
}

void outmidi_controlchange(int portno, int channel, int ctl, int value)
{
    if (ctl < 0) ctl = 0;
    else if (ctl > 127) ctl = 127;
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    sys_queuemidimess(portno, 0, MIDI_CONTROLCHANGE + (channel & 0xf),
        ctl, value);
}

void outmidi_programchange(int portno, int channel, int value)
{
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    sys_queuemidimess(portno, 0,
        MIDI_PROGRAMCHANGE + (channel & 0xf), value, 0);
}

void outmidi_pitchbend(int portno, int channel, int value)
{
    if (value < 0) value = 0;
    else if (value > 16383) value = 16383;
    sys_queuemidimess(portno, 0, MIDI_PITCHBEND + (channel & 0xf),
        (value & 127), ((value>>7) & 127));
}

void outmidi_aftertouch(int portno, int channel, int value)
{
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    sys_queuemidimess(portno, 0, MIDI_AFTERTOUCH + (channel & 0xf), value, 0);
}

void outmidi_polyaftertouch(int portno, int channel, int pitch, int value)
{
    if (pitch < 0) pitch = 0;
    else if (pitch > 127) pitch = 127;
    if (value < 0) value = 0;
    else if (value > 127) value = 127;
    sys_queuemidimess(portno, 0, MIDI_POLYAFTERTOUCH + (channel & 0xf),
        pitch, value);
}

void outmidi_mclk(int portno)
{
   sys_queuemidimess(portno, 1, 0xf8, 0,0);
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

/* ------------------------- MIDI input queue handling ------------------ */
typedef struct midiparser
{
    int mp_status;
    int mp_gotbyte1;
    int mp_byte1;
} t_midiparser;

#define MIDINOTEOFF       0x80  /* 2 following 'data bytes' */
#define MIDINOTEON        0x90  /* 2 */
#define MIDIPOLYTOUCH     0xa0  /* 2 */
#define MIDICONTROLCHANGE 0xb0  /* 2 */
#define MIDIPROGRAMCHANGE 0xc0  /* 1 */
#define MIDICHANNELTOUCH  0xd0  /* 1 */
#define MIDIPITCHBEND     0xe0  /* 2 */
#define MIDISTARTSYSEX    0xf0  /* (until F7) */
#define MIDITIMECODE      0xf1  /* 1 */
#define MIDISONGPOS       0xf2  /* 2 */
#define MIDISONGSELECT    0xf3  /* 1 */
#define MIDIRESERVED1     0xf4  /* ? */
#define MIDIRESERVED2     0xf5  /* ? */
#define MIDITUNEREQUEST   0xf6  /* 0 */
#define MIDIENDSYSEX      0xf7  /* 0 */
#define MIDICLOCK         0xf8  /* 0 */
#define MIDITICK          0xf9  /* 0 */
#define MIDISTART         0xfa  /* 0 */
#define MIDICONT          0xfb  /* 0 */
#define MIDISTOP          0xfc  /* 0 */
#define MIDIACTIVESENSE   0xfe  /* 0 */
#define MIDIRESET         0xff  /* 0 */

    /* functions in x_midi.c */
void inmidi_realtimein(int portno, int cmd);
void inmidi_byte(int portno, int byte);
void inmidi_sysex(int portno, int byte);
void inmidi_noteon(int portno, int channel, int pitch, int velo);
void inmidi_controlchange(int portno, int channel, int ctlnumber, int value);
void inmidi_programchange(int portno, int channel, int value);
void inmidi_pitchbend(int portno, int channel, int value);
void inmidi_aftertouch(int portno, int channel, int value);
void inmidi_polyaftertouch(int portno, int channel, int pitch, int value);

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
            if (byte == MIDITUNEREQUEST || byte == MIDIRESERVED1 ||
                byte == MIDIRESERVED2)
                    parserp->mp_status = 0;
            else if (byte == MIDISTARTSYSEX)
            {
                inmidi_sysex(portno, byte);
                parserp->mp_status = byte;
            }
            else if (byte == MIDIENDSYSEX)
            {
                inmidi_sysex(portno, byte);
                parserp->mp_status = 0;
            }
            else
            {
                parserp->mp_status = byte;
            }
            parserp->mp_gotbyte1 = 0;
        }
        else
        {
            int cmd = (parserp->mp_status >= 0xf0 ? parserp->mp_status :
                (parserp->mp_status & 0xf0));
            int chan = (parserp->mp_status & 0xf);
            int byte1 = parserp->mp_byte1, gotbyte1 = parserp->mp_gotbyte1;
            switch (cmd)
            {
            case MIDINOTEOFF:
                if (gotbyte1)
                    inmidi_noteon(portno, chan, byte1, 0),
                        parserp->mp_gotbyte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotbyte1 = 1;
                break;
            case MIDINOTEON:
                if (gotbyte1)
                    inmidi_noteon(portno, chan, byte1, byte),
                        parserp->mp_gotbyte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotbyte1 = 1;
                break;
            case MIDIPOLYTOUCH:
                if (gotbyte1)
                    inmidi_polyaftertouch(portno, chan, byte1, byte),
                        parserp->mp_gotbyte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotbyte1 = 1;
                break;
            case MIDICONTROLCHANGE:
                if (gotbyte1)
                    inmidi_controlchange(portno, chan, byte1, byte),
                        parserp->mp_gotbyte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotbyte1 = 1;
                break;
            case MIDIPROGRAMCHANGE:
                inmidi_programchange(portno, chan, byte);
                break;
            case MIDICHANNELTOUCH:
                inmidi_aftertouch(portno, chan, byte);
                break;
            case MIDIPITCHBEND:
                if (gotbyte1)
                    inmidi_pitchbend(portno, chan, ((byte << 7) + byte1)),
                        parserp->mp_gotbyte1 = 0;
                else parserp->mp_byte1 = byte, parserp->mp_gotbyte1 = 1;
                break;
            case MIDISTARTSYSEX:
                inmidi_sysex(portno, byte);
                break;
                
                /* other kinds of messages are just dropped here.  We'll
                need another status byte before we start letting MIDI in
                again (no running status across "system" messages). */
            case MIDITIMECODE:     /* 1 data byte*/
                break;
            case MIDISONGPOS:       /* 2 */
                break;
            case MIDISONGSELECT:    /* 1 */
                break;
            }
        }
    }  
    midi_inTail  = (midi_inTail + 1 == MIDI_QUEUE_SIZE ? 0 : midi_inTail + 1);
}

void sys_pollmidiinqueue( void)
{
#ifdef TEST_DEJITTER
    static int db = 0;
#endif
    double logicaltime = .001 * scheduler_getMillisecondsSince(midi_systimeAtStart);
#ifdef TEST_DEJITTER
    if (midi_inHead == midi_inTail)
        db = 0;
#endif
    while (midi_inHead != midi_inTail)
    {
#ifdef TEST_DEJITTER
        if (!db)
        {
            post("in del %f, logicaltime %f, RT %f adcminusRT %f",
                (midi_inQueue[midi_inTail].q_time - logicaltime),
                    logicaltime, sys_getRealTime(), midi_adcTimeMinusRealTime);
            db = 1;
        }
#endif
#if 0
        if (midi_inQueue[midi_inTail].q_time <= logicaltime - 0.007)
            post("late %f",
                1000 * (logicaltime - midi_inQueue[midi_inTail].q_time));
#endif
        if (midi_inQueue[midi_inTail].q_time <= logicaltime)
        {
#if 0
            post("diff %f",
                1000* (logicaltime - midi_inQueue[midi_inTail].q_time));
#endif
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
    midi_inQueue[midi_inHead].q_time = sys_getmidiinrealtime();
    midi_inHead = newhead;
    sys_pollmidiinqueue();
}

void sys_pollmidiqueue( void)
{
#if 0
    static double lasttime;
    double newtime = sys_getRealTime();
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
    sys_pollmidioutqueue();
    sys_pollmidiinqueue();
}

/******************** dialog window and device listing ********************/

#define MAXNDEV 20
#define DEVDESCSIZE 80

#define DEVONSET 1  /* To agree with command line flags, normally start at 1 */


#ifdef USEAPI_ALSA
void midi_alsa_init( void);
#endif
#ifdef USEAPI_OSS
void midi_oss_init( void);
#endif

    /* last requested parameters */
static int midi_nmidiindev;
static int midi_midiindev[MAXIMUM_MIDI_IN];
static char midi_indevnames[MAXIMUM_MIDI_IN * DEVDESCSIZE];
static int midi_nmidioutdev;
static int midi_midioutdev[MAXIMUM_MIDI_OUT];
static char midi_outdevnames[MAXIMUM_MIDI_IN * DEVDESCSIZE];

void sys_get_midi_apis(char *buf)
{
    int n = 0;
    strcpy(buf, "{ ");
#ifdef USEAPI_OSS
    sprintf(buf + strlen(buf), "{OSS-MIDI %d} ", API_DEFAULT_MIDI); n++;
#endif
#ifdef USEAPI_ALSA
    sprintf(buf + strlen(buf), "{ALSA-MIDI %d} ", API_ALSA); n++;
#endif
    strcat(buf, "}");
        /* then again, if only one API (or none) we don't offer any choice. */
    if (n < 2)
        strcpy(buf, "{}");
}

void sys_get_midi_params(int *pnmidiindev, int *pmidiindev,
    int *pnmidioutdev, int *pmidioutdev)
{
    int i, devn;
    *pnmidiindev = midi_nmidiindev;
    for (i = 0; i < midi_nmidiindev; i++)
    {
        if ((devn = sys_mididevnametonumber(0,
            &midi_indevnames[i * DEVDESCSIZE])) >= 0)
                pmidiindev[i] = devn;
        else pmidiindev[i] = midi_midiindev[i]; 
    }
    *pnmidioutdev = midi_nmidioutdev;
    for (i = 0; i < midi_nmidioutdev; i++)
    {
        if ((devn = sys_mididevnametonumber(1,
            &midi_outdevnames[i * DEVDESCSIZE])) >= 0)
                pmidioutdev[i] = devn;
        else pmidioutdev[i] = midi_midioutdev[i]; 
    }
}

static void sys_save_midi_params(
    int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev)
{
    int i;
    midi_nmidiindev = nmidiindev;
    for (i = 0; i < nmidiindev; i++)
    {
        midi_midiindev[i] = midiindev[i];
        sys_mididevnumbertoname(0, midiindev[i],
            &midi_indevnames[i * DEVDESCSIZE], DEVDESCSIZE);
    }
    midi_nmidioutdev = nmidioutdev;
    for (i = 0; i < nmidioutdev; i++)
    {
        midi_midioutdev[i] = midioutdev[i]; 
        sys_mididevnumbertoname(1, midioutdev[i],
            &midi_outdevnames[i * DEVDESCSIZE], DEVDESCSIZE);
    }
}

void sys_open_midi(int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev, int enable)
{
    if (enable)
    {
#ifdef USEAPI_ALSA
        midi_alsa_init();
#endif
#ifdef USEAPI_OSS
        midi_oss_init();
#endif
#ifdef USEAPI_ALSA
        if (midi_api == API_ALSA)
            sys_alsa_do_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev);
        else
#endif /* ALSA */
            sys_do_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev);
    }
    sys_save_midi_params(nmidiindev, midiindev,
        nmidioutdev, midioutdev);

    sys_vGui("set ::var(apiMidi) %d\n", midi_api);

}

    /* open midi using whatever parameters were last used */
void sys_reopen_midi( void)
{
    int nmidiindev, midiindev[MAXIMUM_MIDI_IN];
    int nmidioutdev, midioutdev[MAXIMUM_MIDI_OUT];
    sys_get_midi_params(&nmidiindev, midiindev, &nmidioutdev, midioutdev);
    sys_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev, 1);
}

void sys_listmididevs(void )
{
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    int nindevs = 0, noutdevs = 0, i;

    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXNDEV, DEVDESCSIZE);

    if (!nindevs)
        post("no midi input devices found");
    else
    {
        post("MIDI input devices:");
        for (i = 0; i < nindevs; i++)
            post("%d. %s", i+1, indevlist + i * DEVDESCSIZE);
    }
    if (!noutdevs)
        post("no midi output devices found");
    else
    {
        post("MIDI output devices:");
        for (i = 0; i < noutdevs; i++)
            post("%d. %s", i+DEVONSET, outdevlist + i * DEVDESCSIZE);
    }
}

void sys_set_midi_api(int which)
{
     midi_api = which;
     if (0)
        post("midi_api %d", midi_api);
}

void global_midiProperties(void *dummy, t_float flongform);
void midi_alsa_setndevs(int in, int out);

void global_midiAPI(void *dummy, t_float f)
{
    int newapi = f;
    if (newapi != midi_api)
    {
#ifdef USEAPI_ALSA
        if (midi_api == API_ALSA)
            sys_alsa_close_midi();
        else
#endif
              sys_close_midi();
        midi_api = newapi;
        sys_reopen_midi();
    }
#ifdef USEAPI_ALSA
    midi_alsa_setndevs(midi_nmidiindev, midi_nmidioutdev);
#endif
    global_midiProperties(0, (midi_nmidiindev > 1 || midi_nmidioutdev > 1));
}

extern t_class *global_object;

    /* start an midi settings dialog window */
void global_midiProperties(void *dummy, t_float flongform)
{
    char buf[1024 + 2 * MAXNDEV*(DEVDESCSIZE+4)];
        /* these are the devices you're using: */
    int nindev, midiindev[MAXIMUM_MIDI_IN];
    int noutdev, midioutdev[MAXIMUM_MIDI_OUT];
    int midiindev1, midiindev2, midiindev3, midiindev4, midiindev5,
        midiindev6, midiindev7, midiindev8, midiindev9,
        midioutdev1, midioutdev2, midioutdev3, midioutdev4, midioutdev5,
        midioutdev6, midioutdev7, midioutdev8, midioutdev9;

        /* these are all the devices on your system: */
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    int nindevs = 0, noutdevs = 0, i;

    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXNDEV, DEVDESCSIZE);

    sys_gui("set ::ui_midi::midiIn {none}\n");
    for (i = 0; i < nindevs; i++)
        sys_vGui("lappend ::ui_midi::midiIn {%s}\n",
            indevlist + i * DEVDESCSIZE);

    sys_gui("set ::ui_midi::midiOut {none}\n");
    for (i = 0; i < noutdevs; i++)
        sys_vGui("lappend ::ui_midi::midiOut {%s}\n",
            outdevlist + i * DEVDESCSIZE);

    sys_get_midi_params(&nindev, midiindev, &noutdev, midioutdev);

    if (nindev > 1 || noutdev > 1)
        flongform = 1;

    midiindev1 = (nindev > 0 &&  midiindev[0]>= 0 ? midiindev[0]+1 : 0);
    midiindev2 = (nindev > 1 &&  midiindev[1]>= 0 ? midiindev[1]+1 : 0);
    midiindev3 = (nindev > 2 &&  midiindev[2]>= 0 ? midiindev[2]+1 : 0);
    midiindev4 = (nindev > 3 &&  midiindev[3]>= 0 ? midiindev[3]+1 : 0);
    midiindev5 = (nindev > 4 &&  midiindev[4]>= 0 ? midiindev[4]+1 : 0);
    midiindev6 = (nindev > 5 &&  midiindev[5]>= 0 ? midiindev[5]+1 : 0);
    midiindev7 = (nindev > 6 &&  midiindev[6]>= 0 ? midiindev[6]+1 : 0);
    midiindev8 = (nindev > 7 &&  midiindev[7]>= 0 ? midiindev[7]+1 : 0);
    midiindev9 = (nindev > 8 &&  midiindev[8]>= 0 ? midiindev[8]+1 : 0);
    midioutdev1 = (noutdev > 0 && midioutdev[0]>= 0 ? midioutdev[0]+1 : 0); 
    midioutdev2 = (noutdev > 1 && midioutdev[1]>= 0 ? midioutdev[1]+1 : 0); 
    midioutdev3 = (noutdev > 2 && midioutdev[2]>= 0 ? midioutdev[2]+1 : 0); 
    midioutdev4 = (noutdev > 3 && midioutdev[3]>= 0 ? midioutdev[3]+1 : 0); 
    midioutdev5 = (noutdev > 4 && midioutdev[4]>= 0 ? midioutdev[4]+1 : 0);
    midioutdev6 = (noutdev > 5 && midioutdev[5]>= 0 ? midioutdev[5]+1 : 0);
    midioutdev7 = (noutdev > 6 && midioutdev[6]>= 0 ? midioutdev[6]+1 : 0);
    midioutdev8 = (noutdev > 7 && midioutdev[7]>= 0 ? midioutdev[7]+1 : 0);
    midioutdev9 = (noutdev > 8 && midioutdev[8]>= 0 ? midioutdev[8]+1 : 0);

#ifdef USEAPI_ALSA
      if (midi_api == API_ALSA)
    sprintf(buf,
"::ui_midi::show %%s \
%d %d %d %d 0 0 0 0 0 \
%d %d %d %d 0 0 0 0 0 \
1\n",
        midiindev1, midiindev2, midiindev3, midiindev4, 
        midioutdev1, midioutdev2, midioutdev3, midioutdev4);
      else
#endif
    sprintf(buf,
"::ui_midi::show %%s \
%d %d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d %d \
0\n",
        midiindev1, midiindev2, midiindev3, midiindev4, 
        midiindev5, midiindev6, midiindev7, midiindev8, midiindev9, 
        midioutdev1, midioutdev2, midioutdev3, midioutdev4, 
        midioutdev5, midioutdev6, midioutdev7, midioutdev8, midioutdev9);

    gfxstub_deleteforkey(0);
    gfxstub_new(&global_object, (void *)global_midiProperties, buf);
}

    /* new values from dialog window */
void global_midiDialog(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int nmidiindev, midiindev[MAXIMUM_MIDI_IN];
    int nmidioutdev, midioutdev[MAXIMUM_MIDI_OUT];
    int i, nindev, noutdev;
    int newmidiindev[9], newmidioutdev[9];
    int alsadevin, alsadevout;

    for (i = 0; i < 9; i++)
    {
        newmidiindev[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
        newmidioutdev[i] = (t_int)atom_getFloatAtIndex(i+9, argc, argv);
    }

    for (i = 0, nindev = 0; i < 9; i++)
    {
        if (newmidiindev[i] > 0)
        {
            newmidiindev[nindev] = newmidiindev[i]-1;
            nindev++;
        }
    }
    for (i = 0, noutdev = 0; i < 9; i++)
    {
        if (newmidioutdev[i] > 0)
        {
            newmidioutdev[noutdev] = newmidioutdev[i]-1;
            noutdev++;
        }
    }
    alsadevin = (t_int)atom_getFloatAtIndex(18, argc, argv);
    alsadevout = (t_int)atom_getFloatAtIndex(19, argc, argv);
#ifdef USEAPI_ALSA
            /* invent a story so that saving/recalling "settings" will
            be able to restore the number of devices.  ALSA MIDI handling
            uses its own set of variables.  LATER figure out how to get
            this to work coherently */
    if (midi_api == API_ALSA)
    {
        nindev = alsadevin;
        noutdev = alsadevout;
        for (i = 0; i < nindev; i++)
            newmidiindev[i] = i;
        for (i = 0; i < noutdev; i++)
            newmidioutdev[i] = i;
    }
#endif
    sys_save_midi_params(nindev, newmidiindev,
        noutdev, newmidioutdev);
#ifdef USEAPI_ALSA
    if (midi_api == API_ALSA)
    {
        sys_alsa_close_midi();
        sys_open_midi(alsadevin, newmidiindev, alsadevout, newmidioutdev, 1);
    }
    else
#endif
    {
        sys_close_midi();
        sys_open_midi(nindev, newmidiindev, noutdev, newmidioutdev, 1);
    }

}

void sys_get_midi_devs(char *indevlist, int *nindevs,
                       char *outdevlist, int *noutdevs, 
                       int maxndevs, int devdescsize)
{

#ifdef USEAPI_ALSA
  if (midi_api == API_ALSA)
    midi_alsa_getdevs(indevlist, nindevs, outdevlist, noutdevs, 
                      maxndevs, devdescsize);
  else
#endif /* ALSA */
  midi_getdevs(indevlist, nindevs, outdevlist, noutdevs, maxndevs, devdescsize);
}

/* convert a device name to a (1-based) device number.  (Output device if
'output' parameter is true, otherwise input device).  Negative on failure. */

int sys_mididevnametonumber(int output, const char *name)
{
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    int nindevs = 0, noutdevs = 0, i;

    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXNDEV, DEVDESCSIZE);

    if (output)
    {
            /* try first for exact match */
        for (i = 0; i < noutdevs; i++)
            if (!strcmp(name, outdevlist + i * DEVDESCSIZE))
                return (i);
            /* failing that, a match up to end of shorter string */
        for (i = 0; i < noutdevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(outdevlist + i * DEVDESCSIZE))
                comp = strlen(outdevlist + i * DEVDESCSIZE);
            if (!strncmp(name, outdevlist + i * DEVDESCSIZE, comp))
                return (i);
        }
    }
    else
    {
        for (i = 0; i < nindevs; i++)
            if (!strcmp(name, indevlist + i * DEVDESCSIZE))
                return (i);
        for (i = 0; i < nindevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(indevlist + i * DEVDESCSIZE))
                comp = strlen(indevlist + i * DEVDESCSIZE);
            if (!strncmp(name, indevlist + i * DEVDESCSIZE, comp))
                return (i);
        }
    }
    return (-1);
}

/* convert a (1-based) device number to a device name.  (Output device if
'output' parameter is true, otherwise input device).  Empty string on failure.
*/

void sys_mididevnumbertoname(int output, int devno, char *name, int namesize)
{
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    int nindevs = 0, noutdevs = 0, i;
    if (devno < 0)
    {
        *name = 0;
        return;
    }
    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXNDEV, DEVDESCSIZE);
    if (output && (devno < noutdevs))
        strncpy(name, outdevlist + devno * DEVDESCSIZE, namesize);
    else if (!output && (devno < nindevs))
        strncpy(name, indevlist + devno * DEVDESCSIZE, namesize);
    else *name = 0;
    name[namesize-1] = 0;
}
