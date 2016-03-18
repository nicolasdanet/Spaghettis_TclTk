
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

#include "portaudio.h"
#include "pa_ringbuffer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char                 *logger_buffer;                 /* Shared. */

static PaUtilRingBuffer     logger_ring;                    /* Shared. */
static pthread_t            logger_thread;                  /* Shared. */
static pthread_attr_t       logger_attribute;               /* Shared. */
static int                  logger_quit;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PA_LOGGER_CHUNK         (64)
#define PA_LOGGER_SLEEP         (57)
#define PA_LOGGER_BUFFER_SIZE   (1024 * PA_LOGGER_SLEEP)    /* Allowing 1024 characters per millisecond. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *logger_task (void *dummy)
{
    while (!logger_quit) {
    //
    usleep (MILLISECONDS_TO_MICROSECONDS (PA_LOGGER_SLEEP));
    
    {
        char t[PA_LOGGER_CHUNK + 1] = { 0 };
        while (PaUtil_GetRingBufferReadAvailable (&logger_ring) >= PA_LOGGER_CHUNK) {
            PaUtil_ReadRingBuffer (&logger_ring, t, PA_LOGGER_CHUNK);
            t[PA_LOGGER_CHUNK] = 0;
            post_log ("%s", t);
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
    size_t k = PD_NEXT_POWER_OF_TWO (PA_LOGGER_BUFFER_SIZE);
    
    logger_buffer = PD_MEMORY_GET (k);
    
    if (!PaUtil_InitializeRingBuffer (&logger_ring, sizeof (char), k, logger_buffer)) {
        pthread_attr_init (&logger_attribute);
        pthread_attr_setdetachstate (&logger_attribute, PTHREAD_CREATE_JOINABLE);
        return (pthread_create (&logger_thread, &logger_attribute, logger_task, NULL) != 0);
    }
    
    return PD_ERROR;
}

void logger_releaseNative (void)
{
    logger_quit = 1;
    pthread_join (logger_thread, NULL);
    pthread_attr_destroy (&logger_attribute);
    
    if (logger_buffer) { 
        PD_MEMORY_FREE (logger_buffer);
        logger_buffer = NULL; 
    }
}

void logger_appendStringNative (const char* s, size_t size)
{
    PaUtil_WriteRingBuffer (&logger_ring, s, (ring_buffer_size_t)size);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
