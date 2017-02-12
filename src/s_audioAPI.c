
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that a negative number of channels corresponds to a disabled device. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  audio_state;                                                        /* Shared. */
  
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  audio_numberOfDevicesIn;                                            /* Shared. */
static int  audio_devicesInChannels[DEVICES_MAXIMUM_IO];                        /* Shared. */
static char audio_devicesInNames[DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION];     /* Shared. */

static int  audio_numberOfDevicesOut;                                           /* Shared. */
static int  audio_devicesOutChannels[DEVICES_MAXIMUM_IO];                       /* Shared. */
static char audio_devicesOutNames[DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION];    /* Shared. */

static int  audio_tempSampleRate = AUDIO_DEFAULT_SAMPLERATE;                    /* Shared. */
static int  audio_tempBlockSize  = AUDIO_DEFAULT_BLOCKSIZE;                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error audio_getLists (char *i, int *m, char *o, int *n, int *multiple)
{
    return audio_getListsNative (i, m, o, n, multiple);
}

#if 0

static t_error audio_requireDialogInitialize (int *multiple)
{
    int  m = 0;
    int  n = 0;
    char i[DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION] = { 0 };
    char o[DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION] = { 0 };
    
    t_error err = audio_getLists (i, &m, o, &n, multiple);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int k;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_audio::audioIn [list ");                   // --
    err |= string_copy (t2, PD_STRING, "set ::ui_audio::audioOut [list ");                  // --
    
    for (k = 0; k < m; k++) {
        err |= string_addSprintf (t1, PD_STRING, " {%s}", i + (k * DEVICES_DESCRIPTION));   // --
    }
    for (k = 0; k < n; k++) {
        err |= string_addSprintf (t2, PD_STRING, " {%s}", o + (k * DEVICES_DESCRIPTION));   // --
    }
    
    err |= string_add (t1, PD_STRING, "]\n");
    err |= string_add (t2, PD_STRING, "]\n");
    
    sys_gui (t1);
    sys_gui (t2);
    //
    }
    
    return err;
}

#endif

