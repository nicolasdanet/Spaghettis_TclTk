
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

#define MIDIOSS_LENGTH(x)       (((x)<0xC0)?2:((x)<0xE0)?1:((x)<0xF0)?2:((x)==0xF2)?2:((x)<0xF4)?1:0)
    
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
    int fd = -1;
    int j;
    int outdevindex = -1;
    char namebuf[80];
    int devno = devicesIn[i];
    
    if (devno < 0 || devno >= midioss_numberOfDetectedIn)
        continue;
        
    for (j = 0; j < numberOfDevicesOut; j++)
        if (devicesOut[j] >= 0 && devicesOut[j] <= midioss_numberOfDetectedOut
            && !strcmp(midioss_detectedOutNames[devicesOut[j]],
            midioss_detectedInNames[devno]))
                outdevindex = j;

    sprintf(namebuf, "/dev/midi%s", midioss_detectedInNames[devno]);

        /* try to open the device for read/write. */
    if (outdevindex >= 0)
    {
        //sys_setalarm(1000000);
        fd = open(namebuf, O_RDWR | O_NDELAY);
        if (0)
            post("tried to open %s read/write; got %d\n",
                namebuf, fd);
        if (outdevindex >= 0 && fd >= 0)
            midioss_devicesOut[outdevindex] = fd;
    }
        /* OK, try read-only */
    if (fd < 0)
    {
        //sys_setalarm(1000000);
        fd = open(namebuf, O_RDONLY | O_NDELAY);
        if (0)
            post("tried to open %s read-only; got %d\n",
                namebuf, fd);
    }
    if (fd >= 0)
        midioss_devicesIn[midioss_numberOfDevicesIn++] = fd;       
    else post("couldn't open MIDI input device %s", namebuf);
    //
    }
    
    for (i = 0; i < numberOfDevicesOut; i++)
    {
        int fd = midioss_devicesOut[i];
        char namebuf[80];
        int devno = devicesOut[i];
        if (devno < 0 || devno >= midioss_numberOfDetectedOut)
            continue;
        sprintf(namebuf, "/dev/midi%s", midioss_detectedOutNames[devno]);
        if (fd < 0)
        {
            //sys_setalarm(1000000);
            fd = open(namebuf, O_WRONLY | O_NDELAY);
            if (0)
                post("tried to open %s write-only; got %d\n",
                    namebuf, fd);
        }
        if (fd >= 0)
            midioss_devicesOut[midioss_numberOfDevicesOut++] = fd;     
        else post("couldn't open MIDI output device %s", namebuf);
    }

    if (midioss_numberOfDevicesIn < numberOfDevicesIn || midioss_numberOfDevicesOut < numberOfDevicesOut || 0)
        post("opened %d MIDI input device(s) and %d MIDI output device(s).",
            midioss_numberOfDevicesIn, midioss_numberOfDevicesOut);

    //sys_setalarm(0);
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

void midi_pushNextMessageNative(int portno, int a, int b, int c)
{
    if (portno >= 0 && portno < midioss_numberOfDevicesOut)
    {
       switch (MIDIOSS_LENGTH(a))
       {
       case 2:
            midioss_writeByte(midioss_devicesOut[portno],a);        
            midioss_writeByte(midioss_devicesOut[portno],b);        
            midioss_writeByte(midioss_devicesOut[portno],c);
            return;
       case 1:
            midioss_writeByte(midioss_devicesOut[portno],a);        
            midioss_writeByte(midioss_devicesOut[portno],b);        
            return;
       case 0:
            midioss_writeByte(midioss_devicesOut[portno],a);        
            return;
       };
    }
}

void midi_pushNextByteNative(int portno, int byte)
{
    if (portno >= 0 && portno < midioss_numberOfDevicesOut)
        midioss_writeByte(midioss_devicesOut[portno], byte);       
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if 0   /* this is the "select" version which doesn't work with OSS
        driver for emu10k1 (it doesn't implement select.) */
void midi_pollNative(void)
{
    int i, throttle = 100;
    struct timeval timout;
    int did = 1, maxfd = 0;
    while (did)
    {
        fd_set readset, writeset, exceptset;
        did = 0;
        if (throttle-- < 0)
            break;
        timout.tv_sec = 0;
        timout.tv_usec = 0;

        FD_ZERO(&writeset);
        FD_ZERO(&readset);
        FD_ZERO(&exceptset);
        for (i = 0; i < midioss_numberOfDevicesIn; i++)
        {
            if (midioss_devicesIn[i] > maxfd)
                maxfd = midioss_devicesIn[i];
            FD_SET(midioss_devicesIn[i], &readset);
        }
        select(maxfd+1, &readset, &writeset, &exceptset, &timout);
        for (i = 0; i < midioss_numberOfDevicesIn; i++)
            if (FD_ISSET(midioss_devicesIn[i], &readset))
        {
            char c;
            int ret = read(midioss_devicesIn[i], &c, 1);
            if (ret <= 0)
                fprintf(stderr, "Midi read error\n");
            else midi_receive(i, (c & 0xff));
            did = 1;
        }
    }
}
#else 

    /* this version uses the asynchronous "read()" ... */
void midi_pollNative(void)
{
    int i, throttle = 100;
    struct timeval timout;
    int did = 1, maxfd = 0;
    while (did)
    {
        fd_set readset, writeset, exceptset;
        did = 0;
        if (throttle-- < 0)
            break;
        for (i = 0; i < midioss_numberOfDevicesIn; i++)
        {
            char c;
            int ret = read(midioss_devicesIn[i], &c, 1);
            if (ret < 0)
            {
                if (errno != EAGAIN)
                    perror("MIDI");
            }
            else if (ret != 0)
            {
                midi_receive(i, (c & 0xff));
                did = 1;
            }
        }
    }
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error midi_getListsNative(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs)
{
    int maxndev = MAXIMUM_DEVICES;
    int devdescsize = MAXIMUM_DESCRIPTION;
    
    int i, ndev;
        
    if ((ndev = midioss_numberOfDetectedIn) > maxndev)
        ndev = maxndev;
    for (i = 0; i < ndev; i++)
        sprintf(indevlist + i * devdescsize,
            "/dev/midi%s", midioss_detectedInNames[i]);
    *nindevs = ndev;

    if ((ndev = midioss_numberOfDetectedOut) > maxndev)
        ndev = maxndev;
    for (i = 0; i < ndev; i++)
        sprintf(outdevlist + i * devdescsize,
            "/dev/midi%s", midioss_detectedOutNames[i]);
    *noutdevs = ndev;
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
