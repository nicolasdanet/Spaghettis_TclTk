
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

/* Note that a negative number of channels corresponds to a disabled device. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      audio_state;                                                        /* Shared. */
  
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      audio_numberOfDevicesIn;                                            /* Shared. */
static int      audio_devicesInChannels[MAXIMUM_AUDIO_IN];                          /* Shared. */
static char     audio_devicesInNames[MAXIMUM_MIDI_IN * MAXIMUM_DESCRIPTION];        /* Shared. */

static int      audio_numberOfDevicesOut;                                           /* Shared. */
static int      audio_devicesOutChannels[MAXIMUM_AUDIO_OUT];                        /* Shared. */
static char     audio_devicesOutNames[MAXIMUM_AUDIO_OUT * MAXIMUM_DESCRIPTION];     /* Shared. */

static int      audio_tempSampleRate = AUDIO_DEFAULT_SAMPLERATE;                    /* Shared. */
static int      audio_tempBlockSize  = AUDIO_DEFAULT_BLOCKSIZE;                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUDIO_DEFAULT_DEVICE        0
#define AUDIO_DEFAULT_CHANNELS      2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUDIO_MAXIMUM_BLOCKSIZE     2048

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_open (void)
{
    int m = 0;
    int n = 0;
    int i[MAXIMUM_AUDIO_IN]  = { 0 };
    int j[MAXIMUM_AUDIO_IN]  = { 0 };
    int o[MAXIMUM_AUDIO_OUT] = { 0 };
    int p[MAXIMUM_AUDIO_OUT] = { 0 };
    
    int sampleRate;
    int blockSize;
    
    t_error err = PD_ERROR;
    
    audio_getDevices (&m, i, j, &n, o, p, &sampleRate, &blockSize);
    
    if (m || n) {
    //
    int t;
    
    audio_setSampleRate (sampleRate);
    audio_initializeMemory (audio_getChannelsIn(), audio_getChannelsOut());
    
    for (t = 0; t < m; t++) { j[t] = PD_MAX (0, j[t]); }    /* Avoid negative (disabled) channels. */
    for (t = 0; t < n; t++) { p[t] = PD_MAX (0, p[t]); }
    
    err = audio_openNative (sampleRate,
            (m > 0 ? j[0] : 0),
            (n > 0 ? p[0] : 0),
            blockSize, 
            (m > 0 ? i[0] : 0),
            (n > 0 ? o[0] : 0));
    //
    }
    
    if (err) {
        post_error (PD_TRANSLATE ("audio: fails to open device"));     // --
        audio_state = 0;
        scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
        
    } else {
        audio_state = 1;
        scheduler_setAudioMode (SCHEDULER_AUDIO_POLL);
    }
    
    return err;
}

