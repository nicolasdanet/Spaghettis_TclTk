
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
#include "s_ringbuffer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <portaudio.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS
    #define PA_MICROSLEEP   Sleep (1)
#else
    #define PA_MICROSLEEP   usleep (1000);
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that PortAudio interleaves channels. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern int              audio_channelsIn;
extern int              audio_channelsOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static PaStream         *pa_stream;                     /* Shared. */
static t_sample         *pa_soundIn;                    /* Shared. */
static t_sample         *pa_soundOut;                   /* Shared. */
static char             *pa_bufferIn;                   /* Shared. */
static char             *pa_bufferOut;                  /* Shared. */

static sys_ringbuf      pa_ringIn;                      /* Shared. */
static sys_ringbuf      pa_ringOut;                     /* Shared. */

static int              pa_channelsIn;                  /* Shared. */
static int              pa_channelsOut;                 /* Shared. */

static t_audiocallback  pa_callback;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int pa_callbackLowLevel (const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags, 
    void *userData)
{
    int i, j;
    unsigned long n;
    t_sample *p1 = NULL;
    t_sample *p2 = NULL;
    
    const int blockSize = INTERNAL_BLOCKSIZE;
    
    PD_ASSERT ((frameCount % blockSize) == 0);
    PD_ABORT  ((frameCount % blockSize) != 0);
    
    for (n = 0; n < frameCount; n += blockSize) {
    //
    if (input) {
        t_sample *p = ((t_sample *)input) + (n * pa_channelsIn);
        t_sample *sound = pa_soundIn;
        for (i = 0, p1 = p; i < pa_channelsIn; i++, p1++) {
            for (j = 0, p2 = p1; j < blockSize; j++, p2 += pa_channelsIn) { 
                *sound++ = *p2;
            }
        }
    } else { 
        memset ((void *)pa_soundIn, 0, blockSize * pa_channelsIn * sizeof (t_sample));
    }
    
    memset ((void *)pa_soundOut, 0, blockSize * pa_channelsOut * sizeof (t_sample));
    
    (*pa_callback)();
        
    if (output) {
        t_sample *p = ((t_sample *)output) + (n * pa_channelsOut);
        t_sample *sound = pa_soundOut;
        for (i = 0, p1 = p; i < pa_channelsOut; i++, p1++) {
            for (j = 0, p2 = p1; j < blockSize; j++, p2 += pa_channelsOut) { 
                *p2 = *sound++; 
            }
        }
    }
    //
    }
    
    return paContinue;
}

