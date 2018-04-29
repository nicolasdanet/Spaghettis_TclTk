
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "jack/jack.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_MAXIMUM_PORTS  128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

// -- TODO: Remove naughty mutex.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// -- FIXME: Manage sample rate properly.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample *audio_soundIn;
extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static jack_client_t        *jack_client;                                       /* Static. */

static t_sample             *jack_bufferIn;                                     /* Static. */
static t_sample             *jack_bufferOut;                                    /* Static. */
static jack_port_t          *jack_portsIn[JACK_MAXIMUM_PORTS];                  /* Static. */
static jack_port_t          *jack_portsOut[JACK_MAXIMUM_PORTS];                 /* Static. */

static int                  jack_numberOfPortsIn;                               /* Static. */
static int                  jack_numberOfPortsOut;                              /* Static. */

static jack_nframes_t       jack_framesRequired;                                /* Static. */
static jack_nframes_t       jack_framesFilled;                                  /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static pthread_cond_t       jack_cond;                                          /* Static. */
static pthread_mutex_t      jack_mutex;                                         /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_BUFFER_SIZE    4096        /* Buffer size (per channel). */ 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void audio_vectorShrinkIn   (int);
void audio_vectorShrinkOut  (int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int jack_pollCallback (jack_nframes_t numberOfFrames, void *dummy)
{
    int i;
    jack_default_audio_sample_t *in  = NULL;
    jack_default_audio_sample_t *out = NULL;

    size_t size = numberOfFrames * sizeof (t_sample);
    
    pthread_mutex_lock (&jack_mutex);
        
    jack_framesRequired = PD_MAX (numberOfFrames, INTERNAL_BLOCKSIZE);
    
    if (jack_framesFilled >= numberOfFrames) {
    //
    PD_ASSERT (jack_framesFilled == numberOfFrames);
    
    for (i = 0; i < jack_numberOfPortsOut; i++) {
        if ((out = (jack_default_audio_sample_t *)jack_port_get_buffer (jack_portsOut[i], numberOfFrames))) {
            memcpy (out, jack_bufferOut + (i * JACK_BUFFER_SIZE), size);
        }
    }
    
    for (i = 0; i < jack_numberOfPortsIn; i++) {
        if ((in = (jack_default_audio_sample_t *)jack_port_get_buffer (jack_portsIn[i], numberOfFrames))) {
            memcpy (jack_bufferIn + (i * JACK_BUFFER_SIZE), in, size);
        }
    }
    //
    } else {    /* Fill with zeros. */
    //
    for (i = 0; i < jack_numberOfPortsOut; i++) {
        if ((out = (jack_default_audio_sample_t *)jack_port_get_buffer (jack_portsOut[i], numberOfFrames))) {
            memset (out, 0, size);
        }
    }
    //
    }
    
    jack_framesFilled = 0;
    
    pthread_cond_broadcast (&jack_cond);
    pthread_mutex_unlock (&jack_mutex);
    
    return 0;
}

static void jack_shutdownCallback (void *dummy)
{
    jack_client = NULL;
    scheduler_needToExitWithError();
    pthread_cond_broadcast (&jack_cond);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char *audio_nameNative (void)
{
    static const char *name = "JACK"; return name;      /* Static. */
}

t_error audio_initializeNative (void)
{
    return PD_ERROR_NONE;
}

void audio_releaseNative (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error audio_openNative (t_devicesproperties *p)
{
    int numberOfChannelsIn  = devices_getInSize (p)  ? devices_getInChannelsAtIndex (p, 0)  : 0;
    int numberOfChannelsOut = devices_getOutSize (p) ? devices_getOutChannelsAtIndex (p, 0) : 0;
    // int sampleRate       = devices_getSampleRate (p);
        
    PD_ASSERT (sizeof (t_sample) == sizeof (jack_default_audio_sample_t));
    PD_ABORT  (sizeof (t_sample) != sizeof (jack_default_audio_sample_t));
    
    if (numberOfChannelsIn || numberOfChannelsOut) {
    //
    jack_status_t status;
    
    numberOfChannelsIn  = PD_MIN (numberOfChannelsIn, JACK_MAXIMUM_PORTS);
    numberOfChannelsOut = PD_MIN (numberOfChannelsOut, JACK_MAXIMUM_PORTS);
 
    PD_ASSERT (!jack_client);

    jack_client = jack_client_open (PD_NAME_LOWERCASE, JackNoStartServer, &status, NULL);
    
    if (jack_client) {
    //
    int i;
    
    if (jack_bufferIn)  { PD_MEMORY_FREE (jack_bufferIn);  jack_bufferIn  = NULL; }
    if (jack_bufferOut) { PD_MEMORY_FREE (jack_bufferOut); jack_bufferOut = NULL; }
    
    if (numberOfChannelsIn)  {
    //
    jack_bufferIn  = (t_sample *)PD_MEMORY_GET (JACK_BUFFER_SIZE * sizeof (t_sample) * numberOfChannelsIn);
    //
    }

    if (numberOfChannelsOut) {
    //
    jack_bufferOut = (t_sample *)PD_MEMORY_GET (JACK_BUFFER_SIZE * sizeof (t_sample) * numberOfChannelsOut);
    //
    }

    jack_set_process_callback (jack_client, jack_pollCallback, NULL);
    jack_on_shutdown (jack_client, jack_shutdownCallback, NULL);

    for (i = 0; i < numberOfChannelsIn; i++) {
    //
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "input_%d", i + 1);
    jack_portsIn[i] = jack_port_register (jack_client, t, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    if (!jack_portsIn[i]) {
        error_failed (sym_JACK);
        break;
    }
    //
    }

    audio_vectorShrinkIn (jack_numberOfPortsIn = numberOfChannelsIn = i);
    
    for (i = 0; i < numberOfChannelsOut; i++) {
    //
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "output_%d", i + 1);
    jack_portsOut[i] = jack_port_register (jack_client, t, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if (!jack_portsOut[i]) {
        error_failed (sym_JACK);
        break;  
    }
    //
    }
    
    audio_vectorShrinkOut (jack_numberOfPortsOut = numberOfChannelsOut = i);
    
    if (!jack_activate (jack_client)) {
    //
    pthread_mutex_init (&jack_mutex, NULL);
    pthread_cond_init (&jack_cond, NULL);
    
    return PD_ERROR_NONE;
    }
    //
    }
    //
    }
        
    return PD_ERROR;
}

void audio_closeNative (void) 
{
    if (jack_client) {
    //
    int i;
    jack_deactivate (jack_client);
    for (i = 0; i < jack_numberOfPortsIn; i++)  { 
        jack_port_unregister (jack_client, jack_portsIn[i]); 
        jack_portsIn[i] = NULL;
    }
    for (i = 0; i < jack_numberOfPortsOut; i++) { 
        jack_port_unregister (jack_client, jack_portsOut[i]);
        jack_portsOut[i] = NULL;
    }
    jack_client_close (jack_client);
    jack_client = NULL;
    //
    }
    
    pthread_cond_broadcast (&jack_cond);
    pthread_cond_destroy (&jack_cond);
    pthread_mutex_destroy (&jack_mutex);
    
    if (jack_bufferIn)  { PD_MEMORY_FREE (jack_bufferIn);  jack_bufferIn = NULL;  }
    if (jack_bufferOut) { PD_MEMORY_FREE (jack_bufferOut); jack_bufferOut = NULL; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int audio_pollNative (void)
{
    int status = DACS_YES;
    
    if (!jack_client || (!jack_numberOfPortsIn && !jack_numberOfPortsOut)) { return DACS_NO; }
    else {
    //
    int i;
    t_sample *p = NULL;
    double now = clock_getRealTimeInSeconds();
    size_t size = INTERNAL_BLOCKSIZE * sizeof (t_sample);
    
    pthread_mutex_lock (&jack_mutex);
    
    if (jack_framesFilled >= jack_framesRequired) { pthread_cond_wait (&jack_cond, &jack_mutex); }

    p = audio_soundOut;
    
    for (i = 0; i < jack_numberOfPortsOut; i++) {
        memcpy (jack_bufferOut + (i * JACK_BUFFER_SIZE) + jack_framesFilled, p, size);
        memset (p, 0, size);
        p += INTERNAL_BLOCKSIZE;  
    }
    
    p = audio_soundIn;
    
    for (i = 0; i < jack_numberOfPortsIn; i++) {
        memcpy (p, jack_bufferIn + (i * JACK_BUFFER_SIZE) + jack_framesFilled, size);
        p += INTERNAL_BLOCKSIZE;
    }
    
    jack_framesFilled += INTERNAL_BLOCKSIZE;
    
    pthread_mutex_unlock (&jack_mutex);

    if (clock_getRealTimeInSeconds() - now > PD_MILLISECONDS_TO_SECONDS (2)) { status = DACS_SLEPT; }
    //
    }
    
    return status;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error audio_getListsNative (t_deviceslist *p) 
{
    t_error err = PD_ERROR_NONE;
    
    err |= deviceslist_appendAudioInWithString (p,  "JACK ports", 0);
    err |= deviceslist_appendAudioOutWithString (p, "JACK ports", 0);

    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
