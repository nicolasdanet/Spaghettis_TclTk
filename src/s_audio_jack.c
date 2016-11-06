
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "jack/weakjack.h"
#include "jack/jack.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <regex.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_MAXIMUM_CLIENTS    128
#define JACK_MAXIMUM_PORTS      128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample *audio_soundIn;
extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static char                 *jack_clientNames[JACK_MAXIMUM_CLIENTS];            /* Shared. */
static jack_client_t        *jack_client;                                       /* Shared. */

static t_sample             *jack_bufferIn;                                     /* Shared. */
static t_sample             *jack_bufferOut;                                    /* Shared. */
static jack_port_t          *jack_portsIn[JACK_MAXIMUM_PORTS];                  /* Shared. */
static jack_port_t          *jack_portsOut[JACK_MAXIMUM_PORTS];                 /* Shared. */

static int                  jack_numberOfPortsIn;                               /* Shared. */
static int                  jack_numberOfPortsOut;                              /* Shared. */

static jack_nframes_t       jack_framesRequired;                                /* Shared. */
static jack_nframes_t       jack_framesFilled;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static pthread_cond_t       jack_cond;                                          /* Shared. */
static pthread_mutex_t      jack_mutex;                                         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_BUFFER_SIZE    4096        /* Buffer size (per channel). */ 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int jack_pollCallback (jack_nframes_t numberOfFrames, void *dummy)
{
    int i;
    jack_default_audio_sample_t *in  = NULL;
    jack_default_audio_sample_t *out = NULL;

    size_t size = numberOfFrames * sizeof (t_sample);
    
    pthread_mutex_lock (&jack_mutex);
        
    jack_framesRequired = PD_MAX (numberOfFrames, INTERNAL_BLOCKSIZE);
    
    if (jack_framesFilled >= numberOfFrames) {

        PD_ASSERT (jack_framesFilled == numberOfFrames);
        
        for (i = 0; i < jack_numberOfPortsOut; i++) {
            if (out = jack_port_get_buffer (jack_portsOut[i], numberOfFrames)) {
                memcpy (out, jack_bufferOut + (i * JACK_BUFFER_SIZE), size);
            }
        }
        
        for (i = 0; i < jack_numberOfPortsIn; i++) {
            if (in = jack_port_get_buffer (jack_portsIn[i], numberOfFrames)) {
                memcpy (jack_bufferIn + (i * JACK_BUFFER_SIZE), in, size);
            }
        }

    } else {    /* Fill with zeros. */

        for (i = 0; i < jack_numberOfPortsOut; i++) {
            if (out = jack_port_get_buffer (jack_portsOut[i], numberOfFrames)) {
                memset (out, 0, size);
            }
        }
    }
    
    jack_framesFilled = 0;
    
    pthread_cond_broadcast (&jack_cond);
    pthread_mutex_unlock (&jack_mutex);
    
    return 0;
}

static int jack_blockSizeCallback (jack_nframes_t blockSize, void *dummy)
{
    audio_setBlockSize (blockSize);
    
    return 0;
}

