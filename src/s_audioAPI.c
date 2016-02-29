
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

/* Note that a negative channels number corresponds to a disabled device. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class      *global_object;

extern t_sample     *audio_soundIn;
extern t_sample     *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int      audio_channelsIn;
extern int      audio_channelsOut;
extern int      audio_advanceInSamples;
extern int      audio_advanceInMicroseconds;
extern t_float  audio_sampleRate;
extern int      audio_blockSize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int             audio_api           = API_DEFAULT_AUDIO;                            /* Shared. */
int             audio_openedApi     = -1;                                           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      audio_state;                                                        /* Shared. */
static int      audio_callbackIsOpen;                                               /* Shared. */
  
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      audio_numberOfDevicesIn;                                            /* Shared. */
static int      audio_devicesInChannels[MAXIMUM_AUDIO_IN];                          /* Shared. */
static char     audio_devicesInNames[MAXIMUM_MIDI_IN * MAXIMUM_DESCRIPTION];        /* Shared. */

static int      audio_numberOfDevicesOut;                                           /* Shared. */
static int      audio_devicesOutChannels[MAXIMUM_AUDIO_OUT];                        /* Shared. */
static char     audio_devicesOutNames[MAXIMUM_AUDIO_OUT * MAXIMUM_DESCRIPTION];     /* Shared. */

static int      audio_tempSampleRate;                                               /* Shared. */
static int      audio_tempAdvance;                                                  /* Shared. */
static int      audio_tempWithCallback;                                             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUDIO_DEFAULT_DEVICE        0
#define AUDIO_DEFAULT_CHANNELS      2
#define AUDIO_MAXIMUM_BLOCKSIZE     2048

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_initializeMemory (int usedChannelsIn, int usedChannelsOut, int sampleRate);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_setAPI (void *dummy, t_float f)
{
    int api = (audio_isAPIAvailable ((int)f) ? (int)f : API_DEFAULT_AUDIO);
    
    if (api != audio_api) {
        audio_close();
        audio_api = api;
        audio_numberOfDevicesIn  = 0;
        audio_numberOfDevicesOut = 0;
        sys_vGui ("set ::var(apiAudio) %d\n", audio_api);   // --
    }
}

