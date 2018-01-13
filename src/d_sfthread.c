
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"
#include "d_soundfile.h"
#include "d_sfthread.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SFTHREAD_QUIT       1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SFTHREAD_CHUNK      4096
#define SFTHREAD_SLEEP      MILLISECONDS_TO_NANOSECONDS (37.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *sfthread_class;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *sfthread_readerThread (void *z)
{
    t_sfthread *x = (t_sfthread *)z;
    
    while (!PD_ATOMIC_INT32_READ (&x->sft_flag)) {
    //
    while (ringbuffer_getAvailableWrite (x->sft_buffer) > SFTHREAD_CHUNK) {
        if (!PD_ATOMIC_INT32_READ (&x->sft_flag)) {
            char t[SFTHREAD_CHUNK] = { 0 };
            size_t required = PD_MIN (SFTHREAD_CHUNK, x->sft_data);
            int bytes = (int)read (x->sft_fileDescriptor, t, required);
            if (bytes > 0) {
                ringbuffer_write (x->sft_buffer, t, bytes);
                x->sft_data -= bytes;
            } else {
                PD_ATOMIC_INT32_WRITE (SFTHREAD_QUIT, &x->sft_flag);
            }
        } else {
            break;
        }
    }
    
    if (!PD_ATOMIC_INT32_READ (&x->sft_flag)) {
        nano_sleep (SFTHREAD_SLEEP);
    }
    //
    }
    
    close (x->sft_fileDescriptor);
    
    return (NULL);
}

static void *sfthread_writerThread (void *z)
{
    t_sfthread *x = (t_sfthread *)z;
    
    while (!PD_ATOMIC_INT32_READ (&x->sft_flag)) {
    //
    nano_sleep (SFTHREAD_SLEEP);
    //
    }
    
    close (x->sft_fileDescriptor);
    
    return (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_sfthread *sfthread_new (int type, int bufferSize, int fd, int dataSize)
{
    t_sfthread *x = (t_sfthread *)pd_new (sfthread_class);
    
    bufferSize = PD_MAX (SFTHREAD_CHUNK * 2, bufferSize);
    
    x->sft_type           = type;
    x->sft_fileDescriptor = fd;
    x->sft_data           = PD_MAX (0, dataSize);
    x->sft_buffer         = ringbuffer_new (1, bufferSize);
    
    x->sft_error = (pthread_create (&x->sft_thread,
        NULL,
        (x->sft_type == SFTHREAD_WRITER) ? sfthread_writerThread : sfthread_readerThread,
        (void *)x) != 0);
    
    if (x->sft_error) { pd_free (cast_pd (x)); x = NULL; PD_BUG; }
    
    return x;
}

static void sfthread_free (t_sfthread *x)
{
    if (x->sft_error) { close (x->sft_fileDescriptor); }
    else {
        pthread_join (x->sft_thread, NULL);
    }
    
    ringbuffer_free (x->sft_buffer);
}

void sfthread_release (t_sfthread  *x)
{
    PD_ATOMIC_INT32_WRITE (SFTHREAD_QUIT, &x->sft_flag);
    
    instance_autoreleaseRegister (cast_pd (x));
}

static void sfthread_autorelease (t_sfthread *x)
{
    instance_autoreleaseProceed (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void sfthread_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_sfthread,
            NULL,
            (t_method)sfthread_free,
            sizeof (t_sfthread),
            CLASS_NOBOX,
            A_NULL);
    
    class_addAutorelease (c, (t_method)sfthread_autorelease);
    
    sfthread_class = c;
}

void sfthread_destroy (void)
{
    class_free (sfthread_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
