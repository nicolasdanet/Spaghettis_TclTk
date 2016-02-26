
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

extern int  audio_api;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_sample    *audio_soundIn;                         /* Shared. */
t_sample    *audio_soundOut;                        /* Shared. */

int         audio_channelsIn;                       /* Shared. */
int         audio_channelsOut;                      /* Shared. */
int         audio_advanceInSamples;                 /* Shared. */
int         audio_advanceInMicroseconds;            /* Shared. */
t_float     audio_sampleRate;                       /* Shared. */
int         audio_blockSize;                        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void sys_setchsr(int chin, int chout, int sr)
{
    int nblk;
    int inbytes = (chin ? chin : 2) *
                (AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample));
    int outbytes = (chout ? chout : 2) *
                (AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample));

    if (audio_soundIn)
        PD_MEMORY_FREE(audio_soundIn);
    if (audio_soundOut)
        PD_MEMORY_FREE(audio_soundOut);
    audio_channelsIn = chin;
    audio_channelsOut = chout;
    audio_sampleRate = sr;
    audio_advanceInSamples = (audio_advanceInMicroseconds * audio_sampleRate) / (1000000.);
    if (audio_advanceInSamples < AUDIO_DEFAULT_BLOCKSIZE)
        audio_advanceInSamples = AUDIO_DEFAULT_BLOCKSIZE;

    audio_soundIn = (t_sample *)PD_MEMORY_GET(inbytes);
    memset(audio_soundIn, 0, inbytes);

    audio_soundOut = (t_sample *)PD_MEMORY_GET(outbytes);
    memset(audio_soundOut, 0, outbytes);

    if (0)
        post("input channels = %d, output channels = %d",
            audio_channelsIn, audio_channelsOut);
    canvas_resume_dsp(canvas_suspend_dsp());
}



    /* open audio using whatever parameters were last used */


int sys_send_dacs(void)
{
    #if 0
    if (0 /*sys_meters*/)
    {
        int i, n;
        t_sample maxsamp;
        for (i = 0, n = audio_channelsIn * AUDIO_DEFAULT_BLOCKSIZE, maxsamp = sys_inmax;
            i < n; i++)
        {
            t_sample f = audio_soundIn[i];
            if (f > maxsamp) maxsamp = f;
            else if (-f > maxsamp) maxsamp = -f;
        }
        sys_inmax = maxsamp;
        for (i = 0, n = audio_channelsOut * AUDIO_DEFAULT_BLOCKSIZE, maxsamp = sys_outmax;
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
