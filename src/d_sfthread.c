
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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
            size_t required = PD_MIN (SFTHREAD_CHUNK, x->sft_remainsToRead);
            int bytes = (int)read (x->sft_fileDescriptor, t, required);
            if (bytes > 0) {
                ringbuffer_write (x->sft_buffer, t, bytes);
                x->sft_remainsToRead -= bytes;
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
    
    while (1) {
    //
    while (ringbuffer_getAvailableRead (x->sft_buffer) > 0) {
        char t[SFTHREAD_CHUNK] = { 0 };
        int required = PD_MIN (SFTHREAD_CHUNK, x->sft_maximumToWrite - x->sft_alreadyWritten);
        int loaded   = ringbuffer_read (x->sft_buffer, t, required);
        int written  = (int)write (x->sft_fileDescriptor, t, (size_t)loaded);
        
        x->sft_alreadyWritten += written;
        
        if (required == 0) { PD_ATOMIC_INT32_WRITE (SFTHREAD_QUIT, &x->sft_flag); break; }
        if (written != loaded) {
        
            /* FIXME: File corrupted; what to do? */
            
            PD_ATOMIC_INT32_WRITE (SFTHREAD_QUIT, &x->sft_flag); break; 
        }
    }
    
    if (!PD_ATOMIC_INT32_READ (&x->sft_flag)) {
        nano_sleep (SFTHREAD_SLEEP);
    } else {
        break;
    }
    //
    }
    
    {
    //
    int size = x->sft_properties.ap_numberOfChannels * x->sft_properties.ap_bytesPerSample;
    int framesWritten = x->sft_alreadyWritten / size;
    
    if (soundfile_writeFileClose (x->sft_fileDescriptor, framesWritten, &x->sft_properties)) {
        /* FIXME: File corrupted; what to do? */
        /* Occurs also if stopped before demanded frames. */
    }
    //
    }
    
    close (x->sft_fileDescriptor);
    
    return (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_sfthread *sfthread_new (int type, int bufferSize, int fd, t_audioproperties *p)
{
    t_sfthread *x = (t_sfthread *)pd_new (sfthread_class);
    
    bufferSize = PD_MAX (SFTHREAD_CHUNK * 2, bufferSize);
    
    soundfile_propertiesCopy (&x->sft_properties, p);
    
    x->sft_type           = type;
    x->sft_fileDescriptor = fd;
    x->sft_buffer         = ringbuffer_new (1, bufferSize);
    
    if (x->sft_type == SFTHREAD_READER) {
    //
    int dataSize = x->sft_properties.ap_dataSizeInBytes;
    int frames   = x->sft_properties.ap_numberOfFrames;

    if (frames  != SOUNDFILE_UNKNOWN) {
        int size = x->sft_properties.ap_numberOfChannels * x->sft_properties.ap_bytesPerSample;
        dataSize = PD_MIN (dataSize, frames * size);
    }
        
    x->sft_remainsToRead = PD_MAX (0, dataSize);
    x->sft_error = (pthread_create (&x->sft_thread, NULL, sfthread_readerThread, (void *)x) != 0);
    //
    } else {
    //
    int dataSize = PD_INT_MAX;
    int frames   = x->sft_properties.ap_numberOfFrames;

    if (frames  != SOUNDFILE_UNKNOWN) {
        int size = x->sft_properties.ap_numberOfChannels * x->sft_properties.ap_bytesPerSample;
        dataSize = PD_MIN (dataSize, frames * size);
    }
    
    x->sft_maximumToWrite = PD_MAX (0, dataSize);
    x->sft_error = (pthread_create (&x->sft_thread, NULL, sfthread_writerThread, (void *)x) != 0);
    //
    }
    
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
