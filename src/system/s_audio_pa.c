
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "portaudio.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

double audio_getNanosecondsToSleep (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that PortAudio use interleaved channels. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample *audio_soundIn;
extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static PaStream         *pa_stream;                     /* Static. */

static t_ringbuffer     *pa_ringIn;                     /* Static. */
static t_ringbuffer     *pa_ringOut;                    /* Static. */

static int              pa_channelsIn;                  /* Static. */
static int              pa_channelsOut;                 /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PORTAUDIO_GRAIN     5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PORTAUDIO_BUFFER    4096                        /* Must be > INTERNAL_BLOCKSIZE. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void audio_vectorShrinkIn   (int);
void audio_vectorShrinkOut  (int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int pa_ringCallback (const void *input,
    void *output,
    unsigned long framesCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags, 
    void *userData)
{
    int32_t requiredIn  = (int32_t)(framesCount * pa_channelsIn);
    int32_t requiredOut = (int32_t)(framesCount * pa_channelsOut);
    
    if (output) {
    //
    if (ringbuffer_getAvailableRead (pa_ringOut) >= requiredOut) {
        ringbuffer_read (pa_ringOut, output, requiredOut);
    } else {
        PD_LOG ("*@*");
        memset (output, 0, requiredOut * sizeof (t_sample));        /* Fill with zeros. */
    }
    //
    }
    
    if (input) { ringbuffer_write (pa_ringIn, input, requiredIn); }
    
    return paContinue;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static PaError pa_openWithCallback (double sampleRate, 
    int numberOfChannelsIn,
    int numberOfChannelsOut,
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
            INTERNAL_BLOCKSIZE,
            paNoFlag,
            callback,
            NULL);
              
    if (err == paNoError) {
        err = Pa_StartStream (pa_stream);
        if (err == paNoError) { return paNoError; }
        else {
            audio_closeNative();
        }
    }
    //
    }
    
    pa_stream = NULL; 
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char *audio_nameNative (void)
{
    static const char *name = "PortAudio"; return name;
}

t_error audio_initializeNative (void)
{
    PaError err = Pa_Initialize();

    if (err != paNoError) { error__error1 (Pa_GetErrorText (err)); return PD_ERROR; }
    else {
        return PD_ERROR_NONE;
    }
}

