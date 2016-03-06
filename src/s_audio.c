
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error audio_setDSP (int isOn)
{
    t_error err = PD_ERROR_NONE;
    
    if (isOn) { if (!audio_isOpened()) { err = audio_open(); } } 
    else {
        if (audio_isOpened()) { audio_close(); }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_stopDSP (void)
{
    return audio_setDSP (0);
}


t_error audio_startDSP (void)
{
    return audio_setDSP (1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_pollDSP (void)
{
    if (API_WITH_PORTAUDIO && audio_api == API_PORTAUDIO)   { return pa_pollDSP();      }
    else if (API_WITH_JACK && audio_api == API_JACK)        { return jack_send_dacs();  }
    else if (API_WITH_OSS && audio_api == API_OSS)          { return oss_send_dacs();   }
    else if (API_WITH_ALSA && audio_api == API_ALSA)        { return alsa_send_dacs();  }
    else if (API_WITH_DUMMY && audio_api == API_DUMMY)      { return dummy_pollDSP();   }
    else {
        PD_BUG;
    }
    
    return DACS_NO;
}

void audio_initializeMemoryAndParameters (int usedChannelsIn, int usedChannelsOut, int sampleRate)
{
    /* Channels disposed as consecutive chunk of samples with INTERNAL_BLOCKSIZE length. */
    
    int m = (usedChannelsIn ? usedChannelsIn : 2) * (INTERNAL_BLOCKSIZE * sizeof (t_sample));
    int n = (usedChannelsOut ? usedChannelsOut : 2) * (INTERNAL_BLOCKSIZE * sizeof (t_sample));

    PD_ASSERT (usedChannelsIn >= 0);
    PD_ASSERT (usedChannelsOut >= 0);
    
    if (audio_soundIn)  { PD_MEMORY_FREE (audio_soundIn);  audio_soundIn  = NULL; }
    if (audio_soundOut) { PD_MEMORY_FREE (audio_soundOut); audio_soundOut = NULL; }
    
    audio_soundIn  = (t_sample *)PD_MEMORY_GET (m);
    audio_soundOut = (t_sample *)PD_MEMORY_GET (n);
    
    audio_channelsIn        = usedChannelsIn;
    audio_channelsOut       = usedChannelsOut;
    audio_sampleRate        = (t_float)sampleRate;
    audio_advanceInSamples  = MICROSECONDS_TO_SECONDS (audio_advanceInMicroseconds * audio_sampleRate);
    audio_advanceInSamples  = PD_MAX (audio_advanceInSamples, INTERNAL_BLOCKSIZE);

    canvas_resume_dsp (canvas_suspend_dsp());
}

void audio_release (void)
{
    if (audio_soundIn)  { PD_MEMORY_FREE (audio_soundIn);  audio_soundIn  = NULL; }
    if (audio_soundOut) { PD_MEMORY_FREE (audio_soundOut); audio_soundOut = NULL; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float audio_getSampleRate (void)
{
    return audio_sampleRate;
}

int audio_getChannelsIn (void) 
{
     return audio_channelsIn;
}

int audio_getChannelsOut (void)
{
    return audio_channelsOut; 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
