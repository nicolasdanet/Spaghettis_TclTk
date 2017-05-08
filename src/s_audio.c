
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_deviceslist audio_devices;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_sample *audio_soundIn;                            /* Static. */
t_sample *audio_soundOut;                           /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      audio_totalOfChannelsIn;            /* Static. */
static int      audio_totalOfChannelsOut;           /* Static. */
static t_float  audio_sampleRate;                   /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_stop (void)
{
    if (audio_isOpened()) { audio_close(); } return PD_ERROR_NONE;
}


t_error audio_start (void)
{
    if (!audio_isOpened()) { return audio_open(); } else { return PD_ERROR_NONE; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_poll (void)
{
    return audio_pollNative();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Called by JACK (expect following stores to be atomic). */

void audio_setSampleRate (t_float sampleRate)
{
    #if PD_32BIT
        PD_ASSERT (sizeof (t_float) == 4);
    #endif 
    
    audio_sampleRate = sampleRate;      
}

void audio_setBlockSize (int blockSize)
{
    deviceslist_setBlockSize (&audio_devices, blockSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float audio_getSampleRate (void)
{
    return audio_sampleRate;
}

int audio_getTotalOfChannelsIn (void) 
{
    return audio_totalOfChannelsIn;
}

int audio_getTotalOfChannelsOut (void)
{
    return audio_totalOfChannelsOut; 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_vectorInitialize (t_float sampleRate, int totalOfChannelsIn, int totalOfChannelsOut)
{
    int m = (int)((INTERNAL_BLOCKSIZE * sizeof (t_sample)) * (totalOfChannelsIn ? totalOfChannelsIn : 2));
    int n = (int)((INTERNAL_BLOCKSIZE * sizeof (t_sample)) * (totalOfChannelsOut ? totalOfChannelsOut : 2));

    PD_ASSERT (totalOfChannelsIn >= 0);
    PD_ASSERT (totalOfChannelsOut >= 0);
    
    if (audio_soundIn)  { PD_MEMORY_FREE (audio_soundIn);  audio_soundIn  = NULL; }
    if (audio_soundOut) { PD_MEMORY_FREE (audio_soundOut); audio_soundOut = NULL; }
    
    audio_soundIn  = (t_sample *)PD_MEMORY_GET (m);
    audio_soundOut = (t_sample *)PD_MEMORY_GET (n);
    
    audio_totalOfChannelsIn  = totalOfChannelsIn;
    audio_totalOfChannelsOut = totalOfChannelsOut;
    
    audio_setSampleRate (sampleRate);
}

void audio_vectorShrinkIn (int totalOfChannelsIn)
{
    PD_ASSERT (totalOfChannelsIn <= audio_totalOfChannelsIn);
    
    audio_totalOfChannelsIn = totalOfChannelsIn;
}

void audio_vectorShrinkOut (int totalOfChannelsOut)
{
    PD_ASSERT (totalOfChannelsOut <= audio_totalOfChannelsOut);
    
    audio_totalOfChannelsOut = totalOfChannelsOut;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_initialize (void)
{
    return audio_initializeNative();
}

void audio_release (void)
{
    audio_releaseNative();
    
    if (audio_soundIn)  {
        PD_MEMORY_FREE (audio_soundIn);  audio_soundIn  = NULL; 
    }
    
    if (audio_soundOut) {
        PD_MEMORY_FREE (audio_soundOut); audio_soundOut = NULL; 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
