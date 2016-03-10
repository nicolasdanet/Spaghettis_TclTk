
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

extern int audio_api;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_sample *audio_soundIn;                            /* Shared. */
t_sample *audio_soundOut;                           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      audio_channelsIn;                   /* Shared. */
static int      audio_channelsOut;                  /* Shared. */
static t_float  audio_sampleRate;                   /* Shared. */
static int      audio_advanceInMicroseconds;        /* Shared. */

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
    else if (API_WITH_JACK && audio_api == API_JACK)        { return jack_pollDSP();    }
    else if (API_WITH_OSS && audio_api == API_OSS)          { return oss_send_dacs();   }
    else if (API_WITH_ALSA && audio_api == API_ALSA)        { return alsa_send_dacs();  }
    else if (API_WITH_DUMMY && audio_api == API_DUMMY)      { return dummy_pollDSP();   }
    else {
        PD_BUG;
    }
    
    return DACS_NO;
}

void audio_initializeMemory (int usedChannelsIn, int usedChannelsOut)
{
    int m = (INTERNAL_BLOCKSIZE * sizeof (t_sample)) * (usedChannelsIn ? usedChannelsIn : 2);
    int n = (INTERNAL_BLOCKSIZE * sizeof (t_sample)) * (usedChannelsOut ? usedChannelsOut : 2);

    PD_ASSERT (usedChannelsIn >= 0);
    PD_ASSERT (usedChannelsOut >= 0);
    
    if (audio_soundIn)  { PD_MEMORY_FREE (audio_soundIn);  audio_soundIn  = NULL; }
    if (audio_soundOut) { PD_MEMORY_FREE (audio_soundOut); audio_soundOut = NULL; }
    
    audio_soundIn  = (t_sample *)PD_MEMORY_GET (m);
    audio_soundOut = (t_sample *)PD_MEMORY_GET (n);
    
    audio_channelsIn  = usedChannelsIn;
    audio_channelsOut = usedChannelsOut;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_initialize (void)
{
    t_error err = PD_ERROR_NONE;
    
    #ifdef USEAPI_OSS
        oss_initialize();
    #endif

    #ifdef USEAPI_PORTAUDIO
        err |= pa_initialize();
    #endif
    
    return err;
}

void audio_release (void)
{
    #ifdef USEAPI_PORTAUDIO
        pa_release();
    #endif
    
    if (audio_soundIn)  { PD_MEMORY_FREE (audio_soundIn);  audio_soundIn  = NULL; }
    if (audio_soundOut) { PD_MEMORY_FREE (audio_soundOut); audio_soundOut = NULL; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_shrinkChannelsIn (int numberOfChannelsIn)
{
    PD_ASSERT (numberOfChannelsIn <= audio_channelsIn); 
    audio_channelsIn = numberOfChannelsIn;
}

void audio_shrinkChannelsOut (int numberOfChannelsOut)
{
    PD_ASSERT (numberOfChannelsOut <= audio_channelsOut);
    audio_channelsOut = numberOfChannelsOut;
}

void audio_setSampleRate (t_float sampleRate)
{
    audio_sampleRate = sampleRate;
}

void audio_setAdvanceInMicroseconds (int advanceInMicroseconds)
{
    audio_advanceInMicroseconds = advanceInMicroseconds;
}

void audio_setAdvanceInSamples (int advanceInSamples)
{
    audio_advanceInMicroseconds = SECONDS_TO_MICROSECONDS (advanceInSamples / audio_sampleRate);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_getChannelsIn (void) 
{
    return audio_channelsIn;
}

int audio_getChannelsOut (void)
{
    return audio_channelsOut; 
}

t_float audio_getSampleRate (void)
{
    return audio_sampleRate;
}

int audio_getAdvanceInMicroseconds (void)
{
    return audio_advanceInMicroseconds; 
}

int audio_getAdvanceInSamples (void)
{
    return MICROSECONDS_TO_SECONDS (audio_advanceInMicroseconds * audio_sampleRate);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