t_error audio_getAPIAvailables (char *dest, size_t size)
{
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (dest, size, "{ ");
    
    #ifdef USEAPI_OSS
        err |= string_addSprintf (dest, size, "{OSS %d} ", API_OSS);                    // --
    #endif

    #ifdef USEAPI_MMIO
        err |= string_addSprintf (dest, size, "{MMIO %d} ", API_MMIO);                  // --
    #endif
    
    #ifdef USEAPI_ALSA
        err |= string_addSprintf (dest, size, "{ALSA %d} ", API_ALSA);                  // --
    #endif

    #ifdef USEAPI_PORTAUDIO
        err |= string_addSprintf (dest, size, "{PortAudio %d} ", API_PORTAUDIO);        // --
    #endif
    
    #ifdef USEAPI_JACK
        err |= string_addSprintf (dest, size, "{JACK %d} ", API_JACK);                  // --
    #endif
    
    #ifdef USEAPI_DUMMY
        err |= string_addSprintf (dest, size, "{Dummy %d} ", API_DUMMY);                // --
    #endif
    
    err |= string_add (dest, size, "}");
        
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int audio_isAPIAvailable (int api)
{
    int available = 0;
    
    #ifdef USEAPI_PORTAUDIO
        available += (api == API_PORTAUDIO);
    #endif
    #ifdef USEAPI_JACK
        available += (api == API_JACK);
    #endif
    #ifdef USEAPI_OSS
        available += (api == API_OSS);
    #endif
    #ifdef USEAPI_ALSA
        available += (api == API_ALSA);
    #endif
    #ifdef USEAPI_MMIO
        available += (api == API_MMIO);
    #endif
    #ifdef USEAPI_DUMMY
        available += (api == API_DUMMY);
    #endif

    return available;
}

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
    int advance;
    int withCallback;
    int blockSize;
    
    t_error err = PD_ERROR;
    
    audio_getDevices (&m, i, j, &n, o, p, &sampleRate, &advance, &withCallback, &blockSize);
    
    if (!(audio_api == API_PORTAUDIO || audio_api == API_JACK)) { withCallback = 0; }
    
    if (m || n) {
    //
    int t;
    
    audio_initializeMemory (audio_channelsIn, audio_channelsOut, sampleRate);
    
    for (t = 0; t < m; t++) { j[t] = PD_MAX (0, j[t]); }    /* Avoid negative (disabled) channels. */
    for (t = 0; t < n; t++) { p[t] = PD_MAX (0, p[t]); }
    
    if (API_WITH_PORTAUDIO && audio_api == API_PORTAUDIO)   {
    
        err = pa_open_audio ((m > 0 ? j[0] : 0),
                (n > 0 ? p[0] : 0), 
                sampleRate,
                audio_soundIn,
                audio_soundOut,
                (audio_blockSize ? audio_blockSize : 64), 
                audio_advanceInSamples / (audio_blockSize ? audio_blockSize : 64), 
                (m > 0 ? i[0] : 0),
                (n > 0 ? o[0] : 0),
                (withCallback ? scheduler_audioCallback : NULL));
               
    } else if (API_WITH_JACK && audio_api == API_JACK)      {
    
        err = jack_open_audio ((m > 0 ? j[0] : 0),
                (n > 0 ? p[0] : 0),
                sampleRate,
                (withCallback ? scheduler_audioCallback : NULL));

    } else if (API_WITH_OSS && audio_api == API_OSS)        {
    
        err = oss_open_audio (m, i, m, j, n, o, n, p, sampleRate, audio_blockSize);
        
    } else if (API_WITH_ALSA && audio_api == API_ALSA)      {
    
        err = alsa_open_audio (m, i, m, j, n, o, n, p, sampleRate, audio_blockSize);
        
    } else if (API_WITH_MMIO && audio_api == API_MMIO)      {
    
        err = mmio_open_audio (m, i, m, j, n, o, n, p, sampleRate, audio_blockSize);
        
    } else if (API_WITH_DUMMY && audio_api == API_DUMMY)    {
    
        err = dummy_open_audio();
        
    } else if (audio_api != API_NONE) { PD_BUG; }
    //
    }
    
    if (err) {
        audio_state = 0;
        audio_openedApi = -1;
        audio_callbackIsOpen = 0;
        scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
        
    } else {
        audio_state = 1;
        audio_openedApi = audio_api;
        audio_callbackIsOpen = withCallback;
        scheduler_setAudioMode ((withCallback ? SCHEDULER_AUDIO_CALLBACK : SCHEDULER_AUDIO_POLL));
    }
    
    return err;
}

