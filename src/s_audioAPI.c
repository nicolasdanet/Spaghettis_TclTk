
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

#define AUDIO_DEFAULT_CHANNELS  2

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
static int      audio_nextChannelsIn;                                               /* Shared. */
static int      audio_nextChannelsOut;                                              /* Shared. */

static int      audio_numberOfDevicesIn;                                            /* Shared. */
static int      audio_devicesIn[MAXIMUM_AUDIO_IN];                                  /* Shared. */
static int      audio_devicesInChannels[MAXIMUM_AUDIO_IN];                          /* Shared. */
static char     audio_devicesInNames[MAXIMUM_MIDI_IN * MAXIMUM_DESCRIPTION];        /* Shared. */

static int      audio_numberOfDevicesOut;                                           /* Shared. */
static int      audio_devicesOut[MAXIMUM_AUDIO_OUT];                                /* Shared. */
static int      audio_devicesOutChannels[MAXIMUM_AUDIO_OUT];                        /* Shared. */
static char     audio_devicesOutNames[MAXIMUM_AUDIO_OUT * MAXIMUM_DESCRIPTION];     /* Shared. */

static int      audio_tempSampleRate;                                               /* Shared. */
static int      audio_tempAdvance;                                                  /* Shared. */
static int      audio_tempHasCallback;                                              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_setAPI (void *dummy, t_float f)
{
    int api = (audio_isAPIAvailable ((int)f) ? (int)f : API_DEFAULT_AUDIO);
    
    if (api == API_NONE) { if (audio_isOpened()) { audio_close(); } }
    else {
    //
    if (api != audio_api) {
        audio_close();
        audio_api = (int)f;
        audio_numberOfDevicesIn     = 0;
        audio_numberOfDevicesOut    = 0;
        audio_devicesIn[0]          = AUDIO_DEFAULT_DEVICE;
        audio_devicesOut[0]         = AUDIO_DEFAULT_DEVICE;
        audio_devicesInChannels[0]  = AUDIO_DEFAULT_CHANNELS;
        audio_devicesOutChannels[0] = AUDIO_DEFAULT_CHANNELS;
        audio_open();
    }
    //
    }
}

t_error audio_getAPIAvailables (char *dest, size_t size)
{
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (dest, size, "{ ");
    
    #ifdef USEAPI_OSS
        err |= string_addSprintf (dest, size, "{OSS %d} ", API_OSS);
    #endif

    #ifdef USEAPI_MMIO
        err |= string_addSprintf (dest, size, "{MMIO %d} ", API_MMIO);
    #endif
    
    #ifdef USEAPI_ALSA
        err |= string_addSprintf (dest, size, "{ALSA %d} ", API_ALSA);
    #endif

    #ifdef USEAPI_PORTAUDIO
        err |= string_addSprintf (dest, size, "{PortAudio %d} ", API_PORTAUDIO);
    #endif
    
    #ifdef USEAPI_JACK
        err |= string_addSprintf (dest, size, "{JACK %d} ", API_JACK);
    #endif
    
    #ifdef USEAPI_DUMMY
        err |= string_addSprintf (dest, size, "{Dummy %d} ", API_DUMMY);
    #endif
    
    err |= string_add (dest, size, "}");
        
    return err;
}

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

