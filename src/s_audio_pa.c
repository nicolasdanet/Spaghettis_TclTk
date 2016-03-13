
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "portaudio.h"
#include "pa_ringbuffer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS
    #define PA_MICROSLEEP   Sleep (1)
#else
    #define PA_MICROSLEEP   usleep (1000);
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that PortAudio use interleaved channels. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample         *audio_soundIn;
extern t_sample         *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static PaStream         *pa_stream;                     /* Shared. */
static char             *pa_bufferIn;                   /* Shared. */
static char             *pa_bufferOut;                  /* Shared. */

static PaUtilRingBuffer pa_ringIn;                      /* Shared. */
static PaUtilRingBuffer pa_ringOut;                     /* Shared. */

static int              pa_channelsIn;                  /* Shared. */
static int              pa_channelsOut;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int pa_ringCallback (const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags, 
    void *userData)
{
    ring_buffer_size_t requiredIn  = (ring_buffer_size_t)(frameCount * pa_channelsIn);
    ring_buffer_size_t requiredOut = (ring_buffer_size_t)(frameCount * pa_channelsOut);
    
    if (PaUtil_GetRingBufferReadAvailable (&pa_ringOut) >= requiredOut) {       /* Could be zero. */

        if (output) { PaUtil_ReadRingBuffer (&pa_ringOut, output, requiredOut); }
        if (input)  { PaUtil_WriteRingBuffer (&pa_ringIn, input, requiredIn);   }

    } else {
        if (output) { memset (output, 0, requiredOut * sizeof (t_sample)); }    /* Fill with zeros. */
    }

    return paContinue;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static PaError pa_openWithCallback (double sampleRate, 
    int numberOfChannelsIn,
    int numberOfChannelsOut,
    int blockSize,
    int deviceIn,
    int deviceOut,
    PaStreamCallback *callback)
{
    PaError err;
    PaStreamParameters parametersIn;
    PaStreamParameters parametersOut;
    PaStreamParameters *p1 = NULL;
    PaStreamParameters *p2 = NULL;

    parametersIn.device                     = deviceIn;
    parametersIn.channelCount               = numberOfChannelsIn;
    parametersIn.sampleFormat               = paFloat32;
    parametersIn.suggestedLatency           = 0;
    parametersIn.hostApiSpecificStreamInfo  = NULL;

    parametersOut.device                    = deviceOut;
    parametersOut.channelCount              = numberOfChannelsOut;
    parametersOut.sampleFormat              = paFloat32;
    parametersOut.suggestedLatency          = 0;
    parametersOut.hostApiSpecificStreamInfo = NULL;

    if (numberOfChannelsIn > 0)  { p1 = &parametersIn;  }
    if (numberOfChannelsOut > 0) { p2 = &parametersOut; }

    err = Pa_IsFormatSupported (p1, p2, sampleRate);

    if (err == paFormatIsSupported) {
    //
    err = Pa_OpenStream (&pa_stream, 
            p1, 
            p2, 
            sampleRate, 
            blockSize,
            paNoFlag,
            callback,
            NULL);
              
    if (err == paNoError) {
        err = Pa_StartStream (pa_stream);
        if (err == paNoError) { return paNoError; }
        else {
            pa_close();
        }
    }
    //
    }
    
    pa_stream = NULL; 
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* On Mac OS Pa_Initialize() closes file descriptor 1 (standard output). */
/* As a workaround, dup it to another number and dup2 it back afterward. */
    
t_error pa_initialize (void)
{
    #if PD_APPLE
    
    int f = dup (1);
    int dummy = open ("/dev/null", 0);
    dup2 (dummy, 1);
    
    PaError err = Pa_Initialize();
    
    close (1);
    close (dummy);
    if (f >= 0) { fflush (stdout); dup2 (f, 1); close (f); }
    
    #else
    
    PaError err = Pa_Initialize();
    
    #endif

    if (err != paNoError) { post_error ("PortAudio: `%s'", Pa_GetErrorText (err)); return PD_ERROR; }   // --
    else {
        return PD_ERROR_NONE;
    }
}

void pa_release (void)
{
    Pa_Terminate();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error pa_open (int sampleRate,
    int numberOfChannelsIn, 
    int numberOfChannelsOut,
    int blockSize,
    int advanceInNumberOfBlocks,
    int deviceIn,
    int deviceOut)
{
    int t;
    int n;
    int i = -1;
    int o = -1;
    
    if (pa_stream) { pa_close(); PD_BUG; }

    if (numberOfChannelsIn > 0) {
    //
    for (t = 0, n = 0; t < Pa_GetDeviceCount(); t++) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo (t);
        if (info->maxInputChannels > 0) {
            if (n == deviceIn) {
                numberOfChannelsIn = PD_MIN (numberOfChannelsIn, info->maxInputChannels);
                i = t;
                break;
            }
            n++;
        }
    }
    //
    }   
    
    if (numberOfChannelsOut > 0) {
    //
    for (t = 0, n = 0; t < Pa_GetDeviceCount(); t++) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo (t);
        if (info->maxOutputChannels > 0) {
            if (n == deviceOut) {
                numberOfChannelsOut = PD_MIN (numberOfChannelsOut, info->maxOutputChannels);
                o = t;
                break;
            }
            n++;
        }
    }
    //
    }   

    if (i == -1 || (int)(Pa_GetDeviceInfo (i)->defaultSampleRate) != sampleRate) { numberOfChannelsIn  = 0; }
    if (o == -1 || (int)(Pa_GetDeviceInfo (o)->defaultSampleRate) != sampleRate) { numberOfChannelsOut = 0; }
    
    audio_shrinkChannelsIn (pa_channelsIn = numberOfChannelsIn);        /* Cached for convenience. */
    audio_shrinkChannelsOut (pa_channelsOut = numberOfChannelsOut);     /* Ditto. */

    if (pa_bufferIn)  { PD_MEMORY_FREE (pa_bufferIn);  pa_bufferIn  = NULL; }
    if (pa_bufferOut) { PD_MEMORY_FREE (pa_bufferOut); pa_bufferOut = NULL; }

    if (pa_channelsIn || pa_channelsOut) {
    //
    PaError err = paNoError;
    
    {
        size_t k = advanceInNumberOfBlocks * blockSize * pa_channelsIn;
        k = PD_NEXT_POWER_OF_TWO (k);
        pa_bufferIn = PD_MEMORY_GET (k * sizeof (t_sample));
        PD_ASSERT ((ring_buffer_size_t)k > 0);
        if (PaUtil_InitializeRingBuffer (&pa_ringIn, sizeof (t_sample), k, pa_bufferIn)) { 
            PD_BUG;
        }
    }
    {
        size_t k = advanceInNumberOfBlocks * blockSize * pa_channelsOut;
        k = PD_NEXT_POWER_OF_TWO (k);
        pa_bufferOut = PD_MEMORY_GET (k * sizeof (t_sample));
        PD_ASSERT ((ring_buffer_size_t)k > 0);
        if (PaUtil_InitializeRingBuffer (&pa_ringOut, sizeof (t_sample), k, pa_bufferOut)) { 
            PD_BUG; 
        }
    }
    
    err = pa_openWithCallback (sampleRate,
            numberOfChannelsIn, 
            numberOfChannelsOut,
            blockSize, 
            i,
            o,
            pa_ringCallback);
    
    if (err != paNoError) { post_error ("PortAudio: `%s'", Pa_GetErrorText (err)); return PD_ERROR; }   // --
    else {
        return PD_ERROR_NONE;
    }
    //
    }
    
    return PD_ERROR;
}

void pa_close (void)
{
    if (pa_stream)    { Pa_AbortStream (pa_stream); Pa_CloseStream (pa_stream); pa_stream = NULL; }
    if (pa_bufferIn)  { PD_MEMORY_FREE (pa_bufferIn);  pa_bufferIn  = NULL; }
    if (pa_bufferOut) { PD_MEMORY_FREE (pa_bufferOut); pa_bufferOut = NULL; } 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int pa_pollDSP (void)
{
    t_sample *sound;
    t_sample *p1 = NULL;
    t_sample *p2 = NULL;
    int i, k;
    
    int status = DACS_YES;
        
    if (!pa_stream || (!pa_channelsIn && !pa_channelsOut)) { return DACS_NO; }
    else {
    //
    ring_buffer_size_t requiredIn  = INTERNAL_BLOCKSIZE * pa_channelsIn;
    ring_buffer_size_t requiredOut = INTERNAL_BLOCKSIZE * pa_channelsOut;
    
    t_sample *t = (t_sample *)alloca ((PD_MAX (requiredIn, requiredOut)) * sizeof (t_sample));

    if (pa_channelsOut) {
        int wait = 0;
        while (PaUtil_GetRingBufferWriteAvailable (&pa_ringOut) < requiredOut) {
            status = DACS_SLEPT; if (wait < 10) { PA_MICROSLEEP; } else { return DACS_NO; }
            wait++;
        }
        for (i = 0, sound = audio_soundOut, p1 = t; i < pa_channelsOut; i++, p1++) {
            for (k = 0, p2 = p1; k < INTERNAL_BLOCKSIZE; k++, sound++, p2 += pa_channelsOut) {
                *p2 = *sound;
                *sound = 0.0;
            }
        }
        PaUtil_WriteRingBuffer (&pa_ringOut, t, requiredOut);
    }
    
    if (pa_channelsIn) {
        int wait = 0;
        while (PaUtil_GetRingBufferReadAvailable (&pa_ringIn) < requiredIn) {
            status = DACS_SLEPT; if (wait < 10) { PA_MICROSLEEP; } else { return DACS_NO; }
            wait++;
        }
        PaUtil_ReadRingBuffer (&pa_ringIn, t, requiredIn);
        for (i = 0, sound = audio_soundIn, p1 = t; i < pa_channelsIn; i++, p1++) {
            for (k = 0, p2 = p1; k < INTERNAL_BLOCKSIZE; k++, sound++, p2 += pa_channelsIn) {
                *sound = *p2;
            }
        }
    }
    //
    }
    
    return status;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error pa_getLists (char *devicesIn,
    int *numberOfDevicesIn,
    char *devicesOut,
    int *numberOfDevicesOut,
    int *canMultiple)
{
    int i;
    int m = 0;
    int n = 0;
    
    t_error err = PD_ERROR_NONE;
        
    *canMultiple = 1;           /* One device each for input and for output. */
    
    for (i = 0; i < Pa_GetDeviceCount(); i++) {
    //
    const PaDeviceInfo *info = Pa_GetDeviceInfo (i);
        
    if (info->maxInputChannels > 0 && m < MAXIMUM_DEVICES) {
        err |= string_copy (devicesIn + (m * MAXIMUM_DESCRIPTION),  MAXIMUM_DESCRIPTION, info->name);
        m++;
    }
    
    if (info->maxOutputChannels > 0 && n < MAXIMUM_DEVICES) {
        err |= string_copy (devicesOut + (n * MAXIMUM_DESCRIPTION), MAXIMUM_DESCRIPTION, info->name);
        n++;
    }
    //
    }
    
    *numberOfDevicesIn  = m;
    *numberOfDevicesOut = n;
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
