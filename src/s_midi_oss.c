
/* 
    Copyright (c) 1997-2017 Guenter Geiger, Miller Puckette,
    Larry Troxler, Winfried Ritsch, Karl MacMillan, and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define MIDIOSS_DEVICES         4
#define MIDIOSS_DESCRIPTION     8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  midioss_numberOfDetectedIn;                                                 /* Static. */
static int  midioss_numberOfDetectedOut;                                                /* Static. */
static char midioss_detectedInNames[MIDIOSS_DEVICES * MIDIOSS_DESCRIPTION];             /* Static. */
static char midioss_detectedOutNames[MIDIOSS_DEVICES * MIDIOSS_DESCRIPTION];            /* Static. */

static int  midioss_numberOfDevicesIn;
static int  midioss_numberOfDevicesOut;
static int  midioss_devicesIn[DEVICES_MAXIMUM_IO];
static int  midioss_devicesOut[DEVICES_MAXIMUM_IO];

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void midioss_writeByte (int fd, int n)
{
    char b = n; if ((write (fd, &b, 1)) != 1) { PD_BUG; }
}

static int midioss_initializeSearch (const char *name, const char *description)
{
    int f;
    
    if ((f = open (name, O_RDONLY | O_NDELAY)) >= 0) {
        char *s = midioss_detectedInNames + (midioss_numberOfDetectedIn * MIDIOSS_DESCRIPTION);
        close (f);
        string_copy (s, MIDIOSS_DESCRIPTION, description);
        midioss_numberOfDetectedIn++;
    }
    
    if ((f = open (name, O_WRONLY | O_NDELAY)) >= 0) {
        char *s = midioss_detectedOutNames + (midioss_numberOfDetectedOut * MIDIOSS_DESCRIPTION);
        close (f);
        string_copy (s, MIDIOSS_DESCRIPTION, description);
        midioss_numberOfDetectedOut++;
    }
    
    if (midioss_numberOfDetectedIn >= MIDIOSS_DEVICES)  { return 1; }
    if (midioss_numberOfDetectedOut >= MIDIOSS_DEVICES) { return 1; }
    
    return 0;
}

static void midioss_initialize (void)     
{
    int i;

    midioss_numberOfDetectedIn  = 0;
    midioss_numberOfDetectedOut = 0;

    for (i = 0; i < MIDIOSS_DEVICES; i++) {
    //
    char t[PD_STRING] = { 0 };
    char s[MIDIOSS_DESCRIPTION] = { 0 };

    string_sprintf (t, PD_STRING, "/dev/midi%d", i);
    string_sprintf (s, MIDIOSS_DESCRIPTION, "%d", i);
    
    if (midioss_initializeSearch (t, s)) {
        break;
    }
    
    string_sprintf (t, PD_STRING, "/dev/midi%2d", i);
    string_sprintf (s, MIDIOSS_DESCRIPTION, "%2d", i);
        
    if (midioss_initializeSearch (t, s)) {
        break;
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char *midi_nameNative (void)
{
    static char *name = "OSS"; return name;
}

void midi_initializeNative (void)
{
    midioss_initialize();
}

void midi_releaseNative (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_openNative (t_devicesproperties *p)
{
    int i;
    
    midioss_numberOfDevicesIn  = 0;
    midioss_numberOfDevicesOut = 0;
    
    for (i = 0; i < devices_getInSize (p); i++) {
    //
    int f, n = devices_getInAtIndex (p, i);
    
    if (n >= 0 && n < midioss_numberOfDetectedIn) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, PD_STRING, "/dev/midi%s", midioss_detectedInNames[n]);
        if ((f = open (t, O_RDONLY | O_NDELAY)) >= 0) { 
            midioss_devicesIn[midioss_numberOfDevicesIn] = f; midioss_numberOfDevicesIn++;
        }
    }
    //
    }
    
    for (i = 0; i < devices_getOutSize (p); i++) {
    //
    int f, n = devices_getOutAtIndex (p, i);
    
    if (n >= 0 && n < midioss_numberOfDetectedOut) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, PD_STRING, "/dev/midi%s", midioss_detectedOutNames[n]);
        if ((f = open (t, O_WRONLY | O_NDELAY)) >= 0) { 
            midioss_devicesOut[midioss_numberOfDevicesOut] = f; midioss_numberOfDevicesOut++;
        }
    }
    //
    }
}

void midi_closeNative()
{
    int i;
    
    for (i = 0; i < midioss_numberOfDevicesIn; i++)  { close (midioss_devicesIn[i]);  }
    for (i = 0; i < midioss_numberOfDevicesOut; i++) { close (midioss_devicesOut[i]); }
    
    midioss_numberOfDevicesIn  = 0;
    midioss_numberOfDevicesOut = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_pushNextMessageNative (int port, int a, int b, int c)
{
    if (port >= 0 && port < midioss_numberOfDevicesOut) {
    //
    switch (MIDI_BYTES (a)) {
       case 2   :   midioss_writeByte (midioss_devicesOut[port], a);        
                    midioss_writeByte (midioss_devicesOut[port], b);        
                    midioss_writeByte (midioss_devicesOut[port], c);
                    break;
       case 1   :   midioss_writeByte (midioss_devicesOut[port], a);        
                    midioss_writeByte (midioss_devicesOut[port], b);        
                    break;
       case 0   :   midioss_writeByte (midioss_devicesOut[port], a);        
                    break;
    };
    //
    }
}

void midi_pushNextByteNative (int port, int a)
{
    if (port >= 0 && port < midioss_numberOfDevicesOut) {
    //
    midioss_writeByte (midioss_devicesOut[port], a);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_pollNative (void)
{
    int i, again = 1;
    int throttle = 100;
        
    while (again && (throttle > 0)) {
    //
    throttle--;
    
    again = 0;
    
    for (i = 0; i < midioss_numberOfDevicesIn; i++) {
        unsigned char byte;
        ssize_t n = read (midioss_devicesIn[i], (void *)&byte, 1);
        if (n > 0) { midi_receive (i, (int)(byte & 0xff)); again = 1; }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error midi_getListsNative (t_deviceslist *p)
{
    int i;
    int m = PD_MIN (midioss_numberOfDetectedIn, DEVICES_MAXIMUM_IO);
    int n = PD_MIN (midioss_numberOfDetectedOut, DEVICES_MAXIMUM_IO);

    t_error err = PD_ERROR_NONE;
    
    for (i = 0; i < m; i++) {
        char t[DEVICES_DESCRIPTION] = { 0 };
        err |= string_sprintf (t, DEVICES_DESCRIPTION, "/dev/midi%s", midioss_detectedInNames[i]);
        deviceslist_appendMidiIn (p, t);
    }
    
    for (i = 0; i < n; i++) {
        char t[DEVICES_DESCRIPTION] = { 0 };
        err |= string_sprintf (t, DEVICES_DESCRIPTION, "/dev/midi%s", midioss_detectedOutNames[i]);
        deviceslist_appendMidiOut (p, t);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
