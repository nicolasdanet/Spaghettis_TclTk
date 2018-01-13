
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ( PD_WITH_DEBUG && PD_WITH_LOGGER )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directorySupport;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_ringbuffer                    *logger_ring;                   /* Static. */

static int                      logger_file;                    /* Static. */
static int                      logger_running;                 /* Static. */
static pthread_t                logger_thread;                  /* Static. */
static t_int32Atomic            logger_quit;                    /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define LOGGER_CHUNK            64
#define LOGGER_SLEEP            57
#define LOGGER_BUFFER_SIZE      65536                           /* Power of two. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *logger_task (void *dummy)
{
    while (PD_ATOMIC_INT32_READ (&logger_quit) == 0) {
    //
    nano_sleep (MILLISECONDS_TO_NANOSECONDS (LOGGER_SLEEP));
    
    {
        char t[LOGGER_CHUNK + 1] = { 0 };
        int32_t size;
    
        while ((size = ringbuffer_getAvailableRead (logger_ring)) > 0) {
            int32_t k = PD_MIN (LOGGER_CHUNK, size);
            ringbuffer_read (logger_ring, t, k);
            t[k] = '\n';
            write (logger_file, t, k + 1);
        }
    }
    //
    }
    
    return (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error logger_initialize (void)
{
    char t[PD_STRING] = { 0 };
    t_error err = string_sprintf (t, PD_STRING, "%s/log-XXXXXX", main_directorySupport->s_name);
    
    if (!err && (logger_file = mkstemp (t)) != -1) {
    //
    logger_ring = ringbuffer_new (1, LOGGER_BUFFER_SIZE);
    
    if (!(err = (pthread_create (&logger_thread, NULL, logger_task, NULL) != 0))) {
        logger_running = 1;
    } else {
        ringbuffer_free (logger_ring); close (logger_file);
    }

    return err;
    //
    }
    
    return PD_ERROR;
}

void logger_release (void)
{
    logger_running = 0;
    
    PD_ATOMIC_INT32_WRITE (1, &logger_quit);
    
    pthread_join (logger_thread, NULL);
    
    ringbuffer_free (logger_ring); close (logger_file);
}

int logger_isRunning (void)
{
    return logger_running;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void logger_appendString (const char *s)
{
    ringbuffer_write (logger_ring, s, (int32_t)strlen (s));
}

void logger_appendFloat (double f)
{
    char t[LOGGER_FLOAT_STRING] = { 0 }; logger_appendString (logger_stringWithFloat (t, f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error logger_initialize (void)
{
    return PD_ERROR_NONE;
}

void logger_release (void)
{
}

int logger_isRunning (void)
{
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
