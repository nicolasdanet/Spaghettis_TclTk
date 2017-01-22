
/* 
    Copyright (c) 1997-2016 Guenter Geiger, Miller Puckette,
    Larry Troxler, Winfried Ritsch, Karl MacMillan, and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "portmidi.h"
#include "porttime.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int midipm_numberOfDevicesIn;                        /* Shared. */
static int midipm_numberOfDevicesOut;                       /* Shared. */

static PmStream *midipm_devicesIn[MAXIMUM_MIDI_IN];         /* Shared. */
static PmStream *midipm_devicesOut[MAXIMUM_MIDI_OUT];       /* Shared. */

static int midipm_sysexFlag;                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void midipm_sysexWordIn (int port, int s, int a, int b, int c)
{
    if (midipm_sysexFlag) { 
        midi_receive (port, s); if (s == MIDI_ENDSYSEX) { midipm_sysexFlag = 0; } 
    }
    if (midipm_sysexFlag) {
        midi_receive (port, a); if (a == MIDI_ENDSYSEX) { midipm_sysexFlag = 0; } 
    }
    if (midipm_sysexFlag) {
        midi_receive (port, b); if (b == MIDI_ENDSYSEX) { midipm_sysexFlag = 0; }
    }
    if (midipm_sysexFlag) {
        midi_receive (port, c); if (c == MIDI_ENDSYSEX) { midipm_sysexFlag = 0; }
    }
}

