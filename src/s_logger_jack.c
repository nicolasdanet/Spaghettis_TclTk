
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

#if PD_WITH_LOGGER

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "jack/jack.h"
#include "jack/ringbuffer.h"
#include "jack/weakjack.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryExtras;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static jack_ringbuffer_t    *logger_ring;                   /* Shared. */

static int                  logger_file;                    /* Shared. */
static pthread_t            logger_thread;                  /* Shared. */
static pthread_attr_t       logger_attribute;               /* Shared. */
static int                  logger_quit;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define JACK_LOGGER_CHUNK           (64)
#define JACK_LOGGER_SLEEP           (57)
#define JACK_LOGGER_BUFFER_SIZE     (1024 * JACK_LOGGER_SLEEP)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *logger_task (void *dummy)
{
    while (!logger_quit) {
    //
    usleep (MILLISECONDS_TO_MICROSECONDS (JACK_LOGGER_SLEEP));
    
    {
        char t[JACK_LOGGER_CHUNK + 1] = { 0 };
        size_t size;
        
        while ((size = jack_ringbuffer_read_space (logger_ring)) > 0) {
            size_t k = PD_MIN (JACK_LOGGER_CHUNK, size);
            jack_ringbuffer_read (logger_ring, t, k);
            t[k] = '\n';
            write (logger_file, t, k + 1);
        }
    }
    //
    }
    
    pthread_exit (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error logger_initializeNative (void)
{
    size_t k = PD_NEXTPOWER2 (JACK_LOGGER_BUFFER_SIZE);
    char t[PD_STRING] = { 0 };
    t_error err = string_sprintf (t, PD_STRING, "%s/log-XXXXXX", main_directoryExtras->s_name);
    
    if ((logger_file = mkstemp (t)) != -1) {
    //
    if ((logger_ring = jack_ringbuffer_create (k))) {
        pthread_attr_init (&logger_attribute);
        pthread_attr_setdetachstate (&logger_attribute, PTHREAD_CREATE_JOINABLE);
        return (pthread_create (&logger_thread, &logger_attribute, logger_task, NULL) != 0);
    }
    //
    }
    
    return PD_ERROR;
}

void logger_releaseNative (void)
{
    logger_quit = 1;
    pthread_join (logger_thread, NULL);
    pthread_attr_destroy (&logger_attribute);
    
    if (logger_ring) { 
        jack_ringbuffer_free (logger_ring);
        logger_ring = NULL; 
    }
    
    if (logger_file != -1) { close (logger_file); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void logger_appendStringNative (const char *s)
{    
    jack_ringbuffer_write (logger_ring, s, strlen (s));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_WITH_LOGGER

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