void audio_close (void)
{
    if (audio_isOpened()) {
    //
    if (API_WITH_PORTAUDIO  && audio_openedApi == API_PORTAUDIO)    { pa_close_audio();     }
    else if (API_WITH_JACK  && audio_openedApi == API_JACK)         { jack_close_audio();   }
    else if (API_WITH_OSS   && audio_openedApi == API_OSS)          { oss_close_audio();    }
    else if (API_WITH_ALSA  && audio_openedApi == API_ALSA)         { alsa_close_audio();   }
    else if (API_WITH_MMIO  && audio_openedApi == API_MMIO)         { mmio_close_audio();   }
    else if (API_WITH_DUMMY && audio_openedApi == API_DUMMY)        { dummy_close_audio();  }
    else {
        PD_BUG;
    }
    
    audio_state = 0;
    audio_openedApi = -1;
    audio_callbackIsOpen = 0;
    
    scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
    
    sys_gui ("set ::var(isDsp) 0\n");   // --
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_isOpened (void)
{
    return audio_state;
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
    int *advance,
    int *withCallback,
    int *blockSize)
{
    int i;
    
    *numberOfDevicesIn = audio_numberOfDevicesIn;
    *numberOfDevicesOut = audio_numberOfDevicesOut;
    
    for (i = 0; i < audio_numberOfDevicesIn; i++) {
        devicesIn[i]  = audio_numberWithName (0, &audio_devicesInNames[i * MAXIMUM_DESCRIPTION]);
        channelsIn[i] = audio_devicesInChannels[i];
        PD_ASSERT (devicesIn[i] != -1);
    }
    
    for (i = 0; i < audio_numberOfDevicesOut; i++) {
        devicesOut[i]  = audio_numberWithName (1, &audio_devicesOutNames[i * MAXIMUM_DESCRIPTION]);
        channelsOut[i] = audio_devicesOutChannels[i]; 
        PD_ASSERT (devicesOut[i] != -1);
    }
    
    *sampleRate   = audio_tempSampleRate;
    *advance      = audio_tempAdvance;
    *withCallback = audio_tempWithCallback;
    *blockSize    = audio_blockSize;
}

static void audio_setDevices (int numberOfDevicesIn, 
    int *devicesIn,
    int *channelsIn,
    int numberOfDevicesOut,
    int *devicesOut,
    int *channelsOut,
    int sampleRate,
    int advance,
    int withCallback,
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
    audio_tempAdvance           = advance;
    audio_tempWithCallback      = withCallback;
    audio_blockSize             = blockSize;
}

static void audio_setDevicesAndParameters (int numberOfDevicesIn,
    int *devicesIn,
    int *channelsIn, 
    int numberOfDevicesOut, 
    int *devicesOut,
    int *channelsOut, 
    int sampleRate, 
    int advance, 
    int withCallback, 
    int blockSize)
{
    int i;
    int totalOfChannelsIn  = 0;
    int totalOfChannelsOut = 0;
    
    if (!PD_IS_POWER2 (blockSize))  { blockSize  = AUDIO_DEFAULT_BLOCKSIZE;  }
    if (sampleRate < 1)             { sampleRate = AUDIO_DEFAULT_SAMPLERATE; }
    if (advance < 0)                { advance    = AUDIO_DEFAULT_ADVANCE;    }

    blockSize = PD_CLAMP (blockSize, AUDIO_DEFAULT_BLOCKSIZE, AUDIO_MAXIMUM_BLOCKSIZE); 
    
    audio_advanceInMicroseconds = MILLISECONDS_TO_MICROSECONDS (advance);

    audio_setDevices (numberOfDevicesIn, 
        devicesIn, 
        channelsIn,
        numberOfDevicesOut, 
        devicesOut,
        channelsOut,
        sampleRate, 
        advance, 
        withCallback,
        blockSize);
    
    for (i = 0; i < audio_numberOfDevicesIn; i++) {
        if (audio_devicesInChannels[i] > 0)  { totalOfChannelsIn += audio_devicesInChannels[i]; }
    }
    
    for (i = 0; i < audio_numberOfDevicesOut; i++) {
        if (audio_devicesOutChannels[i] > 0) { totalOfChannelsOut += audio_devicesOutChannels[i]; }
    }
    
    audio_initializeMemory (totalOfChannelsIn, totalOfChannelsOut, sampleRate);
}

void audio_setDefaultDevicesAndParameters (int numberOfDevicesIn,
    int *devicesIn,
    int *channelsIn, 
    int numberOfDevicesOut, 
    int *devicesOut,
    int *channelsOut, 
    int sampleRate, 
    int advance, 
    int withCallback, 
    int blockSize)
{
    /* For convenience, initialize with first devices if none are provided. */
    
    if (numberOfDevicesIn == 0) { 
        *devicesIn = AUDIO_DEFAULT_DEVICE;  *channelsIn  = AUDIO_DEFAULT_CHANNELS; 
        numberOfDevicesIn  = 1;
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
        advance,
        withCallback,
        blockSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error audio_getLists (char *i, int *m, char *o, int *n, int *multiple, int *callback)
{
    int k = audio_api;
    
    if (API_WITH_PORTAUDIO && k == API_PORTAUDIO)   { pa_getdevs (i, m, o, n, multiple, callback);    }
    else if (API_WITH_JACK && k == API_JACK)        { jack_getdevs (i, m, o, n, multiple, callback);  }
    else if (API_WITH_OSS && k == API_OSS)          { oss_getdevs (i, m, o, n, multiple, callback);   }
    else if (API_WITH_ALSA && k == API_ALSA)        { alsa_getdevs (i, m, o, n, multiple, callback);  }
    else if (API_WITH_MMIO && k == API_MMIO)        { mmio_getdevs (i, m, o, n, multiple, callback);  }
    else if (API_WITH_DUMMY && k == API_DUMMY)      { dummy_getdevs (i, m, o, n, multiple, callback); }
    else {
        PD_BUG; *m = *n = *i = *o = 0; return PD_ERROR;
    }
    
    return PD_ERROR_NONE;
}

int audio_numberWithName (int isOutput, const char *name)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    int  canMultiple;
    int  canCallback;
    int  k;
    
    if (!audio_getLists (i, &m, o, &n, &canMultiple, &canCallback)) {
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
    int  canCallback;
    
    t_error err = PD_ERROR;
    
    if (k >= 0 && !audio_getLists (i, &m, o, &n, &canMultiple, &canCallback)) { 
        if (isOutput && (k < n))        { err = string_copy (dest, size, o + (k * MAXIMUM_DESCRIPTION)); }
        else if (!isOutput && (k < m))  { err = string_copy (dest, size, i + (k * MAXIMUM_DESCRIPTION)); }
    }
    
    if (err) { *dest = 0; }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error audio_requireDialogInitialize (int *multiple, int *callback)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    
    t_error err = audio_getLists (i, &m, o, &n, multiple, callback);
    
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
    int advance;
    int withCallback;
    int blockSize;
    
    int canMultiple;
    int canCallback;

    t_error err = audio_requireDialogInitialize (&canMultiple, &canCallback);
    
    audio_getDevices (&m, i, j, &n, o, p, &sampleRate, &advance, &withCallback, &blockSize);

    if (!err) {
    //
    char t[PD_STRING];
    
    int i1          = (m > 0 && i[0] >= 0 ? i[0] : 0);
    int i2          = (m > 1 && i[1] >= 0 ? i[1] : 0);
    int i3          = (m > 2 && i[2] >= 0 ? i[2] : 0);
    int i4          = (m > 3 && i[3] >= 0 ? i[3] : 0);
    int iChannels1  = (m > 0 ? j[0] : 0);
    int iChannels2  = (m > 1 ? j[1] : 0);
    int iChannels3  = (m > 2 ? j[2] : 0);
    int iChannels4  = (m > 3 ? j[3] : 0);
    int o1          = (n > 0 && o[0] >=0 ? o[0] : 0);  
    int o2          = (n > 1 && o[1] >=0 ? o[1] : 0);  
    int o3          = (n > 2 && o[2] >=0 ? o[2] : 0);  
    int o4          = (n > 3 && o[3] >=0 ? o[3] : 0); 
    int oChannels1  = (n > 0 ? p[0] : 0);
    int oChannels2  = (n > 1 ? p[1] : 0);
    int oChannels3  = (n > 2 ? p[2] : 0);
    int oChannels4  = (n > 3 ? p[3] : 0);
    
    err |= string_sprintf (t, PD_STRING,
        "::ui_audio::show %%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
        i1, i2, i3, i4, iChannels1, iChannels2, iChannels3, iChannels4, 
        o1, o2, o3, o4, oChannels1, oChannels2, oChannels3, oChannels4, 
        sampleRate,
        advance,
        canMultiple,
        (canCallback ? withCallback : -1),
        blockSize);
        
    if (!err) {
        gfxstub_deleteforkey (NULL);
        gfxstub_new (&global_object, (void *)audio_requireDialog, t);
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
    int advance;
    int withCallback;
    int blockSize;

    int t;
    
    PD_ASSERT (argc == 20);
    PD_ASSERT (MAXIMUM_AUDIO_IN  >= 4);
    PD_ASSERT (MAXIMUM_AUDIO_OUT >= 4);
    
    for (t = 0; t < 4; t++) {
        i[t] = (t_int)atom_getFloatAtIndex (t + 0,  argc, argv);
        j[t] = (t_int)atom_getFloatAtIndex (t + 4,  argc, argv);
        o[t] = (t_int)atom_getFloatAtIndex (t + 8,  argc, argv);
        p[t] = (t_int)atom_getFloatAtIndex (t + 12, argc, argv);
    }
    
    sampleRate   = (t_int)atom_getFloatAtIndex (16, argc, argv);
    advance      = (t_int)atom_getFloatAtIndex (17, argc, argv);
    withCallback = (t_int)atom_getFloatAtIndex (18, argc, argv);
    blockSize    = (t_int)atom_getFloatAtIndex (19, argc, argv);
    
    withCallback = (withCallback > 0);
    
    /* Remove devices with number of channels set to zero. */
    
    for (t = 0; t < 4; t++) { if (j[t] != 0) { i[m] = i[t]; j[m] = j[t]; m++; } }
    for (t = 0; t < 4; t++) { if (p[t] != 0) { o[n] = o[t]; p[n] = p[t]; n++; } }
    
    audio_close();
        
    audio_setDevicesAndParameters (m, i, j, n, o, p, sampleRate, advance, withCallback, blockSize);
        
    if (withCallback) { scheduler_needToRestart(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