static void midipm_writeFourBytes (PmStream *stream, int a, int b, int c, int d)
{
    PmEvent buffer;
    buffer.message   = ((a & 0xff) | ((b & 0xff) << 8) | ((c & 0xff) << 16) | ((d & 0xff) << 24));
    buffer.timestamp = 0;
    Pm_Write (stream, &buffer, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *midi_nameNative (void)
{
    static char *name = "PortMidi"; return name;
}

void midi_initializeNative (void)
{
    Pm_Initialize();
    
    if (!Pt_Started()) { Pt_Start (1, NULL, NULL); }    /* Start a timer with millisecond accuracy. */
}

void midi_releaseNative (void)
{
    Pm_Terminate();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_openNative (int numberOfDevicesIn, int *devicesIn, int numberOfDevicesOut, int *devicesOut)
{
    int i, j, n;

    midipm_numberOfDevicesIn  = 0;
    midipm_numberOfDevicesOut = 0;
    
    for (i = 0; i < numberOfDevicesIn; i++) {
    //
    for (j = 0, n = 0; j < Pm_CountDevices(); j++) {
    //
    const PmDeviceInfo *info = Pm_GetDeviceInfo (j);
    
    if (info->input) {
        if (devicesIn[i] == n) {
            PmStream **t = &midipm_devicesIn[midipm_numberOfDevicesIn];
            PmError err = Pm_OpenInput (t, j, NULL, 100, NULL, NULL);
            if (err) { error__error1 (Pm_GetErrorText (err)); }
            else {
                midipm_numberOfDevicesIn++;
            }
        }
        n++;
    }
    //
    }
    //
    }   
    
    for (i = 0; i < numberOfDevicesOut; i++) {
    //
    for (j = 0, n = 0; j < Pm_CountDevices(); j++) {
    //
    const PmDeviceInfo *info = Pm_GetDeviceInfo (j);
    
    if (info->output) {
        if (devicesOut[i] == n) {
            PmStream **t = &midipm_devicesOut[midipm_numberOfDevicesOut];
            PmError err = Pm_OpenOutput (t, j, NULL, 0, NULL, NULL, 0);
            if (err) { error__error1 (Pm_GetErrorText (err)); }
            else {
                midipm_numberOfDevicesOut++;
            }
        }
        n++;
    }
    //
    }
    //
    }   
}

void midi_closeNative (void)
{
    int i;
    
    for (i = 0; i < midipm_numberOfDevicesIn; i++)  { Pm_Close (midipm_devicesIn[i]);  }
    for (i = 0; i < midipm_numberOfDevicesOut; i++) { Pm_Close (midipm_devicesOut[i]); }
    
    midipm_numberOfDevicesIn  = 0;
    midipm_numberOfDevicesOut = 0; 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_pushNextMessageNative (int port, int a, int b, int c)
{
    if (port >= 0 && port < midipm_numberOfDevicesOut) {
    //
    PmEvent buffer;
    
    buffer.message   = Pm_Message (a, b, c);
    buffer.timestamp = 0;
    
    Pm_Write (midipm_devicesOut[port], &buffer, 1);
    //
    }
}

void midi_pushNextByteNative (int port, int byte)
{
    static int t[4];            /* Shared. */
    static int n = 0;           /* Shared. */
    static int sysex = 0;       /* Shared. */
    
    if (port >= 0 && port < midipm_numberOfDevicesOut) {
    //
    PmStream *device = midipm_devicesOut[port];
    
    if (byte >= MIDI_CLOCK)             { 
    
        midipm_writeFourBytes (device, byte, 0, 0, 0); 
    
    } else if (byte == MIDI_STARTSYSEX) {
    
        t[0] = MIDI_STARTSYSEX; 
        n = 1;
        sysex = 1;
        
    } else if (byte == MIDI_ENDSYSEX)   {
    
        int i;
        t[n] = byte;
        for (i = n + 1; i < 4; i++) { t[i] = 0; }
        midipm_writeFourBytes (device, t[0], t[1], t[2], t[3]);
        n = 0;
        sysex = 0;
        
    } else if (byte >= MIDI_NOTEOFF)    {
    
        if (byte == MIDI_RESERVED1 || byte == MIDI_RESERVED2 || byte == MIDI_TUNEREQUEST) {
            midipm_writeFourBytes (device, byte, 0, 0, 0);
            n = 0;
            sysex = 0;
        } else {
            t[0] = byte;
            n = 1;
            sysex = 0;
        }
        
    } else if (sysex)   {
    
        t[n] = byte;
        n++;
        if (n == 4) {
            midipm_writeFourBytes (device, t[0], t[1], t[2], t[3]);
            n = 0;
        }
        
    } else if (n)       {
    
        int k = t[0];
        if (k < MIDI_STARTSYSEX) { k &= 0xf0; }
        if (k == MIDI_PROGRAMCHANGE || k == MIDI_AFTERTOUCH || k == MIDI_TIMECODE || k == MIDI_SONGSELECT) {
        
            midipm_writeFourBytes (device, t[0], byte, 0, 0);           /* Two bytes messages. */
            n = (k < MIDI_STARTSYSEX ? 1 : 0);
            
        } else {
            if (n == 1) { t[1] = byte; n = 2; }
            else {
                midipm_writeFourBytes (device, t[0], t[1], byte, 0);    /* Three bytes messages. */
                n = (k < MIDI_STARTSYSEX ? 1 : 0);
            }
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_pollNative (void)
{
    int port;
    int throttle = 100;
    
    PmEvent buffer;
    
    for (port = 0; port < midipm_numberOfDevicesIn; port++) {
    //
    while (throttle > 0) {
    //
    int n = Pm_Read (midipm_devicesIn[port], &buffer, 1);
    
    throttle--;
        
    if (n <= 0) { break; }
    else {
        int status = Pm_MessageStatus (buffer.message);
        int data1  = Pm_MessageData1  (buffer.message);
        int data2  = Pm_MessageData2  (buffer.message);
        int data3  = ((buffer.message >> 24) & 0xff);
        int type   = ((status & 0xf0) == 0xf0 ? status : (status & 0xf0));      /* LSB kept only for SYSEX. */
        
        if (midipm_sysexFlag) { midipm_sysexWordIn (port, status, data1, data2, data3); }
        else switch (type) {
        //
        case MIDI_NOTEOFF       :   /* Missing break deliberately. */
        case MIDI_NOTEON        : 
        case MIDI_POLYPRESSURE  :
        case MIDI_CONTROLCHANGE :
        case MIDI_PITCHBEND     :
        case MIDI_SONGPOS       :
                                    midi_receive (port, status);
                                    midi_receive (port, data1);
                                    midi_receive (port, data2);
                                    break; 
        case MIDI_PROGRAMCHANGE :
        case MIDI_AFTERTOUCH    :
        case MIDI_TIMECODE      :
        case MIDI_SONGSELECT    :
                                    midi_receive (port, status);
                                    midi_receive (port, data1);
                                    break;
        case MIDI_STARTSYSEX    :
                                    midipm_sysexFlag = 1;
                                    midipm_sysexWordIn (port, status, data1, data2, data3);
                                    break; 
        default                 :
                                    midi_receive (port, status);
                                    break;
        //
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error midi_getListsNative (char *devicesIn,
    int *numberOfDevicesIn, 
    char *devicesOut, 
    int *numberOfDevicesOut)
{
    int i;
    int m = 0;
    int n = 0;
    
    t_error err = PD_ERROR_NONE;
    
    for (i = 0; i < Pm_CountDevices(); i++) {
    //
    const PmDeviceInfo *info = Pm_GetDeviceInfo (i);

    if (info->input && m < MAXIMUM_DEVICES) {
        err |= string_copy (devicesIn + (m * MAXIMUM_DESCRIPTION), MAXIMUM_DESCRIPTION, info->name);
        m++;
    }
    
    if (info->output && n < MAXIMUM_DEVICES) {
        err |= string_copy (devicesOut + (n * MAXIMUM_DESCRIPTION), MAXIMUM_DESCRIPTION, info->name);
        n++;
    }
    //
    }
    
    *numberOfDevicesIn  = m;
    *numberOfDevicesOut = n;
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
