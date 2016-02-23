
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
#pragma mark -

extern  t_class *global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_sample    *audio_soundIn;                                                     /* Shared. */
t_sample    *audio_soundOut;                                                    /* Shared. */

int         audio_channelsIn;                                                   /* Shared. */
int         audio_channelsOut;                                                  /* Shared. */
int         audio_advanceInSamples;                                             /* Shared. */
int         audio_advanceInMicroseconds;                                        /* Shared. */
int         audio_blockSize;                                                    /* Shared. */
t_float     audio_sampleRate;                                                   /* Shared. */
int         audio_api                   = API_DEFAULT;                          /* Shared. */
int         audio_previouslyOpenedApi   = -1;                                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  audio_state;                                                        /* Shared. */
static int  audio_callbackIsOpen;                                               /* Shared. */
static int  audio_nextChannelsIn;                                               /* Shared. */
static int  audio_nextChannelsOut;                                              /* Shared. */

static int  audio_numberOfDevicesIn;                                            /* Shared. */
static int  audio_devicesIn[MAXIMUM_AUDIO_IN];                                  /* Shared. */
static int  audio_devicesInChannels[MAXIMUM_AUDIO_IN];                          /* Shared. */
static char audio_devicesInNames[MAXIMUM_MIDI_IN * MAXIMUM_DESCRIPTION];        /* Shared. */

static int  audio_numberOfDevicesOut;                                           /* Shared. */
static int  audio_devicesOut[MAXIMUM_AUDIO_OUT];                                /* Shared. */
static int  audio_devicesOutChannels[MAXIMUM_AUDIO_OUT];                        /* Shared. */
static char audio_devicesOutNames[MAXIMUM_AUDIO_OUT * MAXIMUM_DESCRIPTION];     /* Shared. */

static int  audio_tempRate;                                                     /* Shared. */
static int  audio_tempAdvance;                                                  /* Shared. */
static int  audio_tempCallback;                                                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int audio_isopen (void)
{
    return (audio_state &&
        ((audio_numberOfDevicesIn > 0 && audio_devicesInChannels[0] > 0) 
            || (audio_numberOfDevicesOut > 0 && audio_devicesOutChannels[0] > 0)));
}

void sys_get_audio_params(
    int *pnaudioindev, int *paudioindev, int *chindev,
    int *pnaudiooutdev, int *paudiooutdev, int *choutdev,
    int *prate, int *padvance, int *pcallback, int *pblocksize)
{
    int i, devn;
    *pnaudioindev = audio_numberOfDevicesIn;
    for (i = 0; i < audio_numberOfDevicesIn; i++)
    {
        if ((devn = sys_audiodevnametonumber(0,
            &audio_devicesInNames[i * MAXIMUM_DESCRIPTION])) >= 0)
                paudioindev[i] = devn;
        else paudioindev[i] = audio_devicesIn[i];
        chindev[i] = audio_devicesInChannels[i];
    }
    *pnaudiooutdev = audio_numberOfDevicesOut;
    for (i = 0; i < audio_numberOfDevicesOut; i++)
    {
        if ((devn = sys_audiodevnametonumber(1,
            &audio_devicesOutNames[i * MAXIMUM_DESCRIPTION])) >= 0)
                paudiooutdev[i] = devn;
        else paudiooutdev[i] = audio_devicesOut[i];
        choutdev[i] = audio_devicesOutChannels[i]; 
    }
    *prate = audio_tempRate;
    *padvance = audio_tempAdvance;
    *pcallback = audio_tempCallback;
    *pblocksize = audio_blockSize;
}

void sys_save_audio_params(
    int naudioindev, int *audioindev, int *chindev,
    int naudiooutdev, int *audiooutdev, int *choutdev,
    int rate, int advance, int callback, int blocksize)
{
    int i;
    audio_numberOfDevicesIn = naudioindev;
    for (i = 0; i < naudioindev; i++)
    {
        audio_devicesIn[i] = audioindev[i],
        audio_devicesInChannels[i] = chindev[i];
        sys_audiodevnumbertoname(0, audioindev[i],
            &audio_devicesInNames[i * MAXIMUM_DESCRIPTION], MAXIMUM_DESCRIPTION);
    }
    audio_numberOfDevicesOut = naudiooutdev;
    for (i = 0; i < naudiooutdev; i++)
    {
        audio_devicesOut[i] = audiooutdev[i],
        audio_devicesOutChannels[i] = choutdev[i];
        sys_audiodevnumbertoname(1, audiooutdev[i],
            &audio_devicesOutNames[i * MAXIMUM_DESCRIPTION], MAXIMUM_DESCRIPTION);
    }
    
    audio_tempRate = rate;
    audio_tempAdvance = advance;
    audio_tempCallback = callback;
    audio_blockSize = blocksize;
}

