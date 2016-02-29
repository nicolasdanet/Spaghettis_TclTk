
/* 
    Copyright (c) 1997-2003 Guenter Geiger, Miller Puckette,
    Larry Troxler, Winfried Ritsch, Karl MacMillan, and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "portmidi.h"
#include "porttime.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int midipm_numberOfDevicesIn;
static int midipm_numberOfDevicesOut;

static PmStream *midipm_devicesIn[MAXIMUM_MIDI_IN];
static PmStream *midipm_devicesOut[MAXIMUM_MIDI_OUT];

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void midi_openNative (int numberOfDevicesIn, int *devicesIn, int numberOfDevicesOut, int *devicesOut)
{
    int i, j, n;
    
    Pt_Start (1, NULL, NULL);       /* Start a timer with millisecond accuracy. */

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
            if (err) { PD_BUG; }
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
            if (err) { PD_BUG; }
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

void sys_putmidimess(int portno, int a, int b, int c)
{
    PmEvent buffer;
    /* fprintf(stderr, "put 1 msg %d %d\n", portno, midipm_numberOfDevicesOut); */
    if (portno >= 0 && portno < midipm_numberOfDevicesOut)
    {
        buffer.message = Pm_Message(a, b, c);
        buffer.timestamp = 0;
        /* fprintf(stderr, "put msg\n"); */
        Pm_Write(midipm_devicesOut[portno], &buffer, 1);
    }
}

static void writemidi4(PortMidiStream* stream, int a, int b, int c, int d)
{
    PmEvent buffer;
    buffer.timestamp = 0;
    buffer.message = ((a & 0xff) | ((b & 0xff) << 8)
        | ((c & 0xff) << 16) | ((d & 0xff) << 24));
    Pm_Write(stream, &buffer, 1);
}


void sys_putmidibyte(int portno, int byte)
{
        /* try to parse the bytes into MIDI messages so they can
        fit into PortMidi buffers. */
    static int mess[4];
    static int nbytes = 0, sysex = 0, i;
    if (byte >= 0xf8)   /* MIDI real time */
        writemidi4(midipm_devicesOut[portno], byte, 0, 0, 0);
    else if (byte == 0xf0)
    {
        mess[0] = 0xf0;
        nbytes = 1;
        sysex = 1;
    }
    else if (byte == 0xf7)
    {
        mess[nbytes] = byte;
        for (i = nbytes+1; i < 4; i++)
            mess[i] = 0;
        writemidi4(midipm_devicesOut[portno],
            mess[0], mess[1], mess[2], mess[3]);
        sysex = 0;
        nbytes = 0;
    }
    else if (byte >= 0x80)
    {
        sysex = 0;
        if (byte == 0xf4 || byte == 0xf5 || byte == 0xf6)
        {
            writemidi4(midipm_devicesOut[portno], byte, 0, 0, 0);
            nbytes = 0;
        }
        else
        {
            mess[0] = byte;
            nbytes = 1;
        }
    }
    else if (sysex)
    {
        mess[nbytes] = byte;
        nbytes++;
        if (nbytes == 4)
        {
            writemidi4(midipm_devicesOut[portno],
                mess[0], mess[1], mess[2], mess[3]);
            nbytes = 0;
        }
    }
    else if (nbytes)
    {
        int status = mess[0];
        if (status < 0xf0)
            status &= 0xf0;
                /* 2 byte messages: */
        if (status == 0xc0 || status == 0xd0 ||
            status == 0xf1 || status == 0xf3)
        {
            writemidi4(midipm_devicesOut[portno],
                mess[0], byte, 0, 0);
            nbytes = (status < 0xf0 ? 1 : 0);
        }
        else
        {
            if (nbytes == 1)
            {
                mess[1] = byte;
                nbytes = 2;
            }
            else
            {
                writemidi4(midipm_devicesOut[portno],
                    mess[0], mess[1], byte, 0);
                nbytes = (status < 0xf0 ? 1 : 0);
            }
        }
    }
}

/* this is non-zero if we are in the middle of transmitting sysex */

int nd_sysex_mode=0;

/* send in 4 bytes of sysex data. if one of the bytes is 0xF7 (sysex end)
    stop and unset nd_sysex_mode */ 
void nd_sysex_inword(int midiindev, int status, int data1, int data2, int data3)
{
    if (nd_sysex_mode) {
        midi_receive(midiindev, status);
        if (status == 0xF7)
            nd_sysex_mode = 0;
    }

    if (nd_sysex_mode) {
        midi_receive(midiindev, data1);
        if (data1 == 0xF7)
            nd_sysex_mode = 0;
    }

    if (nd_sysex_mode) {
        midi_receive(midiindev, data2);
        if (data2 == 0xF7)
            nd_sysex_mode = 0;
    }

    if (nd_sysex_mode) {
        midi_receive(midiindev, data3);
        if (data3 == 0xF7)
            nd_sysex_mode = 0;
    }
}

void sys_poll_midi(void)
{
    int i, nmess, throttle = 100;
    PmEvent buffer;
    for (i = 0; i < midipm_numberOfDevicesIn; i++)
    {
        while (1)
        {
            if (!throttle--)
                goto overload;
            nmess = Pm_Read(midipm_devicesIn[i], &buffer, 1);
            if (nmess > 0)
            {
                int status = Pm_MessageStatus(buffer.message);
                int data1  = Pm_MessageData1(buffer.message);
                int data2  = Pm_MessageData2(buffer.message);
                int data3 = ((buffer.message >> 24) & 0xff);
                int msgtype = ((status & 0xf0) == 0xf0 ?
                    status : (status & 0xf0));
                if (nd_sysex_mode)
                    nd_sysex_inword(i, status, data1, data2, data3);
                else switch (msgtype)
                {
                case MIDI_NOTEOFF: 
                case MIDI_NOTEON: 
                case MIDI_POLYPRESSURE:
                case MIDI_CONTROLCHANGE:
                case MIDI_PITCHBEND:
                case MIDI_SONGPOS:
                    midi_receive(i, status);
                    midi_receive(i, data1);
                    midi_receive(i, data2);
                    break; 
                case MIDI_PROGRAMCHANGE:
                case MIDI_AFTERTOUCH:
                case MIDI_TIMECODE:
                case MIDI_SONGSELECT:
                    midi_receive(i, status);
                    midi_receive(i, data1);
                    break;
                case MIDI_STARTSYSEX:
                    nd_sysex_mode=1;
                    nd_sysex_inword(i, status, data1, data2, data3);
                    break; 
                default:
                    midi_receive(i, status);
                    break;
                }
            }
            else break;
        }
    }
    overload: ;
}

void midi_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs)
{
    int maxndev = MAXIMUM_DEVICES;
    int devdescsize = MAXIMUM_DESCRIPTION;
    
    int i, nindev = 0, noutdev = 0;
    for (i = 0; i < Pm_CountDevices(); i++)
    {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        /* post("%d: %s, %s (%d,%d)", i, info->interf, info->name,
            info->input, info->output); */
        if (info->input && nindev < maxndev)
        {
            strcpy(indevlist + nindev * devdescsize, info->name);
            nindev++;
        }
        if (info->output && noutdev < maxndev)
        {
            strcpy(outdevlist + noutdev * devdescsize, info->name);
            noutdev++;
        }
    }
    *nindevs = nindev;
    *noutdevs = noutdev;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