static int pa_callbackFIFO (const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags, 
    void *userData)
{
    unsigned long availableOut = ringbuffer_getReadAvailable (&pa_ringOut);
    unsigned long requiredIn   = frameCount * sizeof (t_sample) * pa_channelsIn;
    unsigned long requiredOut  = frameCount * sizeof (t_sample) * pa_channelsOut;
    
    if (availableOut >= requiredOut) {
        if (output) { ringbuffer_read (&pa_ringOut, output, requiredOut, pa_bufferOut); }
        if (input)  { ringbuffer_write (&pa_ringIn, input, requiredOut, pa_bufferIn);   }
            
    } else { 
        if (output) {   /* Fill with zeros. */
            int i;
            for (i = 0; i < pa_channelsOut; i++) {
                unsigned long j;
                t_sample *p = ((t_sample *)output) + i;
                for (j = 0; j < frameCount; j++, p += pa_channelsOut) { *p = 0.0; }
            }
        }
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
    PaStreamParameters *parametersInPointer  = NULL;
    PaStreamParameters *parametersOutPointer = NULL;

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

    if (numberOfChannelsIn > 0)  { parametersInPointer  = &parametersIn;  }
    if (numberOfChannelsOut > 0) { parametersOutPointer = &parametersOut; }

    err = Pa_IsFormatSupported (parametersInPointer, parametersOutPointer, sampleRate);

    if (err == paFormatIsSupported) {
    //
    err = Pa_OpenStream (&pa_stream, 
            parametersInPointer, 
            parametersOutPointer, 
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

    if (err != paNoError) { post_error ("PortAudio: `%s'", Pa_GetErrorText (err)); return PD_ERROR; }
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
    t_sample *soundIn,
    t_sample *soundOut,
    int blockSize,
    int advanceInNumberOfBlocks,
    int deviceIn,
    int deviceOut,
    t_audiocallback callback)
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
        const PaDeviceInfo *info = Pa_GetDeviceInfo(t);
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

    if (i == -1) { numberOfChannelsIn  = 0; }
    if (o == -1) { numberOfChannelsOut = 0; }
    
    PD_ASSERT (numberOfChannelsIn <= audio_channelsIn);
    PD_ASSERT (numberOfChannelsOut <= audio_channelsOut);
    
    pa_channelsIn   = audio_channelsIn  = numberOfChannelsIn;
    pa_channelsOut  = audio_channelsOut = numberOfChannelsOut;
    pa_soundIn      = soundIn;
    pa_soundOut     = soundOut;

    if (pa_bufferIn)  { PD_MEMORY_FREE (pa_bufferIn);  pa_bufferIn  = NULL; }
    if (pa_bufferOut) { PD_MEMORY_FREE (pa_bufferOut); pa_bufferOut = NULL; }

    if (numberOfChannelsIn || numberOfChannelsOut) {
    //
    PaError err;
    
    if (callback) {
    
        pa_callback = callback;
        
        err = pa_openWithCallback (sampleRate, 
                numberOfChannelsIn, 
                numberOfChannelsOut,
                blockSize, 
                i, 
                o, 
                pa_callbackLowLevel);
            
    } else {
    
        if (pa_channelsIn) {
            size_t k = advanceInNumberOfBlocks * blockSize * pa_channelsIn * sizeof (t_sample);
            pa_bufferIn = PD_MEMORY_GET (k);
            ringbuffer_initialize (&pa_ringIn, k, pa_bufferIn, 0);
        }
        if (pa_channelsOut) {
            size_t k = advanceInNumberOfBlocks * blockSize * pa_channelsOut * sizeof (t_sample);
            pa_bufferOut = PD_MEMORY_GET (k);
            ringbuffer_initialize (&pa_ringOut, k, pa_bufferOut, 0);
        }
        
        err = pa_openWithCallback (sampleRate,
                numberOfChannelsIn, 
                numberOfChannelsOut,
                blockSize, 
                i,
                o,
                pa_callbackFIFO);
    }

    if (err != paNoError) { 
        post_error ("PortAudio: `%s'", Pa_GetErrorText (err)); return PD_ERROR; 
    }
    //
    }
    
    return PD_ERROR_NONE;
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
    int j, k;
    
    int status = DACS_YES;
        
    if (!pa_stream || (!pa_channelsIn && !pa_channelsOut)) { return DACS_NO; }
    else {
    //
    size_t requiredIn  = INTERNAL_BLOCKSIZE * sizeof (t_sample) * pa_channelsIn;
    size_t requiredOut = INTERNAL_BLOCKSIZE * sizeof (t_sample) * pa_channelsOut;
    t_sample *t = (t_sample *)alloca (PD_MAX (requiredIn, requiredOut));

    /* If there's no input channels synchnronize on output. */
    
    if (!pa_channelsIn) {
        while (ringbuffer_getWriteAvailable (&pa_ringOut) < requiredOut) {
            status = DACS_SLEPT; PA_MICROSLEEP; 
        }
    }
    
    if (pa_channelsOut) {
        for (j = 0, sound = pa_soundOut, p1 = t; j < pa_channelsOut; j++, p1++) {
            for (k = 0, p2 = p1; k < INTERNAL_BLOCKSIZE; k++, sound++, p2 += pa_channelsOut) {
                *p2 = *sound;
            }
        }
        ringbuffer_write (&pa_ringOut, t, requiredOut, pa_bufferOut);
    }
    
    /* If there's input channels synchnronize on it. */
    
    if (pa_channelsIn) {
        while (ringbuffer_getReadAvailable (&pa_ringIn) < requiredIn) {
            status = DACS_SLEPT; PA_MICROSLEEP;
        }
    }
    
    if (pa_channelsIn) {
        ringbuffer_read (&pa_ringIn, t, requiredIn, pa_bufferIn);
        for (j = 0, sound = pa_soundIn, p1 = t; j < pa_channelsIn; j++, p1++) {
            for (k = 0, p2 = p1; k < INTERNAL_BLOCKSIZE; k++, sound++, p2 += pa_channelsIn) {
                *sound = *p2;
            }
        }
    }

    memset (pa_soundOut, 0, requiredOut);
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
    int *canMultiple,
    int *canCallback)
{
    int i;
    int m = 0;
    int n = 0;
    
    t_error err = PD_ERROR_NONE;
        
    *canMultiple = 1;           /* One device each for input and for output. */
    *canCallback = 1;
    
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