static void audio_init( void)
{
    static int initted = 0;
    if (initted)
        return;
    initted = 1;
#ifdef USEAPI_OSS
    oss_init();
#endif
} 

    /* set channels and sample rate.  */

void sys_setchsr(int chin, int chout, int sr)
{
    int nblk;
    int inbytes = (chin ? chin : 2) *
                (AUDIO_DEFAULT_BLOCK*sizeof(t_sample));
    int outbytes = (chout ? chout : 2) *
                (AUDIO_DEFAULT_BLOCK*sizeof(t_sample));

    if (audio_soundIn)
        PD_MEMORY_FREE(audio_soundIn);
    if (audio_soundOut)
        PD_MEMORY_FREE(audio_soundOut);
    audio_channelsIn = chin;
    audio_channelsOut = chout;
    audio_sampleRate = sr;
    audio_advanceInSamples = (audio_advanceInMicroseconds * audio_sampleRate) / (1000000.);
    if (audio_advanceInSamples < AUDIO_DEFAULT_BLOCK)
        audio_advanceInSamples = AUDIO_DEFAULT_BLOCK;

    audio_soundIn = (t_sample *)PD_MEMORY_GET(inbytes);
    memset(audio_soundIn, 0, inbytes);

    audio_soundOut = (t_sample *)PD_MEMORY_GET(outbytes);
    memset(audio_soundOut, 0, outbytes);

    if (0)
        post("input channels = %d, output channels = %d",
            audio_channelsIn, audio_channelsOut);
    canvas_resume_dsp(canvas_suspend_dsp());
}

void sys_close_audio(void)
{
    if (!audio_isopen())
        return;
#ifdef USEAPI_PORTAUDIO
    if (audio_previouslyOpenedApi == API_PORTAUDIO)
        pa_close_audio();
    else 
#endif
#ifdef USEAPI_JACK
    if (audio_previouslyOpenedApi == API_JACK)
        jack_close_audio();
    else
#endif
#ifdef USEAPI_OSS
    if (audio_previouslyOpenedApi == API_OSS)
        oss_close_audio();
    else
#endif
#ifdef USEAPI_ALSA
    if (audio_previouslyOpenedApi == API_ALSA)
        alsa_close_audio();
    else
#endif
#ifdef USEAPI_MMIO
    if (audio_previouslyOpenedApi == API_MMIO)
        mmio_close_audio();
    else
#endif
#ifdef USEAPI_DUMMY
    if (audio_previouslyOpenedApi == API_DUMMY)
        dummy_close_audio();
    else
#endif
        post("sys_close_audio: unknown API %d", audio_previouslyOpenedApi);
    audio_channelsIn = audio_channelsOut = 0;
    audio_previouslyOpenedApi = -1;
    scheduler_setAudioMode(SCHEDULER_AUDIO_NONE);
    audio_state = 0;
    audio_callbackIsOpen = 0;

    // sys_vGui("set ::var(apiAudio) 0\n");
}

    /* open audio using whatever parameters were last used */