static int jack_sampleRateCallback (jack_nframes_t sampleRate, void *dummy)
{
    audio_setSampleRate (sampleRate);
    
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
#pragma mark -

static void jack_freeClientNames (void)
{
    int i;
    for (i = 0; i < JACK_MAXIMUM_CLIENTS; i++) {
        if (jack_clientNames[i]) { PD_MEMORY_FREE (jack_clientNames[i]); } jack_clientNames[i] = NULL;
    }
}

static void jack_fetchClientNames (void)
{
    const char **ports;
    
    PD_ASSERT (jack_client_name_size() <= PD_STRING);
    
    jack_freeClientNames();
    
    ports = jack_get_ports (jack_client, "", "", 0);
    
    if (ports) {
    //
    int i, n = 0;
    
    regex_t e; regcomp (&e, "^[^:]*", REG_EXTENDED);

    for (i = 0; ports[i] != NULL && n < JACK_MAXIMUM_CLIENTS; i++) {
    //
    int j, seen = 0;
    size_t size = 0;
    regmatch_t info;
    char t[PD_STRING] = { 0 };
    
    /* Parse "clientname:portname" syntax (i.e. "system:playback_1" to "system"). */
    
    regexec (&e, ports[i], 1, &info, 0);
    size = PD_MIN (info.rm_eo - info.rm_so, PD_STRING - 1);
    memcpy (t, &ports[i][info.rm_so], size);
    t[size] = 0;
        
    /* Do we know about this port's client yet? */

    for (j = 0; j < n; j++) { if (strcmp (t, jack_clientNames[j]) == 0) { seen = 1; } }

    /* Append the unknown ones. */
    
    if (!seen) {
    //
    jack_clientNames[n] = PD_MEMORY_GET (strlen (t) + 1);

    if ((strcmp ("alsa_pcm", t) == 0) && (n > 0)) {    /* The "alsa_pcm" client MUST be the first. */
        char *tmp = jack_clientNames[n];
        jack_clientNames[n] = jack_clientNames[0];
        jack_clientNames[0] = tmp;
        strcpy (jack_clientNames[0], t);
        
    } else {
        strcpy (jack_clientNames[n], t);
    }
    
    n++;
    //
    }
    //
    }

    jack_free (ports);
    //
    }
}

static void jack_connectAllPortsToFirstClient (void)
{
    if (jack_clientNames[0] != NULL) {
    //
    const char **ports;
    char t[PD_STRING] = { 0 };

    string_sprintf (t, PD_STRING, "%s:.*", jack_clientNames[0]);

    ports = jack_get_ports (jack_client, t, NULL, JackPortIsOutput);
    
    if (ports) {
        int i;
        for (i = 0; ports[i] != NULL && i < jack_numberOfPortsIn; i++) {
            jack_connect (jack_client, ports[i], jack_port_name (jack_portsIn[i]));
        }
        jack_free (ports);
    }
    
    ports = jack_get_ports (jack_client, t, NULL, JackPortIsInput);
    
    if (ports) {
        int i;
        for (i = 0; ports[i] != NULL && i < jack_numberOfPortsOut; i++) {     
            jack_connect (jack_client, jack_port_name (jack_portsOut[i]), ports[i]);
        }
        jack_free (ports);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *audio_nameNative (void)
{
    static char *name = "JACK"; return name;
}

int audio_getPriorityNative (int min, int max, int isWatchdog)
{
    return (isWatchdog ? min + 7 : min + 5);
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
#pragma mark -

t_error audio_openNative (int sampleRate, 
    int numberOfChannelsIn, 
    int numberOfChannelsOut, 
    int blockSize, 
    int deviceIn, 
    int deviceOut)
{
    (void)sampleRate;
    (void)blockSize;
    (void)deviceIn;
    (void)deviceOut;
    
    #if PD_APPLE    /* Jackmp linked as a weak framework. */
        
    if (!jack_client_open) {
        error_canNotFind (sym_audio, sym_JACK); return PD_ERROR;
    }
    
    #endif

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
    
    if (jack_bufferIn)  { PD_MEMORY_FREE (jack_bufferIn);  }
    if (jack_bufferOut) { PD_MEMORY_FREE (jack_bufferOut); }
    
    if (numberOfChannelsIn) { 
        jack_bufferIn = PD_MEMORY_GET (numberOfChannelsIn * JACK_BUFFER_SIZE * sizeof (t_sample));
    }

    if (numberOfChannelsOut) {
        jack_bufferOut = PD_MEMORY_GET (numberOfChannelsOut * JACK_BUFFER_SIZE * sizeof (t_sample));
    }

    jack_set_process_callback (jack_client, jack_pollCallback, NULL);
    jack_set_buffer_size_callback (jack_client, jack_blockSizeCallback, NULL);
    jack_set_sample_rate_callback (jack_client, jack_sampleRateCallback, NULL);
    jack_on_shutdown (jack_client, jack_shutdownCallback, NULL);

    audio_setSampleRate (jack_get_sample_rate (jack_client));

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

    audio_shrinkChannelsIn (jack_numberOfPortsIn = numberOfChannelsIn = i);
    
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
    
    audio_shrinkChannelsOut (jack_numberOfPortsOut = numberOfChannelsOut = i);
    
    if (!jack_activate (jack_client)) {
    //
    jack_fetchClientNames();
    jack_connectAllPortsToFirstClient();
    
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
    
    jack_freeClientNames();
    
    if (jack_bufferIn)  { PD_MEMORY_FREE (jack_bufferIn);  jack_bufferIn = NULL;  }
    if (jack_bufferOut) { PD_MEMORY_FREE (jack_bufferOut); jack_bufferOut = NULL; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_pollDSPNative (void)
{
    int status = DACS_YES;
    
    if (!jack_client || (!jack_numberOfPortsIn && !jack_numberOfPortsOut)) { return DACS_NO; }
    else {
    //
    int i;
    t_sample *p = NULL;
    double now = sys_getRealTimeInSeconds();
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

    if (sys_getRealTimeInSeconds() - now > MILLISECONDS_TO_SECONDS (2)) { status = DACS_SLEPT; }
    //
    }
    
    return status;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_getListsNative (char *devicesIn, 
    int *numberOfDevicesIn,
    char *devicesOut,
    int *numberOfDevicesOut,
    int *canMultiple) 
{
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (devicesIn,  MAXIMUM_DESCRIPTION, "JACK ports");
    err |= string_copy (devicesOut, MAXIMUM_DESCRIPTION, "JACK ports");
    
    *numberOfDevicesIn  = 1;
    *numberOfDevicesOut = 1;
    *canMultiple        = 0;
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