static void audio_setDevicesProceed (t_devicesproperties *p)
{
    int i;
    
    int m = 0;
    int n = 0;

    for (i = 0; i < devices_getInSize (p); i++) {
        char *s = &audio_devicesInNames[m * DEVICES_DESCRIPTION];
        if (!audio_deviceAsStringWithNumber (0, devices_getInAtIndex (p, i), s, DEVICES_DESCRIPTION)) {
            int t = DEVICES_MAXIMUM_CHANNELS;
            audio_devicesInChannels[m] = PD_CLAMP (devices_getInChannelsAtIndex (p, i), -t, t);
            m++;
        }
    }

    for (i = 0; i < devices_getOutSize (p); i++) {
        char *s = &audio_devicesOutNames[n * DEVICES_DESCRIPTION];
        if (!audio_deviceAsStringWithNumber (1, devices_getOutAtIndex (p, i), s, DEVICES_DESCRIPTION)) {
            int t = DEVICES_MAXIMUM_CHANNELS; 
            audio_devicesOutChannels[n] = PD_CLAMP (devices_getOutChannelsAtIndex (p, i), -t, t);
            n++;
        }
    }
    
    audio_numberOfDevicesIn  = m;
    audio_numberOfDevicesOut = n;
    audio_tempSampleRate     = devices_getSampleRate (p);
    audio_tempBlockSize      = devices_getBlockSize (p);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_open (void)
{
    t_error err = PD_ERROR;
    
    t_devicesproperties audio; devices_initAsAudio (&audio);
    
    audio_getDevices (&audio);
    
    if (devices_getInSize (&audio) || devices_getOutSize (&audio)) {
    
        audio_setSampleRate (devices_getSampleRate (&audio));
        audio_initializeMemory (audio_getChannelsIn(), audio_getChannelsOut());
        devices_checkChannels (&audio);
        
        err = audio_openNative (&audio); 
    }
    
    if (err) {
        error_canNotOpen (sym_audio);
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

void audio_getDevices (t_devicesproperties *p)
{
    int i;
    
    for (i = 0; i < audio_numberOfDevicesIn; i++) {
        devices_appendAudioIn (p, 
            &audio_devicesInNames[i * DEVICES_DESCRIPTION],
            audio_devicesInChannels[i]);
    }
    
    for (i = 0; i < audio_numberOfDevicesOut; i++) {
        devices_appendAudioOut (p,
            &audio_devicesOutNames[i * DEVICES_DESCRIPTION],
            audio_devicesOutChannels[i]);
    }
    
    devices_setBlockSize (p, audio_tempBlockSize);
    devices_setSampleRate (p, audio_tempSampleRate);
}

void audio_setDevices (t_devicesproperties *p)
{
    int i;
    int totalOfChannelsIn  = 0;
    int totalOfChannelsOut = 0;
 
    audio_setDevicesProceed (p);
    
    for (i = 0; i < audio_numberOfDevicesIn; i++) {
        if (audio_devicesInChannels[i] > 0)  { totalOfChannelsIn += audio_devicesInChannels[i]; }
    }
    
    for (i = 0; i < audio_numberOfDevicesOut; i++) {
        if (audio_devicesOutChannels[i] > 0) { totalOfChannelsOut += audio_devicesOutChannels[i]; }
    }
    
    audio_setSampleRate (devices_getSampleRate (p));
    audio_initializeMemory (totalOfChannelsIn, totalOfChannelsOut);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Called by JACK to notify the number of frames used. */

void audio_setBlockSize (int blockSize)
{
    audio_tempBlockSize = blockSize;        /* Expect store to be thread-safe. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_deviceAsNumberWithString (int isOutput, char *name)
{
    int  m = 0;
    int  n = 0;
    char i[DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION] = { 0 };
    char o[DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION] = { 0 };
    int  canMultiple;
    int  k;
    
    if (!audio_getLists (i, &m, o, &n, &canMultiple)) {
    //
    if (isOutput) {
        for (k = 0; k < n; k++) {
            if (!strcmp (name, o + (k * DEVICES_DESCRIPTION))) { return k; }
        }
    } else {
        for (k = 0; k < m; k++) {
            if (!strcmp (name, i + (k * DEVICES_DESCRIPTION))) { return k; }
        }
    }
    //
    }
    
    return -1;
}

t_error audio_deviceAsStringWithNumber (int isOutput, int k, char *dest, size_t size)
{
    int  m = 0;
    int  n = 0;
    char i[DEVICES_MAXIMUM_DEVICES*DEVICES_DESCRIPTION];
    char o[DEVICES_MAXIMUM_DEVICES*DEVICES_DESCRIPTION];
    int  canMultiple;
    
    t_error err = PD_ERROR;
    
    if (k >= 0 && !audio_getLists (i, &m, o, &n, &canMultiple)) { 
        if (isOutput && (k < n))        { err = string_copy (dest, size, o + (k * DEVICES_DESCRIPTION)); }
        else if (!isOutput && (k < m))  { err = string_copy (dest, size, i + (k * DEVICES_DESCRIPTION)); }
    }
    
    if (err) { *dest = 0; }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_requireDialog (void *dummy)
{
    #if 0
    
    int m = 0;
    int n = 0;
    int i[DEVICES_MAXIMUM_IO]  = { 0 };
    int j[DEVICES_MAXIMUM_IO]  = { 0 };
    int o[DEVICES_MAXIMUM_IO] = { 0 };
    int p[DEVICES_MAXIMUM_IO] = { 0 };
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
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void audio_fromDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    #if 0
    
    int m = 0;
    int n = 0;
    int i[DEVICES_MAXIMUM_IO] = { 0 };
    int j[DEVICES_MAXIMUM_IO] = { 0 };
    int o[DEVICES_MAXIMUM_IO] = { 0 };
    int p[DEVICES_MAXIMUM_IO] = { 0 };
        
    int sampleRate;
    int blockSize;

    int t;
    
    PD_ASSERT (argc == 18);
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 4);
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 4);
    
    for (t = 0; t < 4; t++) {
        i[t] = (int)atom_getFloatAtIndex (t + 0,  argc, argv);
        j[t] = (int)atom_getFloatAtIndex (t + 4,  argc, argv);
        o[t] = (int)atom_getFloatAtIndex (t + 8,  argc, argv);
        p[t] = (int)atom_getFloatAtIndex (t + 12, argc, argv);
    }
    
    sampleRate   = (int)atom_getFloatAtIndex (16, argc, argv);
    blockSize    = (int)atom_getFloatAtIndex (17, argc, argv);
    
    /* Remove devices with number of channels set to zero. */
    
    for (t = 0; t < 4; t++) { if (j[t] != 0) { i[m] = i[t]; j[m] = j[t]; m++; } }
    for (t = 0; t < 4; t++) { if (p[t] != 0) { o[n] = o[t]; p[n] = p[t]; n++; } }
    
    // audio_close();
        
    // audio_setDevices (m, i, j, n, o, p, sampleRate, blockSize);
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