void sys_reopen_audio( void)
{
    int naudioindev, audioindev[MAXIMUM_AUDIO_IN], chindev[MAXIMUM_AUDIO_IN];
    int naudiooutdev, audiooutdev[MAXIMUM_AUDIO_OUT], choutdev[MAXIMUM_AUDIO_OUT];
    int rate, advance, callback, blocksize, outcome = 0;
    sys_get_audio_params(&naudioindev, audioindev, chindev,
        &naudiooutdev, audiooutdev, choutdev, &rate, &advance, &callback,
            &blocksize);
    sys_setchsr(audio_nextChannelsIn, audio_nextChannelsOut, rate);
    if (!naudioindev && !naudiooutdev)
    {
        scheduler_setAudioMode(SCHEDULER_AUDIO_NONE);
        return;
    }
#ifdef USEAPI_PORTAUDIO
    if (audio_api == API_PORTAUDIO)
    {
        int blksize = (audio_blockSize ? audio_blockSize : 64);
        if (0)
            fprintf(stderr, "blksize %d, advance %d\n", blksize, audio_advanceInSamples/blksize);
        outcome = pa_open_audio((naudioindev > 0 ? chindev[0] : 0),
        (naudiooutdev > 0 ? choutdev[0] : 0), rate, audio_soundIn,
            audio_soundOut, blksize, audio_advanceInSamples/blksize, 
             (naudioindev > 0 ? audioindev[0] : 0),
              (naudiooutdev > 0 ? audiooutdev[0] : 0),
               (callback ? scheduler_audioCallback : 0));
    }
    else
#endif
#ifdef USEAPI_JACK
    if (audio_api == API_JACK) 
        outcome = jack_open_audio((naudioindev > 0 ? chindev[0] : 0),
            (naudiooutdev > 0 ? choutdev[0] : 0), rate,
                (callback ? scheduler_audioCallback : 0));

    else
#endif    
#ifdef USEAPI_OSS
    if (audio_api == API_OSS)
        outcome = oss_open_audio(naudioindev, audioindev, naudioindev,
            chindev, naudiooutdev, audiooutdev, naudiooutdev, choutdev, rate,
                audio_blockSize);
    else
#endif
#ifdef USEAPI_ALSA
        /* for alsa, only one device is supported; it may
        be open for both input and output. */
    if (audio_api == API_ALSA)
        outcome = alsa_open_audio(naudioindev, audioindev, naudioindev,
            chindev, naudiooutdev, audiooutdev, naudiooutdev, choutdev, rate,
                audio_blockSize);
    else 
#endif
#ifdef USEAPI_MMIO
    if (audio_api == API_MMIO)
        outcome = mmio_open_audio(naudioindev, audioindev, naudioindev,
            chindev, naudiooutdev, audiooutdev, naudiooutdev, choutdev, rate,
                audio_blockSize);
    else
#endif
#ifdef USEAPI_DUMMY
    if (audio_api == API_DUMMY)
        outcome = dummy_open_audio(naudioindev, naudiooutdev, rate);
    else
#endif
    if (audio_api == API_NONE)
        ;
    else post("unknown audio API specified");
    if (outcome)    /* failed */
    {
        audio_state = 0;
        scheduler_setAudioMode(SCHEDULER_AUDIO_NONE);
        audio_previouslyOpenedApi = -1;
        audio_callbackIsOpen = 0;
    }
    else
    {
                /* fprintf(stderr, "started w/callback %d\n", callback); */
        audio_state = 1;
        scheduler_setAudioMode(
            (callback ? SCHEDULER_AUDIO_CALLBACK : SCHEDULER_AUDIO_POLL));
        audio_previouslyOpenedApi = audio_api;
        audio_callbackIsOpen = callback;
    }
    sys_vGui("set ::var(apiAudio) %d\n",  (outcome == 0 ? audio_api : 0));
}

int sys_send_dacs(void)
{
    #if 0
    if (0 /*sys_meters*/)
    {
        int i, n;
        t_sample maxsamp;
        for (i = 0, n = audio_channelsIn * AUDIO_DEFAULT_BLOCK, maxsamp = sys_inmax;
            i < n; i++)
        {
            t_sample f = audio_soundIn[i];
            if (f > maxsamp) maxsamp = f;
            else if (-f > maxsamp) maxsamp = -f;
        }
        sys_inmax = maxsamp;
        for (i = 0, n = audio_channelsOut * AUDIO_DEFAULT_BLOCK, maxsamp = sys_outmax;
            i < n; i++)
        {
            t_sample f = audio_soundOut[i];
            if (f > maxsamp) maxsamp = f;
            else if (-f > maxsamp) maxsamp = -f;
        }
        sys_outmax = maxsamp;
    }
    #endif
    
#ifdef USEAPI_PORTAUDIO
    if (audio_api == API_PORTAUDIO)
        return (pa_send_dacs());
    else 
#endif
#ifdef USEAPI_JACK
      if (audio_api == API_JACK) 
        return (jack_send_dacs());
    else
#endif
#ifdef USEAPI_OSS
    if (audio_api == API_OSS)
        return (oss_send_dacs());
    else
#endif
#ifdef USEAPI_ALSA
    if (audio_api == API_ALSA)
        return (alsa_send_dacs());
    else
#endif
#ifdef USEAPI_MMIO
    if (audio_api == API_MMIO)
        return (mmio_send_dacs());
    else
#endif
#ifdef USEAPI_DUMMY
    if (audio_api == API_DUMMY)
        return (dummy_send_dacs());
    else
#endif
    post("unknown API");    
    return (0);
}

