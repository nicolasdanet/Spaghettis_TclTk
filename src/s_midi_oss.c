
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

#define MIDIOSS_DEVICES         4
#define MIDIOSS_DESCRIPTION     8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  midioss_numberOfDetectedIn;                                                 /* Shared. */
static int  midioss_numberOfDetectedOut;                                                /* Shared. */
static char midioss_detectedInNames[MIDIOSS_DEVICES * MIDIOSS_DESCRIPTION];             /* Shared. */
static char midioss_detectedOutNames[MIDIOSS_DEVICES * MIDIOSS_DESCRIPTION];            /* Shared. */

static int  midioss_numberOfDevicesIn;
static int  midioss_numberOfDevicesOut;
static int  midioss_devicesIn[MAXIMUM_MIDI_IN];
static int  midioss_devicesOut[MAXIMUM_MIDI_OUT];

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

void midi_initializeNative (void)
{
    midioss_initialize();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_openNative (int numberOfDevicesIn, int *devicesIn, int numberOfDevicesOut, int *devicesOut)
{
    int i;
    
    midioss_numberOfDevicesIn  = 0;
    midioss_numberOfDevicesOut = 0;
    
    for (i = 0; i < numberOfDevicesIn; i++) {
    //
    int f, n = devicesIn[i];
    
    if (n >= 0 && n < midioss_numberOfDetectedIn) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, "/dev/midi%s", midioss_detectedInNames[n]);
        if ((f = open (t, O_RDONLY | O_NDELAY)) >= 0) { 
            midioss_devicesIn[midioss_numberOfDevicesIn] = f; midioss_numberOfDevicesIn++;
        }
    }
    //
    }
    
    for (i = 0; i < numberOfDevicesOut; i++) {
    //
    int f, n = devicesOut[i];
    
    if (n >= 0 && n < midioss_numberOfDetectedOut) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, "/dev/midi%s", midioss_detectedOutNames[n]);
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
#pragma mark -

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
#pragma mark -

void midi_pollNative (void)
{
    int i, again = 1;
    int throttle = 100;
        
    while (again && (throttle > 0)) {
    //
    throttle--;
    
    again = 0;
    
    for (i = 0; i < midioss_numberOfDevicesIn; i++) {
        char c;
        ssize_t n = read (midioss_devicesIn[i], (void *)&c, 1);
        if (n > 0) { midi_receive (i, (c & 0xff)); again = 1; }
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
    int m = PD_MIN (midioss_numberOfDetectedIn, MAXIMUM_DEVICES);
    int n = PD_MIN (midioss_numberOfDetectedOut, MAXIMUM_DEVICES);

    t_error err = PD_ERROR_NONE;
    
    for (i = 0; i < m; i++) {
        char *s = devicesIn + (i * MAXIMUM_DESCRIPTION);
        err |= string_sprintf (s, "/dev/midi%s", midioss_detectedInNames[i]);
    }
    
    for (i = 0; i < n; i++) {
        char *s = devicesOut + (i * MAXIMUM_DESCRIPTION);
        err |= string_sprintf (s, "/dev/midi%s", midioss_detectedOutNames[i]);
    }
    
    *numberOfDevicesIn  = m;
    *numberOfDevicesOut = n;
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