void audio_open (void)
{
    int m = 0;
    int n = 0;
    int i[MAXIMUM_AUDIO_IN]  = { 0 };
    int j[MAXIMUM_AUDIO_IN]  = { 0 };
    int o[MAXIMUM_AUDIO_OUT] = { 0 };
    int p[MAXIMUM_AUDIO_OUT] = { 0 };
    
    int sampleRate;
    int advance;
    int callback;
    int blockSize;
    
    t_error err = PD_ERROR;
    
    audio_getDevices (&m, i, j, &n, o, p, &sampleRate, &advance, &callback, &blockSize);
    sys_setchsr (audio_nextChannelsIn, audio_nextChannelsOut, sampleRate);
    
    if (!m && !n) { scheduler_setAudioMode (SCHEDULER_AUDIO_NONE); return; }
    
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
                (callback ? scheduler_audioCallback : NULL));
               
    } else if (API_WITH_JACK && audio_api == API_JACK)      {
    
        err = jack_open_audio ((m > 0 ? j[0] : 0),
                (n > 0 ? p[0] : 0),
                sampleRate,
                (callback ? scheduler_audioCallback : NULL));

    } else if (API_WITH_OSS && audio_api == API_OSS)        {
    
        err = oss_open_audio (m, i, m, j, n, o, n, p, sampleRate, audio_blockSize);
        
    } else if (API_WITH_ALSA && audio_api == API_ALSA)      {
    
        err = alsa_open_audio (m, i, m, j, n, o, n, p, sampleRate, audio_blockSize);
        
    } else if (API_WITH_MMIO && audio_api == API_MMIO)      {
    
        err = mmio_open_audio (m, i, m, j, n, o, n, p, sampleRate, audio_blockSize);
        
    } else if (API_WITH_DUMMY && audio_api == API_DUMMY)    {
    
        err = dummy_open_audio();
        
    } else if (audio_api != API_NONE) { PD_BUG; }
    
    if (err) {
        audio_state = 0;
        audio_openedApi = -1;
        audio_callbackIsOpen = 0;
        scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
        sys_vGui ("set ::var(apiAudio) %d\n", 0);
        
    } else {
        audio_state = 1;
        audio_openedApi = audio_api;
        audio_callbackIsOpen = callback;
        scheduler_setAudioMode ((callback ? SCHEDULER_AUDIO_CALLBACK : SCHEDULER_AUDIO_POLL));
        sys_vGui ("set ::var(apiAudio) %d\n", audio_api);
    }
}

void audio_close (void)
{
    if (!audio_isOpened()) { return; }

    if (API_WITH_PORTAUDIO  && audio_openedApi == API_PORTAUDIO)    { pa_close_audio();     }
    else if (API_WITH_JACK  && audio_openedApi == API_JACK)         { jack_close_audio();   }
    else if (API_WITH_OSS   && audio_openedApi == API_OSS)          { oss_close_audio();    }
    else if (API_WITH_ALSA  && audio_openedApi == API_ALSA)         { alsa_close_audio();   }
    else if (API_WITH_MMIO  && audio_openedApi == API_MMIO)         { mmio_close_audio();   }
    else if (API_WITH_DUMMY && audio_openedApi == API_DUMMY)        { dummy_close_audio();  }
    else {
        PD_BUG;
    }
    
    audio_openedApi         = -1;
    audio_state             = 0;
    audio_channelsIn        = 0;
    audio_channelsOut       = 0;
    audio_callbackIsOpen    = 0;
    
    scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
}

int audio_isOpened (void)
{
    if (audio_state) { 
    //
    if (audio_numberOfDevicesIn > 0 && audio_devicesInChannels[0] > 0)   { return 1; }
    if (audio_numberOfDevicesOut > 0 && audio_devicesOutChannels[0] > 0) { return 1; }
    //
    }
            
    return 0;
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
    int *hasCallback,
    int *blockSize)
{
    int i, n;
    
    *numberOfDevicesIn = audio_numberOfDevicesIn;
    *numberOfDevicesOut = audio_numberOfDevicesOut;
    
    for (i = 0; i < audio_numberOfDevicesIn; i++) {
        if ((n = audio_numberWithName (0, &audio_devicesInNames[i * MAXIMUM_DESCRIPTION])) >= 0) {
            devicesIn[i] = n;
        } else {
            devicesIn[i] = audio_devicesIn[i];
        }
        
        channelsIn[i] = audio_devicesInChannels[i];
    }
    
    for (i = 0; i < audio_numberOfDevicesOut; i++) {
        if ((n = audio_numberWithName(1, &audio_devicesOutNames[i * MAXIMUM_DESCRIPTION])) >= 0) {
            devicesOut[i] = n;
        } else { 
            devicesOut[i] = audio_devicesOut[i];
        }
        
        channelsOut[i] = audio_devicesOutChannels[i]; 
    }
    
    *sampleRate  = audio_tempSampleRate;
    *advance     = audio_tempAdvance;
    *hasCallback = audio_tempHasCallback;
    *blockSize   = audio_blockSize;
}