t_float sys_getsr(void)
{
     return (audio_sampleRate);
}

int sys_get_outchannels(void)
{
     return (audio_channelsOut); 
}

int sys_get_inchannels(void) 
{
     return (audio_channelsIn);
}
/*
void sys_getmeters(t_sample *inmax, t_sample *outmax)
{
    if (inmax)
    {
        sys_meters = 1;
        *inmax = sys_inmax;
        *outmax = sys_outmax;
    }
    else
        sys_meters = 0;
    sys_inmax = sys_outmax = 0;
}
*/
/* this could later be set by a preference but for now it seems OK to just
keep jack audio open but close unused audio devices for any other API */
int audio_shouldkeepopen( void)
{
    return (audio_api == API_JACK);
}

static void audio_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, int *cancallback,
        int maxndev, int devdescsize)
{
    audio_init();
    *cancallback = 0;   /* may be overridden by specific API implementation */
#ifdef USEAPI_PORTAUDIO
    if (audio_api == API_PORTAUDIO)
    {
        pa_getdevs(indevlist, nindevs, outdevlist, noutdevs, canmulti,
            maxndev, devdescsize);
        *cancallback = 1;
    }
    else
#endif
#ifdef USEAPI_JACK
    if (audio_api == API_JACK)
    {
        jack_getdevs(indevlist, nindevs, outdevlist, noutdevs, canmulti,
            maxndev, devdescsize);
        *cancallback = 1;
    }
    else
#endif
#ifdef USEAPI_OSS
    if (audio_api == API_OSS)
    {
        oss_getdevs(indevlist, nindevs, outdevlist, noutdevs, canmulti,
            maxndev, devdescsize);
    }
    else
#endif
#ifdef USEAPI_ALSA
    if (audio_api == API_ALSA)
    {
        alsa_getdevs(indevlist, nindevs, outdevlist, noutdevs, canmulti,
            maxndev, devdescsize);
    }
    else
#endif
#ifdef USEAPI_MMIO
    if (audio_api == API_MMIO)
    {
        mmio_getdevs(indevlist, nindevs, outdevlist, noutdevs, canmulti,
            maxndev, devdescsize);
    }
    else
#endif
#ifdef USEAPI_DUMMY
    if (audio_api == API_DUMMY)
    {
        dummy_getdevs(indevlist, nindevs, outdevlist, noutdevs, canmulti,
            maxndev, devdescsize);
    }
    else
#endif
    {
            /* this shouldn't happen once all the above get filled in. */
        int i;
        *nindevs = *noutdevs = 3;
        for (i = 0; i < 3; i++)
        {
            sprintf(indevlist + i * devdescsize, "input device #%d", i+1);
            sprintf(outdevlist + i * devdescsize, "output device #%d", i+1);
        }
        *canmulti = 0;
    }
}


