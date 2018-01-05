
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
static pthread_t                logger_thread;                  /* Static. */
static pthread_attr_t           logger_attribute;               /* Static. */
static int                      logger_quit;                    /* Static. */
static int                      logger_running;                 /* Static. */

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
    while (!logger_quit) {
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
    
    pthread_exit (NULL);
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
    
    pthread_attr_init (&logger_attribute);
    pthread_attr_setdetachstate (&logger_attribute, PTHREAD_CREATE_JOINABLE);
    
    if (!(err = (pthread_create (&logger_thread, &logger_attribute, logger_task, NULL) != 0))) {
        logger_running = 1;
    }

    return err;
    //
    }
    
    return PD_ERROR;
}

void logger_release (void)
{
    logger_quit = 1;
    logger_running = 0;
    
    pthread_join (logger_thread, NULL);
    pthread_attr_destroy (&logger_attribute);
    
    ringbuffer_free (logger_ring);
    
    if (logger_file != -1) { close (logger_file); }
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