static void audio_setDevices (int numberOfDevicesIn, 
    int *devicesIn,
    int *channelsIn,
    int numberOfDevicesOut,
    int *devicesOut,
    int *channelsOut,
    int sampleRate,
    int advance,
    int hasCallback,
    int blockSize)
{
    int i;
    
    audio_numberOfDevicesIn = numberOfDevicesIn;
    audio_numberOfDevicesOut = numberOfDevicesOut;
        
    for (i = 0; i < numberOfDevicesIn; i++) {
        char *s = &audio_devicesInNames[i * MAXIMUM_DESCRIPTION];
        audio_devicesIn[i] = devicesIn[i],
        audio_devicesInChannels[i] = channelsIn[i];
        audio_numberToName (0, devicesIn[i], s, MAXIMUM_DESCRIPTION);
    }

    for (i = 0; i < numberOfDevicesOut; i++) {
        char *s = &audio_devicesOutNames[i * MAXIMUM_DESCRIPTION];
        audio_devicesOut[i] = devicesOut[i],
        audio_devicesOutChannels[i] = channelsOut[i];
        audio_numberToName(1, devicesOut[i], s, MAXIMUM_DESCRIPTION);
    }
    
    audio_tempSampleRate  = sampleRate;
    audio_tempAdvance     = advance;
    audio_tempHasCallback = hasCallback;
    audio_blockSize       = blockSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error audio_getLists (char *i, int *m, char *o, int *n, int *canMultiple, int *canCallback)
{
    int k = audio_api;
    
    if (API_WITH_PORTAUDIO && k == API_PORTAUDIO) { pa_getdevs (i, m, o, n, canMultiple, canCallback);    }
    else if (API_WITH_JACK && k == API_JACK)      { jack_getdevs (i, m, o, n, canMultiple, canCallback);  }
    else if (API_WITH_OSS && k == API_OSS)        { oss_getdevs (i, m, o, n, canMultiple, canCallback);   }
    else if (API_WITH_ALSA && k == API_ALSA)      { alsa_getdevs (i, m, o, n, canMultiple, canCallback);  }
    else if (API_WITH_MMIO && k == API_MMIO)      { mmio_getdevs (i, m, o, n, canMultiple, canCallback);  }
    else if (API_WITH_DUMMY && k == API_DUMMY)    { dummy_getdevs (i, m, o, n, canMultiple, canCallback); }
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

void audio_numberToName (int isOutput, int k, char *dest, size_t size)
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
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------------------- public routines ----------------------- */

    /* set audio device settings (after cleaning up the specified device and
    channel vectors).  The audio devices are "zero based" (i.e. "0" means the
    first one.)  We can later re-open audio and/or show the settings on a
    dialog window. */

void sys_set_audio_settings(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate, int advance, int callback, int blocksize)
{
    int i, *ip;
    int defaultchannels = AUDIO_DEFAULT_CHANNELS;
    int inchans, outchans, nrealindev, nrealoutdev;
    int realindev[MAXIMUM_AUDIO_IN], realoutdev[MAXIMUM_AUDIO_OUT];
    int realinchans[MAXIMUM_AUDIO_IN], realoutchans[MAXIMUM_AUDIO_OUT];

    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int indevs = 0, outdevs = 0, canmulti = 0, cancallback = 0;
    audio_getLists(indevlist, &indevs, outdevlist, &outdevs, &canmulti,
        &cancallback);

    if (rate < 1)
        rate = AUDIO_DEFAULT_SAMPLING;
    if (advance < 0)
        advance = AUDIO_DEFAULT_ADVANCE;
    if (blocksize != (1 << ilog2(blocksize)) || blocksize < AUDIO_DEFAULT_BLOCK)
        blocksize = AUDIO_DEFAULT_BLOCK;
        /* Since the channel vector might be longer than the
        audio device vector, or vice versa, we fill the shorter one
        in to match the longer one.  Also, if both are empty, we fill in
        one device (the default) and two channels. */ 
    if (naudioindev == -1)
    {           /* no input audio devices specified */
        if (nchindev == -1)
        {
            if (indevs >= 1)
            {
                nchindev=1;
                chindev[0] = defaultchannels;
                naudioindev = 1;
                audioindev[0] = AUDIO_DEFAULT_DEVICE;
            }
            else naudioindev = nchindev = 0;
        }
        else
        {
            for (i = 0; i < MAXIMUM_AUDIO_IN; i++)
                audioindev[i] = i;
            naudioindev = nchindev;
        }
    }
    else
    {
        if (nchindev == -1)
        {
            nchindev = naudioindev;
            for (i = 0; i < naudioindev; i++)
                chindev[i] = defaultchannels;
        }
        else if (nchindev > naudioindev)
        {
            for (i = naudioindev; i < nchindev; i++)
            {
                if (i == 0)
                    audioindev[0] = AUDIO_DEFAULT_DEVICE;
                else audioindev[i] = audioindev[i-1] + 1;
            }
            naudioindev = nchindev;
        }
        else if (nchindev < naudioindev)
        {
            for (i = nchindev; i < naudioindev; i++)
            {
                if (i == 0)
                    chindev[0] = defaultchannels;
                else chindev[i] = chindev[i-1];
            }
            naudioindev = nchindev;
        }
    }

    if (naudiooutdev == -1)
    {           /* not set */
        if (nchoutdev == -1)
        {
            if (outdevs >= 1)
            {
                nchoutdev=1;
                choutdev[0]=defaultchannels;
                naudiooutdev=1;
                audiooutdev[0] = AUDIO_DEFAULT_DEVICE;
            }
            else nchoutdev = naudiooutdev = 0;
        }
        else
        {
            for (i = 0; i < MAXIMUM_AUDIO_OUT; i++)
                audiooutdev[i] = i;
            naudiooutdev = nchoutdev;
        }
    }
    else
    {
        if (nchoutdev == -1)
        {
            nchoutdev = naudiooutdev;
            for (i = 0; i < naudiooutdev; i++)
                choutdev[i] = defaultchannels;
        }
        else if (nchoutdev > naudiooutdev)
        {
            for (i = naudiooutdev; i < nchoutdev; i++)
            {
                if (i == 0)
                    audiooutdev[0] = AUDIO_DEFAULT_DEVICE;
                else audiooutdev[i] = audiooutdev[i-1] + 1;
            }
            naudiooutdev = nchoutdev;
        }
        else if (nchoutdev < naudiooutdev)
        {
            for (i = nchoutdev; i < naudiooutdev; i++)
            {
                if (i == 0)
                    choutdev[0] = defaultchannels;
                else choutdev[i] = choutdev[i-1];
            }
            naudiooutdev = nchoutdev;
        }
    }
    
        /* count total number of input and output channels */
    for (i = nrealindev = inchans = 0; i < naudioindev; i++)
        if (chindev[i] > 0)
    {
        realinchans[nrealindev] = chindev[i];
        realindev[nrealindev] = audioindev[i];
        inchans += chindev[i];
        nrealindev++;
    }
    for (i = nrealoutdev = outchans = 0; i < naudiooutdev; i++)
        if (choutdev[i] > 0)
    {
        realoutchans[nrealoutdev] = choutdev[i];
        realoutdev[nrealoutdev] = audiooutdev[i];
        outchans += choutdev[i];
        nrealoutdev++;
    }
    audio_advanceInMicroseconds = advance * 1000;
    //sys_log_error(ERROR_NONE);
    audio_nextChannelsIn = inchans;
    audio_nextChannelsOut = outchans;
    sys_setchsr(audio_nextChannelsIn, audio_nextChannelsOut, rate);
    audio_setDevices(nrealindev, realindev, realinchans,
        nrealoutdev, realoutdev, realoutchans, rate, advance, callback,
            blocksize);
}

    /* start an audio settings dialog window */
void global_audioProperties(void *dummy, t_float flongform)
{
    char buf[1024 + 2 * MAXIMUM_DEVICES*(MAXIMUM_DESCRIPTION+4)];
        /* these are the devices you're using: */
    int naudioindev, audioindev[MAXIMUM_AUDIO_IN], chindev[MAXIMUM_AUDIO_IN];
    int naudiooutdev, audiooutdev[MAXIMUM_AUDIO_OUT], choutdev[MAXIMUM_AUDIO_OUT];
    int audioindev1, audioindev2, audioindev3, audioindev4,
        audioinchan1, audioinchan2, audioinchan3, audioinchan4,
        audiooutdev1, audiooutdev2, audiooutdev3, audiooutdev4,
        audiooutchan1, audiooutchan2, audiooutchan3, audiooutchan4;
    int rate, advance, callback, blocksize;
        /* these are all the devices on your system: */
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, canmulti = 0, cancallback = 0, i;

    audio_getLists(indevlist, &nindevs, outdevlist, &noutdevs, &canmulti,
         &cancallback);

    sys_gui("set ::ui_audio::audioIn {}\n");
    for (i = 0; i < nindevs; i++)
        sys_vGui("lappend ::ui_audio::audioIn {%s}\n",
            indevlist + i * MAXIMUM_DESCRIPTION);

    sys_gui("set ::ui_audio::audioOut {}\n");
    for (i = 0; i < noutdevs; i++)
        sys_vGui("lappend ::ui_audio::audioOut {%s}\n",
            outdevlist + i * MAXIMUM_DESCRIPTION);

    audio_getDevices(&naudioindev, audioindev, chindev,
        &naudiooutdev, audiooutdev, choutdev, &rate, &advance, &callback,
            &blocksize);

    /* post("naudioindev %d naudiooutdev %d longform %f",
            naudioindev, naudiooutdev, flongform); */
    if (naudioindev > 1 || naudiooutdev > 1)
        flongform = 1;

    audioindev1 = (naudioindev > 0 &&  audioindev[0]>= 0 ? audioindev[0] : 0);
    audioindev2 = (naudioindev > 1 &&  audioindev[1]>= 0 ? audioindev[1] : 0);
    audioindev3 = (naudioindev > 2 &&  audioindev[2]>= 0 ? audioindev[2] : 0);
    audioindev4 = (naudioindev > 3 &&  audioindev[3]>= 0 ? audioindev[3] : 0);
    audioinchan1 = (naudioindev > 0 ? chindev[0] : 0);
    audioinchan2 = (naudioindev > 1 ? chindev[1] : 0);
    audioinchan3 = (naudioindev > 2 ? chindev[2] : 0);
    audioinchan4 = (naudioindev > 3 ? chindev[3] : 0);
    audiooutdev1 = (naudiooutdev > 0 && audiooutdev[0]>=0 ? audiooutdev[0] : 0);  
    audiooutdev2 = (naudiooutdev > 1 && audiooutdev[1]>=0 ? audiooutdev[1] : 0);  
    audiooutdev3 = (naudiooutdev > 2 && audiooutdev[2]>=0 ? audiooutdev[2] : 0);  
    audiooutdev4 = (naudiooutdev > 3 && audiooutdev[3]>=0 ? audiooutdev[3] : 0);  
    audiooutchan1 = (naudiooutdev > 0 ? choutdev[0] : 0);
    audiooutchan2 = (naudiooutdev > 1 ? choutdev[1] : 0);
    audiooutchan3 = (naudiooutdev > 2 ? choutdev[2] : 0);
    audiooutchan4 = (naudiooutdev > 3 ? choutdev[3] : 0);
    sprintf(buf,
"::ui_audio::show %%s \
%d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d \
%d %d %d %d %d\n",
        audioindev1, audioindev2, audioindev3, audioindev4, 
        audioinchan1, audioinchan2, audioinchan3, audioinchan4, 
        audiooutdev1, audiooutdev2, audiooutdev3, audiooutdev4,
        audiooutchan1, audiooutchan2, audiooutchan3, audiooutchan4, 
        rate, advance, canmulti, (cancallback ? callback : -1),
        blocksize);
    gfxstub_deleteforkey(0);
    gfxstub_new(&global_object, (void *)global_audioProperties, buf);
}

    /* new values from dialog window */
void global_audioDialog(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int naudioindev, audioindev[MAXIMUM_AUDIO_IN], chindev[MAXIMUM_AUDIO_IN];
    int naudiooutdev, audiooutdev[MAXIMUM_AUDIO_OUT], choutdev[MAXIMUM_AUDIO_OUT];
    int rate, advance, audioon, i, nindev, noutdev;
    int audioindev1, audioinchan1, audiooutdev1, audiooutchan1;
    int newaudioindev[4], newaudioinchan[4],
        newaudiooutdev[4], newaudiooutchan[4];
        /* the new values the dialog came back with: */
    int newrate = (t_int)atom_getFloatAtIndex(16, argc, argv);
    int newadvance = (t_int)atom_getFloatAtIndex(17, argc, argv);
    int newcallback = (t_int)atom_getFloatAtIndex(18, argc, argv);
    int newblocksize = (t_int)atom_getFloatAtIndex(19, argc, argv);

    for (i = 0; i < 4; i++)
    {
        newaudioindev[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
        newaudioinchan[i] = (t_int)atom_getFloatAtIndex(i+4, argc, argv);
        newaudiooutdev[i] = (t_int)atom_getFloatAtIndex(i+8, argc, argv);
        newaudiooutchan[i] = (t_int)atom_getFloatAtIndex(i+12, argc, argv);
    }

    for (i = 0, nindev = 0; i < 4; i++)
    {
        if (newaudioinchan[i])
        {
            newaudioindev[nindev] = newaudioindev[i];
            newaudioinchan[nindev] = newaudioinchan[i];
            /* post("in %d %d %d", nindev,
                newaudioindev[nindev] , newaudioinchan[nindev]); */
            nindev++;
        }
    }
    for (i = 0, noutdev = 0; i < 4; i++)
    {
        if (newaudiooutchan[i])
        {
            newaudiooutdev[noutdev] = newaudiooutdev[i];
            newaudiooutchan[noutdev] = newaudiooutchan[i];
            /* post("out %d %d %d", noutdev,
                newaudiooutdev[noutdev] , newaudioinchan[noutdev]); */
            noutdev++;
        }
    }
    
    sys_set_audio_settings_reopen(nindev, newaudioindev, nindev, newaudioinchan,
        noutdev, newaudiooutdev, noutdev, newaudiooutchan,
        newrate, newadvance, newcallback, newblocksize);
}

void sys_set_audio_settings_reopen(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate, int advance, int callback, int newblocksize)
{
    if (callback < 0)
        callback = 0;
    if (newblocksize != (1<<ilog2(newblocksize)) ||
        newblocksize < AUDIO_DEFAULT_BLOCK || newblocksize > 2048)
            newblocksize = AUDIO_DEFAULT_BLOCK;
    
    if (!audio_callbackIsOpen && !callback)
        audio_close();
    sys_set_audio_settings(naudioindev, audioindev, nchindev, chindev,
        naudiooutdev, audiooutdev, nchoutdev, choutdev,
        rate, advance, (callback >= 0 ? callback : 0), newblocksize);
    if (!audio_callbackIsOpen && !callback)
        audio_open();
    else scheduler_needToRestart();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