static void sys_listaudiodevs(void )
{
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i, canmulti = 0, cancallback = 0;

    audio_getdevs(indevlist, &nindevs, outdevlist, &noutdevs, &canmulti,
        &cancallback, MAXIMUM_DEVICES, MAXIMUM_DESCRIPTION);

    if (!nindevs)
        post("no audio input devices found");
    else
    {
            /* To agree with command line flags, normally start at 1 */
            /* But microsoft "MMIO" device list starts at 0 (the "mapper"). */
            /* (see also sys_mmio variable in s_main.c)  */

        post("audio input devices:");
        for (i = 0; i < nindevs; i++)
            post("%d. %s", i + (audio_api != API_MMIO),
                indevlist + i * MAXIMUM_DESCRIPTION);
    }
    if (!noutdevs)
        post("no audio output devices found");
    else
    {
        post("audio output devices:");
        for (i = 0; i < noutdevs; i++)
            post("%d. %s", i + (audio_api != API_MMIO),
                outdevlist + i * MAXIMUM_DESCRIPTION);
    }
    post("API number %d\n", audio_api);
}

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
    audio_getdevs(indevlist, &indevs, outdevlist, &outdevs, &canmulti,
        &cancallback, MAXIMUM_DEVICES, MAXIMUM_DESCRIPTION);

    if (rate < 1)
        rate = AUDIO_DEFAULT_SAMPLING;
    if (advance < 0)
        advance = AUDIO_DEFAULT_ADVANCE;
    if (blocksize != (1 << ilog2(blocksize)) || blocksize < AUDIO_DEFAULT_BLOCK)
        blocksize = AUDIO_DEFAULT_BLOCK;
     audio_init();
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
    sys_save_audio_params(nrealindev, realindev, realinchans,
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

    audio_getdevs(indevlist, &nindevs, outdevlist, &noutdevs, &canmulti,
         &cancallback, MAXIMUM_DEVICES, MAXIMUM_DESCRIPTION);

    sys_gui("set ::ui_audio::audioIn {}\n");
    for (i = 0; i < nindevs; i++)
        sys_vGui("lappend ::ui_audio::audioIn {%s}\n",
            indevlist + i * MAXIMUM_DESCRIPTION);

    sys_gui("set ::ui_audio::audioOut {}\n");
    for (i = 0; i < noutdevs; i++)
        sys_vGui("lappend ::ui_audio::audioOut {%s}\n",
            outdevlist + i * MAXIMUM_DESCRIPTION);

    sys_get_audio_params(&naudioindev, audioindev, chindev,
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

extern int pa_foo;
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
        sys_close_audio();
    sys_set_audio_settings(naudioindev, audioindev, nchindev, chindev,
        naudiooutdev, audiooutdev, nchoutdev, choutdev,
        rate, advance, (callback >= 0 ? callback : 0), newblocksize);
    if (!audio_callbackIsOpen && !callback)
        sys_reopen_audio();
    else scheduler_needToRestart();
}

/*
void sys_listdevs(void )
{
#ifdef USEAPI_PORTAUDIO
    if (audio_api == API_PORTAUDIO)
        sys_listaudiodevs();
    else 
#endif
#ifdef USEAPI_JACK
    if (audio_api == API_JACK)
        jack_listdevs();
    else
#endif
#ifdef USEAPI_OSS
    if (audio_api == API_OSS)
        sys_listaudiodevs();
    else
#endif
#ifdef USEAPI_ALSA
    if (audio_api == API_ALSA)
        sys_listaudiodevs();
    else
#endif
#ifdef USEAPI_MMIO
    if (audio_api == API_MMIO)
        sys_listaudiodevs();
    else
#endif
#ifdef USEAPI_DUMMY
    if (audio_api == API_DUMMY)
        sys_listaudiodevs();
    else
#endif
    post("unknown API");    

    sys_listmididevs();
}
*/
void sys_get_audio_devs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, int *cancallback, 
                        int maxndev, int devdescsize)
{
  audio_getdevs(indevlist, nindevs,
                outdevlist, noutdevs, 
                canmulti, cancallback, 
                maxndev, devdescsize);
}

void sys_set_audio_api(int which)
{
    int ok = 0;    /* check if the API is actually compiled in */
#ifdef USEAPI_PORTAUDIO
    ok += (which == API_PORTAUDIO);
#endif
#ifdef USEAPI_JACK
    ok += (which == API_JACK);
#endif
#ifdef USEAPI_OSS
    ok += (which == API_OSS);
#endif
#ifdef USEAPI_ALSA
    ok += (which == API_ALSA);
#endif
#ifdef USEAPI_MMIO
    ok += (which == API_MMIO);
#endif
#ifdef USEAPI_DUMMY
    ok += (which == API_DUMMY);
#endif
    if (!ok)
    {
        /*post("API %d not supported, reverting to %d (%s)",
            which, API_DEFAULT, API_DEFAULT_STRING);*/
        which = API_DEFAULT;
    }
    audio_api = which;
    if (0 && ok)
        post("audio_api set to %d", audio_api);
}