void audio_releaseNative (void)
{
    Pa_Terminate();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error audio_openNative (t_devices *p)
{
    int deviceIn            = devices_getInSize (p)  ? devices_getInAtIndexAsNumber (p, 0)  : 0;
    int deviceOut           = devices_getOutSize (p) ? devices_getOutAtIndexAsNumber (p, 0) : 0;
    int numberOfChannelsIn  = devices_getInSize (p)  ? devices_getInChannelsAtIndex (p, 0)  : 0;
    int numberOfChannelsOut = devices_getOutSize (p) ? devices_getOutChannelsAtIndex (p, 0) : 0;
    int sampleRate          = devices_getSampleRate (p);
    
    int t;
    int n;
    int i = -1;
    int o = -1;
    
    if (pa_stream) { audio_closeNative(); PD_BUG; }

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
    
    audio_vectorShrinkIn (pa_channelsIn = numberOfChannelsIn);        /* Cached for convenience. */
    audio_vectorShrinkOut (pa_channelsOut = numberOfChannelsOut);     /* Ditto. */

    if (pa_ringIn)  { ringbuffer_free (pa_ringIn);  pa_ringIn  = NULL; }
    if (pa_ringOut) { ringbuffer_free (pa_ringOut); pa_ringOut = NULL; }

    if (pa_channelsIn) {    /* For now audio in is required to synchronize properly the callback. */
    //
    PaError err = paNoError;
    
    {
        int32_t k  = (int32_t)PD_NEXT_POWER_2 (PORTAUDIO_BUFFER * pa_channelsIn);
        pa_ringIn  = ringbuffer_new (sizeof (t_sample), k == 0 ? 1 : k);
    }
    {
        int32_t k  = (int32_t)PD_NEXT_POWER_2 (PORTAUDIO_BUFFER * pa_channelsOut);
        pa_ringOut = ringbuffer_new (sizeof (t_sample), k == 0 ? 1 : k);
    }
    
    err = pa_openWithCallback (sampleRate,
            numberOfChannelsIn, 
            numberOfChannelsOut,
            i,
            o,
            pa_ringCallback);
    
    if (err != paNoError) { error__error1 (Pa_GetErrorText (err)); return PD_ERROR; }
    else {
        return PD_ERROR_NONE;
    }
    //
    }
    
    return PD_ERROR;
}

void audio_closeNative (void)
{
    if (pa_stream) {
        Pa_AbortStream (pa_stream);
        Pa_CloseStream (pa_stream);
        pa_stream = NULL;
    }
    
    if (pa_ringIn)  { ringbuffer_free (pa_ringIn);  pa_ringIn  = NULL; }
    if (pa_ringOut) { ringbuffer_free (pa_ringOut); pa_ringOut = NULL; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int audio_pollNative (void)
{
    t_sample *sound;
    t_sample *p1 = NULL;
    t_sample *p2 = NULL;
    int i, k;
    
    int status = DACS_YES;
        
    if (!pa_stream || (pa_channelsIn <= 0 && pa_channelsOut <= 0)) { return DACS_NO; }
    else {
    //
    int32_t requiredIn  = INTERNAL_BLOCKSIZE * pa_channelsIn;
    int32_t requiredOut = INTERNAL_BLOCKSIZE * pa_channelsOut;
    
    /* Buffer on the stack in order to interleave and deinterleave. */
    
    t_sample *t = (t_sample *)alloca ((PD_MAX (requiredIn, requiredOut)) * sizeof (t_sample));

    int needToWait = 0; double ns = audio_getNanosecondsToSleep() / (double)PORTAUDIO_GRAIN;
    
    if (pa_channelsIn)  {
        while (ringbuffer_getAvailableRead (pa_ringIn) < requiredIn) {
            status = DACS_SLEPT;
            if (needToWait < PORTAUDIO_GRAIN * 2) {
                PD_LOG (".");
                nano_sleep (ns);
            } else { return DACS_NO; }
            needToWait++;
        }
    }
    
    if (pa_channelsOut) {
        while (ringbuffer_getAvailableWrite (pa_ringOut) < requiredOut) {
            status = DACS_SLEPT;
            if (needToWait < PORTAUDIO_GRAIN * 2) {
                PD_LOG (".");
                nano_sleep (ns);
            } else { return DACS_NO; }
            needToWait++;
        }
    }
    
    if (pa_channelsIn)  {
        ringbuffer_read (pa_ringIn, t, requiredIn);
        for (i = 0, sound = audio_soundIn, p1 = t; i < pa_channelsIn; i++, p1++) {
            for (k = 0, p2 = p1; k < INTERNAL_BLOCKSIZE; k++, sound++, p2 += pa_channelsIn) {
                *sound = *p2;
            }
        }
    }
    
    if (pa_channelsOut) {
        for (i = 0, sound = audio_soundOut, p1 = t; i < pa_channelsOut; i++, p1++) {
            for (k = 0, p2 = p1; k < INTERNAL_BLOCKSIZE; k++, sound++, p2 += pa_channelsOut) {
                *p2 = *sound;
                *sound = 0.0;
            }
        }
        ringbuffer_write (pa_ringOut, t, requiredOut);
    }
    //
    }
    
    return status;
}

int audio_getVectorSizeNative (void)
{
    return INTERNAL_BLOCKSIZE;  /* Assumed always the case. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error audio_getListsNative (t_deviceslist *p)
{
    t_error err = PD_ERROR_NONE;
    
    int i;
        
    for (i = 0; i < Pa_GetDeviceCount(); i++) {
    
        const PaDeviceInfo *info = Pa_GetDeviceInfo (i);
            
        if (info->maxInputChannels > 0) {
            err |= deviceslist_appendAudioInWithString (p, info->name, info->maxInputChannels);
        }
        
        if (info->maxOutputChannels > 0) {
            err |= deviceslist_appendAudioOutWithString (p, info->name, info->maxOutputChannels);
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