void audio_close (void)
{
    if (audio_isOpened()) { audio_closeNative(); }
    audio_state = 0;
    scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
    // sys_gui ("set ::var(isDsp) 0\n");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_isOpened (void)
{
    return (audio_state != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_getDevices (int *numberOfDevicesIn, 
    int *devicesIn,
    int *channelsIn,
    int *numberOfDevicesOut,
    int *devicesOut,
    int *channelsOut,
    int *sampleRate,
    int *blockSize)
{
    int i;
    int m = 0;
    int n = 0;
    
    for (i = 0; i < audio_numberOfDevicesIn; i++) {
        int t = audio_numberWithName (0, &audio_devicesInNames[i * MAXIMUM_DESCRIPTION]);
        if (t != -1) {
            devicesIn[m]  = t;
            channelsIn[m] = audio_devicesInChannels[i];
            m++;
        }
    }
    
    for (i = 0; i < audio_numberOfDevicesOut; i++) {
        int t = audio_numberWithName (1, &audio_devicesOutNames[i * MAXIMUM_DESCRIPTION]);
        if (t != -1) {
            devicesOut[n]  = t;
            channelsOut[n] = audio_devicesOutChannels[i];
            n++;
        }
    }
    
    *numberOfDevicesIn  = m;
    *numberOfDevicesOut = n;
    
    *sampleRate   = audio_tempSampleRate;
    *blockSize    = audio_tempBlockSize;
}

static void audio_setDevices (int numberOfDevicesIn, 
    int *devicesIn,
    int *channelsIn,
    int numberOfDevicesOut,
    int *devicesOut,
    int *channelsOut,
    int sampleRate,
    int blockSize)
{
    int i;
    
    int m = 0;
    int n = 0;
    
    PD_ASSERT (numberOfDevicesIn <= MAXIMUM_AUDIO_IN);
    PD_ASSERT (numberOfDevicesOut <= MAXIMUM_AUDIO_OUT);
    
    for (i = 0; i < numberOfDevicesIn; i++) {
        char *s = &audio_devicesInNames[m * MAXIMUM_DESCRIPTION];
        if (!audio_numberToName (0, devicesIn[i], s, MAXIMUM_DESCRIPTION)) {
            int t = MAXIMUM_CHANNELS_IN;
            audio_devicesInChannels[m] = PD_CLAMP (channelsIn[i], -t, t);
            m++;
        }
    }

    for (i = 0; i < numberOfDevicesOut; i++) {
        char *s = &audio_devicesOutNames[n * MAXIMUM_DESCRIPTION];
        if (!audio_numberToName (1, devicesOut[i], s, MAXIMUM_DESCRIPTION)) {
            int t = MAXIMUM_CHANNELS_OUT; 
            audio_devicesOutChannels[n] = PD_CLAMP (channelsOut[i], -t, t);
            n++;
        }
    }
    
    audio_numberOfDevicesIn     = m;
    audio_numberOfDevicesOut    = n;
    audio_tempSampleRate        = sampleRate;
    audio_tempBlockSize         = blockSize;
}

static void audio_setDevicesAndParameters (int numberOfDevicesIn,
    int *devicesIn,
    int *channelsIn, 
    int numberOfDevicesOut, 
    int *devicesOut,
    int *channelsOut, 
    int sampleRate, 
    int blockSize)
{
    int i;
    int totalOfChannelsIn  = 0;
    int totalOfChannelsOut = 0;
    
    if (sampleRate < 1)           { sampleRate = AUDIO_DEFAULT_SAMPLERATE; }
    if (!PD_ISPOWER2 (blockSize)) { blockSize  = AUDIO_DEFAULT_BLOCKSIZE;  }
    
    blockSize = PD_CLAMP (blockSize, INTERNAL_BLOCKSIZE, AUDIO_MAXIMUM_BLOCKSIZE); 
        
    audio_setDevices (numberOfDevicesIn, 
        devicesIn, 
        channelsIn,
        numberOfDevicesOut, 
        devicesOut,
        channelsOut,
        sampleRate,
        blockSize);
    
    for (i = 0; i < audio_numberOfDevicesIn; i++) {
        if (audio_devicesInChannels[i] > 0)  { totalOfChannelsIn += audio_devicesInChannels[i]; }
    }
    
    for (i = 0; i < audio_numberOfDevicesOut; i++) {
        if (audio_devicesOutChannels[i] > 0) { totalOfChannelsOut += audio_devicesOutChannels[i]; }
    }
    
    audio_setSampleRate (sampleRate);
    audio_initializeMemory (totalOfChannelsIn, totalOfChannelsOut);
}

/* Called by JACK to notify the number of frames used. */

void audio_setBlockSize (int blockSize)
{
    audio_tempBlockSize = blockSize;        /* Expect store to be thread-safe. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_setDevicesWithDefault (int numberOfDevicesIn,
    int *devicesIn,
    int *channelsIn, 
    int numberOfDevicesOut, 
    int *devicesOut,
    int *channelsOut, 
    int sampleRate, 
    int blockSize)
{
    /* For convenience, initialize with the first devices if none are provided. */
    
    if (numberOfDevicesIn == 0) { 
        *devicesIn = AUDIO_DEFAULT_DEVICE; *channelsIn = AUDIO_DEFAULT_CHANNELS; 
        numberOfDevicesIn = 1;
    }
    
    if (numberOfDevicesOut == 0) { 
        *devicesOut = AUDIO_DEFAULT_DEVICE; *channelsOut = AUDIO_DEFAULT_CHANNELS;
        numberOfDevicesOut = 1;
    }
    
    audio_setDevicesAndParameters (numberOfDevicesIn,
        devicesIn,
        channelsIn,
        numberOfDevicesOut,
        devicesOut,
        channelsOut,
        sampleRate,
        blockSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error audio_getLists (char *i, int *m, char *o, int *n, int *multiple)
{
    return audio_getListsNative (i, m, o, n, multiple);
}

int audio_numberWithName (int isOutput, const char *name)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    int  canMultiple;
    int  k;
    
    if (!audio_getLists (i, &m, o, &n, &canMultiple)) {
    //
    if (isOutput) {
        for (k = 0; k < n; k++) {
            if (!strcmp (name, o + (k * MAXIMUM_DESCRIPTION))) { return k; }
        }
    } else {
        for (k = 0; k < m; k++) {
            if (!strcmp (name, i + (k * MAXIMUM_DESCRIPTION))) { return k; }
        }
    }
    //
    }
    
    return -1;
}

t_error audio_numberToName (int isOutput, int k, char *dest, size_t size)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    char o[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int  canMultiple;
    
    t_error err = PD_ERROR;
    
    if (k >= 0 && !audio_getLists (i, &m, o, &n, &canMultiple)) { 
        if (isOutput && (k < n))        { err = string_copy (dest, size, o + (k * MAXIMUM_DESCRIPTION)); }
        else if (!isOutput && (k < m))  { err = string_copy (dest, size, i + (k * MAXIMUM_DESCRIPTION)); }
    }
    
    if (err) { *dest = 0; }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error audio_requireDialogInitialize (int *multiple)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    
    t_error err = audio_getLists (i, &m, o, &n, multiple);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int k;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_audio::audioIn [list ");                       // --
    err |= string_copy (t2, PD_STRING, "set ::ui_audio::audioOut [list ");                      // --
    
    for (k = 0; k < m; k++) {
        err |= string_addSprintf (t1, PD_STRING, " {%s}", i + (k * MAXIMUM_DESCRIPTION));       // --
    }
    for (k = 0; k < n; k++) {
        err |= string_addSprintf (t2, PD_STRING, " {%s}", o + (k * MAXIMUM_DESCRIPTION));       // --
    }
    
    err |= string_add (t1, PD_STRING, "]\n");
    err |= string_add (t2, PD_STRING, "]\n");
    
    sys_gui (t1);
    sys_gui (t2);
    //
    }
    
    return err;
}

void audio_requireDialog (void *dummy)
{
    int m = 0;
    int n = 0;
    int i[MAXIMUM_AUDIO_IN]  = { 0 };
    int j[MAXIMUM_AUDIO_IN]  = { 0 };
    int o[MAXIMUM_AUDIO_OUT] = { 0 };
    int p[MAXIMUM_AUDIO_OUT] = { 0 };
    int sampleRate;
    int blockSize;
    
    int canMultiple;

    t_error err = audio_requireDialogInitialize (&canMultiple);
    
    audio_getDevices (&m, i, j, &n, o, p, &sampleRate, &blockSize);

    if (!err) {
    //
    char t[PD_STRING] = { 0 };
    
    int i1          = (m > 0 && i[0] >= 0 ? i[0] : 0);
    int i2          = (m > 1 && i[1] >= 0 ? i[1] : 0);
    int i3          = (m > 2 && i[2] >= 0 ? i[2] : 0);
    int i4          = (m > 3 && i[3] >= 0 ? i[3] : 0);
    int iChannels1  = (m > 0 ? j[0] : 0);
    int iChannels2  = (m > 1 ? j[1] : 0);
    int iChannels3  = (m > 2 ? j[2] : 0);
    int iChannels4  = (m > 3 ? j[3] : 0);
    int o1          = (n > 0 && o[0] >= 0 ? o[0] : 0);  
    int o2          = (n > 1 && o[1] >= 0 ? o[1] : 0);  
    int o3          = (n > 2 && o[2] >= 0 ? o[2] : 0);  
    int o4          = (n > 3 && o[3] >= 0 ? o[3] : 0); 
    int oChannels1  = (n > 0 ? p[0] : 0);
    int oChannels2  = (n > 1 ? p[1] : 0);
    int oChannels3  = (n > 2 ? p[2] : 0);
    int oChannels4  = (n > 3 ? p[3] : 0);
    
    err |= string_sprintf (t, PD_STRING,
        "::ui_audio::show %%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
        i1, i2, i3, i4, iChannels1, iChannels2, iChannels3, iChannels4, 
        o1, o2, o3, o4, oChannels1, oChannels2, oChannels3, oChannels4, 
        sampleRate,
        blockSize,
        canMultiple);
        
    if (!err) {
        guistub_new (&global_object, (void *)audio_requireDialog, t);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void audio_fromDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int m = 0;
    int n = 0;
    int i[MAXIMUM_AUDIO_IN]  = { 0 };
    int j[MAXIMUM_AUDIO_IN]  = { 0 };
    int o[MAXIMUM_AUDIO_OUT] = { 0 };
    int p[MAXIMUM_AUDIO_OUT] = { 0 };
        
    int sampleRate;
    int blockSize;

    int t;
    
    PD_ASSERT (argc == 18);
    PD_ASSERT (MAXIMUM_AUDIO_IN  >= 4);
    PD_ASSERT (MAXIMUM_AUDIO_OUT >= 4);
    
    for (t = 0; t < 4; t++) {
        i[t] = (t_int)atom_getFloatAtIndex (t + 0,  argc, argv);
        j[t] = (t_int)atom_getFloatAtIndex (t + 4,  argc, argv);
        o[t] = (t_int)atom_getFloatAtIndex (t + 8,  argc, argv);
        p[t] = (t_int)atom_getFloatAtIndex (t + 12, argc, argv);
    }
    
    sampleRate   = (t_int)atom_getFloatAtIndex (16, argc, argv);
    blockSize    = (t_int)atom_getFloatAtIndex (17, argc, argv);
    
    /* Remove devices with number of channels set to zero. */
    
    for (t = 0; t < 4; t++) { if (j[t] != 0) { i[m] = i[t]; j[m] = j[t]; m++; } }
    for (t = 0; t < 4; t++) { if (p[t] != 0) { o[n] = o[t]; p[n] = p[t]; n++; } }
    
    audio_close();
        
    audio_setDevicesAndParameters (m, i, j, n, o, p, sampleRate, blockSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