void global_audioAPI(void *dummy, t_float f)
{
    int newapi = f;
    if (newapi)
    {
        if (newapi == audio_api)
        {
            if (!audio_isopen() && audio_shouldkeepopen())
                sys_reopen_audio();
        }
        else
        {
            sys_close_audio();
            audio_api = newapi;
                /* bash device params back to default */
            audio_numberOfDevicesIn = audio_numberOfDevicesOut = 1;
            audio_devicesIn[0] = audio_devicesOut[0] = AUDIO_DEFAULT_DEVICE;
            audio_devicesInChannels[0] = audio_devicesOutChannels[0] = AUDIO_DEFAULT_CHANNELS;
            sys_reopen_audio();
        }
        global_audioProperties(0, 0);
    }
    else if (audio_isopen())
    {
        sys_close_audio();
    }
}

    /* start or stop the audio hardware */
void sys_set_audio_state(int onoff)
{
    if (onoff)  /* start */
    {
        if (!audio_isopen())
            sys_reopen_audio();    
    }
    else
    {
        if (audio_isopen())
            sys_close_audio();
    }
}

void sys_get_audio_apis(char *buf)
{
    int n = 0;
    strcpy(buf, "{ ");
#ifdef USEAPI_OSS
    sprintf(buf + strlen(buf), "{OSS %d} ", API_OSS); n++;
#endif
#ifdef USEAPI_MMIO
    sprintf(buf + strlen(buf), "{MMIO %d} ", API_MMIO); n++;
#endif
#ifdef USEAPI_ALSA
    sprintf(buf + strlen(buf), "{ALSA %d} ", API_ALSA); n++;
#endif
#ifdef USEAPI_PORTAUDIO
#ifdef _WIN32
    sprintf(buf + strlen(buf),
        "{ASIO %d} ", API_PORTAUDIO);
#else
#ifdef __APPLE__
    sprintf(buf + strlen(buf),
        "{PortAudio %d} ", API_PORTAUDIO);
#else
    sprintf(buf + strlen(buf), "{PortAudio %d} ", API_PORTAUDIO);
#endif
#endif
     n++;
#endif
#ifdef USEAPI_JACK
    sprintf(buf + strlen(buf), "{JACK %d} ", API_JACK); n++;
#endif
#ifdef USEAPI_DUMMY
    sprintf(buf + strlen(buf), "{Dummy %d} ", API_DUMMY); n++;
#endif
    strcat(buf, "}");
        /* then again, if only one API (or none) we don't offer any choice. */
    if (n < 2)
        strcpy(buf, "{}");
}

/* convert a device name to a (1-based) device number.  (Output device if
'output' parameter is true, otherwise input device).  Negative on failure. */

int sys_audiodevnametonumber(int output, const char *name)
{
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i, canmulti, cancallback;

    sys_get_audio_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        &canmulti, &cancallback, MAXIMUM_DEVICES, MAXIMUM_DESCRIPTION);

    if (output)
    {
        for (i = 0; i < noutdevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(outdevlist + i * MAXIMUM_DESCRIPTION))
                comp = strlen(outdevlist + i * MAXIMUM_DESCRIPTION);
            if (!strncmp(name, outdevlist + i * MAXIMUM_DESCRIPTION, comp))
                return (i);
        }
    }
    else
    {
        for (i = 0; i < nindevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(indevlist + i * MAXIMUM_DESCRIPTION))
                comp = strlen(indevlist + i * MAXIMUM_DESCRIPTION);
            if (!strncmp(name, indevlist + i * MAXIMUM_DESCRIPTION, comp))
                return (i);
        }
    }
    return (-1);
}

/* convert a (1-based) device number to a device name.  (Output device if
'output' parameter is true, otherwise input device).  Empty string on failure.
*/

void sys_audiodevnumbertoname(int output, int devno, char *name, int namesize)
{
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i, canmulti, cancallback;
    if (devno < 0)
    {
        *name = 0;
        return;
    }
    sys_get_audio_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        &canmulti, &cancallback, MAXIMUM_DEVICES, MAXIMUM_DESCRIPTION);
    if (output && (devno < noutdevs))
        strncpy(name, outdevlist + devno * MAXIMUM_DESCRIPTION, namesize);
    else if (!output && (devno < nindevs))
        strncpy(name, indevlist + devno * MAXIMUM_DESCRIPTION, namesize);
    else *name = 0;
    name[namesize-1] = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
