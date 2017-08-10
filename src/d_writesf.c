
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that this object use a nasty mutex inside the DSP method. */
/* Probably best to rewrite it entirely with a lock-free circular buffer. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing > */
/* < http://atastypixel.com/blog/four-common-mistakes-in-audio-development/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_soundfile.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *writesf_tilde_class;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _writesf_tilde {
    t_object            sf_obj;                     /* Must be the first. */
    t_float             sf_f;
    t_float             sf_signalSampleRate;
    t_audioproperties   sf_properties;
    t_error             sf_error;
    int                 sf_vectorSize;
    int                 sf_threadState;
    int                 sf_threadRequest;
    int                 sf_fileDescriptor;
    int                 sf_fifoSize;
    int                 sf_fifoHead;
    int                 sf_fifoTail;
    int                 sf_fifoCount;
    int                 sf_fifoPeriod;
    int                 sf_isEndOfFile;
    int                 sf_itemsWritten;
    int                 sf_numberOfChannels;
    int                 sf_bufferSize;
    char                *sf_buffer;
    t_glist             *sf_owner;
    t_sample            *(sf_vectorsOut[SOUNDFILE_CHANNELS]);
    pthread_mutex_t     sf_mutex;
    pthread_cond_t      sf_condRequest;
    pthread_cond_t      sf_condAnswer;
    pthread_t           sf_thread;
    } t_writesf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define WRITESF_BUFFER_SIZE         65536

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define WRITESF_REQUEST_NOTHING     0
#define WRITESF_REQUEST_OPEN        1
#define WRITESF_REQUEST_CLOSE       2
#define WRITESF_REQUEST_QUIT        3
#define WRITESF_REQUEST_BUSY        4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define WRITESF_STATE_IDLE          0
#define WRITESF_STATE_START         1
#define WRITESF_STATE_STREAM        2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define WRITESF_NO_REQUEST          (x->sf_threadRequest == WRITESF_REQUEST_BUSY)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void writesf_tilde_threadCloseFile (t_writesf_tilde *x)
{
    if (x->sf_fileDescriptor >= 0) {
    //
    int f = x->sf_fileDescriptor;
    int itemsWritten = x->sf_itemsWritten;
    
    t_audioproperties properties; soundfile_setPropertiesByCopy (&properties, &x->sf_properties);
    
    properties.ap_numberOfFrames = SOUNDFILE_UNKNOWN;
    
    x->sf_fileDescriptor = -1;
    
    pthread_mutex_unlock (&x->sf_mutex);

        soundfile_writeFileClose (f, itemsWritten, &properties);
        
        close (f);

    pthread_mutex_lock (&x->sf_mutex);
    //
    }
}

static inline int writesf_tilde_threadOpenLoopRun (t_writesf_tilde *x)
{
    if (x->sf_threadRequest == WRITESF_REQUEST_QUIT)  { return 0; }
    if (x->sf_threadRequest == WRITESF_REQUEST_BUSY)  { return 1; }
    if (x->sf_threadRequest == WRITESF_REQUEST_CLOSE) {
        if (x->sf_fifoHead != x->sf_fifoTail) {
            return 1; 
        }
    }
    
    return 0;
}

static inline int writesf_tilde_threadOpenLoopNeedToWrite (t_writesf_tilde *x)
{
    if (x->sf_fifoHead < x->sf_fifoTail) { return 1; }
    if (x->sf_fifoHead >= x->sf_fifoTail + WRITESF_BUFFER_SIZE) { return 1; }
    if (x->sf_threadRequest == WRITESF_REQUEST_CLOSE) {
        if (x->sf_fifoHead != x->sf_fifoTail) {
            return 1; 
        }
    }
    
    return 0;
}

static int writesf_tilde_threadOpenLoopWrite (t_writesf_tilde * x, int m, int n)
{
    int bytes = -1;
    
    if (m < (PD_INT_MAX - n)) {                  /* Reached the soundfile limit? */
    //
    int f   = x->sf_fileDescriptor;
    char *t = x->sf_buffer + x->sf_fifoTail;
    
    pthread_mutex_unlock (&x->sf_mutex);
        
        bytes = (int)write (f, t, (size_t)m);
        
    pthread_mutex_lock (&x->sf_mutex);
    //
    }
    
    return bytes;
}

static void writesf_tilde_threadOpenLoop (t_writesf_tilde *x)
{
    while (writesf_tilde_threadOpenLoopRun (x)) {
    //
    if (writesf_tilde_threadOpenLoopNeedToWrite (x)) {
    //
    int bytesToWrite = 0;
    
    if (x->sf_fifoHead < x->sf_fifoTail) { bytesToWrite = x->sf_fifoSize - x->sf_fifoTail; }
    else {
        bytesToWrite = x->sf_fifoHead - x->sf_fifoTail;
    }
    
    bytesToWrite = PD_MIN (bytesToWrite, WRITESF_BUFFER_SIZE);
    
    if (bytesToWrite) {

        int bytesPerFrame       = x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels;
        int bytesAlreadyWritten = x->sf_itemsWritten * bytesPerFrame;
        int bytesWritten        = writesf_tilde_threadOpenLoopWrite (x, bytesToWrite, bytesAlreadyWritten);

        if (bytesWritten > 0) { x->sf_itemsWritten += bytesWritten / bytesPerFrame; }
        
        if (bytesWritten < bytesToWrite) {
            x->sf_error = PD_ERROR; 
            x->sf_threadRequest = WRITESF_REQUEST_NOTHING;
            break;
            
        } else {
            x->sf_fifoTail += bytesWritten;
            if (x->sf_fifoTail == x->sf_fifoSize) {
                x->sf_fifoTail = 0;
            }
            pthread_cond_signal (&x->sf_condAnswer);
        }
    //
    }
    //
    } else {
        pthread_cond_signal (&x->sf_condAnswer); pthread_cond_wait (&x->sf_condRequest, &x->sf_mutex);
    }
    //
    }
}

static void writesf_tilde_threadNothing (t_writesf_tilde *x)
{
    pthread_cond_signal (&x->sf_condAnswer); pthread_cond_wait (&x->sf_condRequest, &x->sf_mutex);
}

static void writesf_tilde_threadOpen (t_writesf_tilde *x)
{
    x->sf_threadRequest = WRITESF_REQUEST_BUSY;
    x->sf_error = PD_ERROR_NONE;
        
    writesf_tilde_threadCloseFile (x);

    if (WRITESF_NO_REQUEST) {       /* The request could have been changed once releasing the lock. */
    //
    int f = -1;
        
    /* Make a local copy to used it unlocked. */
    
    t_audioproperties copy; soundfile_setPropertiesByCopy (&copy, &x->sf_properties);
    
    pthread_mutex_unlock (&x->sf_mutex);
    
        f = soundfile_writeFileHeader (x->sf_owner, &copy);
            
    pthread_mutex_lock (&x->sf_mutex);

    x->sf_fileDescriptor = f;
    
    if (x->sf_fileDescriptor < 0) {
        x->sf_error = PD_ERROR;
        x->sf_threadRequest = WRITESF_REQUEST_NOTHING;
    
    /* Wait for the fifo to have data and write it to disk. */
    
    } else if (WRITESF_NO_REQUEST) { writesf_tilde_threadOpenLoop (x); }
    //
    }
}

static void writesf_tilde_threadClose (t_writesf_tilde *x)
{
    writesf_tilde_threadCloseFile (x);
    
    x->sf_threadRequest = WRITESF_REQUEST_NOTHING;
    
    pthread_cond_signal (&x->sf_condAnswer);
}

static void *writesf_tilde_thread (void *z)
{
    t_writesf_tilde *x = z;

    pthread_mutex_lock (&x->sf_mutex);
    
        while (1) {
        //
        int t = x->sf_threadRequest;
        
        if (t == WRITESF_REQUEST_NOTHING)    { writesf_tilde_threadNothing (x);  }
        else if (t == WRITESF_REQUEST_OPEN)  { writesf_tilde_threadOpen (x);     }
        else if (t == WRITESF_REQUEST_CLOSE) { writesf_tilde_threadClose (x);    }
        else if (t == WRITESF_REQUEST_QUIT)  {
            writesf_tilde_threadClose (x);
            break;
        }
        //
        }

    pthread_mutex_unlock (&x->sf_mutex);
    
    return (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void writesf_tilde_start (t_writesf_tilde *x)
{
    if (x->sf_threadState == WRITESF_STATE_START) { x->sf_threadState = WRITESF_STATE_STREAM; }
    else {
        error_unexpected (sym_writesf__tilde__, sym_start);
    }
}

static void writesf_tilde_stop (t_writesf_tilde *x)
{
    x->sf_threadState = WRITESF_STATE_IDLE;
    
    pthread_mutex_lock (&x->sf_mutex);
        
        x->sf_threadRequest = WRITESF_REQUEST_CLOSE;

    pthread_cond_signal (&x->sf_condRequest);
    pthread_mutex_unlock (&x->sf_mutex);
}

static void writesf_tilde_open (t_writesf_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    t_audioproperties properties; soundfile_initProperties (&properties);
    
    if (x->sf_threadState != WRITESF_STATE_IDLE) { writesf_tilde_stop (x); }
    
    err = soundfile_writeFileParse (sym_writesf__tilde__, &argc, &argv, &properties);

    if (x->sf_signalSampleRate > 0) { properties.ap_sampleRate = x->sf_signalSampleRate; }
    
    if (!err) {
    //
    pthread_mutex_lock (&x->sf_mutex);
    
        while (x->sf_threadRequest != WRITESF_REQUEST_NOTHING) {
            pthread_cond_signal (&x->sf_condRequest);
            pthread_cond_wait (&x->sf_condAnswer, &x->sf_mutex);
        }
        
        soundfile_setPropertiesByCopy (&x->sf_properties, &properties);
        
        x->sf_properties.ap_numberOfChannels = x->sf_numberOfChannels;
        
        {
            x->sf_threadState   = WRITESF_STATE_START;
            x->sf_threadRequest = WRITESF_REQUEST_OPEN;
            x->sf_isEndOfFile   = 0;
            x->sf_itemsWritten  = 0;
        
            /* The size of the fifo must be a multiple of the number of bytes per DSP tick. */
            /* The "request" condition will be signalled 16 times among the buffer. */
            
            int bytesPerFrame   = x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels;
            int bytesPerTick    = bytesPerFrame * x->sf_vectorSize;
            
            x->sf_fifoTail      = 0;
            x->sf_fifoHead      = 0;
            x->sf_fifoSize      = x->sf_bufferSize - (x->sf_bufferSize % bytesPerTick);
            x->sf_fifoPeriod    = x->sf_fifoSize / (16 * bytesPerTick);
            x->sf_fifoCount     = x->sf_fifoPeriod;
        }
        
    pthread_cond_signal (&x->sf_condRequest);
    pthread_mutex_unlock (&x->sf_mutex);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int writesf_tilde_performGetAvailableSize (t_writesf_tilde *x)
{
    int k = x->sf_fifoTail - x->sf_fifoHead;
    if (k <= 0) {
        k += x->sf_fifoSize;
    }
    
    return k;
}

static inline void writesf_tilde_performStoreIfNecessary (t_writesf_tilde * x)
{
    x->sf_fifoCount--;
    
    if (x->sf_fifoCount <= 0) {
        pthread_cond_signal (&x->sf_condRequest); x->sf_fifoCount = x->sf_fifoPeriod;
    }
}

static t_int *writesf_tilde_perform (t_int *w)
{
    t_writesf_tilde *x = (t_writesf_tilde *)(w[1]);

    pthread_mutex_lock (&x->sf_mutex);
    
    if (!x->sf_error && x->sf_threadState == WRITESF_STATE_STREAM) {
    //
    int bytesPerFrame = x->sf_properties.ap_numberOfChannels * x->sf_properties.ap_bytesPerSample;
    int bytesToWrite  = x->sf_vectorSize * bytesPerFrame;
    int availableSize = writesf_tilde_performGetAvailableSize (x);
        
    while (availableSize < bytesToWrite + 1) {
        pthread_cond_signal (&x->sf_condRequest);
        pthread_cond_wait (&x->sf_condAnswer, &x->sf_mutex);
        availableSize = writesf_tilde_performGetAvailableSize (x);
    }

    soundfile_encode (x->sf_properties.ap_numberOfChannels,
        x->sf_vectorsOut,
        (unsigned char *)(x->sf_buffer + x->sf_fifoHead),
        x->sf_vectorSize,
        0,
        x->sf_properties.ap_bytesPerSample,
        x->sf_properties.ap_isBigEndian,
        1,
        (t_sample)1.0);
    
    x->sf_fifoHead += bytesToWrite; if (x->sf_fifoHead >= x->sf_fifoSize) { x->sf_fifoHead = 0; }
    
    writesf_tilde_performStoreIfNecessary (x);
    //
    }
    
    pthread_mutex_unlock (&x->sf_mutex);
    
    return (w + 2);
}

static void writesf_tilde_dsp (t_writesf_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vectorSize == AUDIO_DEFAULT_BLOCKSIZE);     /* Not implemented yet. */
    PD_ABORT  (sp[0]->s_vectorSize != AUDIO_DEFAULT_BLOCKSIZE);
    
    int i;
    
    pthread_mutex_lock (&x->sf_mutex);
    
        x->sf_signalSampleRate = sp[0]->s_sampleRate;
    
        for (i = 0; i < x->sf_numberOfChannels; i++) { x->sf_vectorsOut[i] = sp[i]->s_vector; }
    
    pthread_mutex_unlock (&x->sf_mutex);
    
    dsp_add (writesf_tilde_perform, 1, x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *writesf_tilde_new (t_float f1, t_float f2)
{
    t_error err = PD_ERROR_NONE;
    
    int i, n = PD_CLAMP ((int)f1, 1, SOUNDFILE_CHANNELS);
    int size = PD_CLAMP ((int)f2, WRITESF_BUFFER_SIZE * 4 * n, WRITESF_BUFFER_SIZE * 256 * n);

    t_writesf_tilde *x = (t_writesf_tilde *)pd_new (writesf_tilde_class);
    
    soundfile_initProperties (&x->sf_properties);
    
    x->sf_vectorSize        = AUDIO_DEFAULT_BLOCKSIZE;
    x->sf_threadState       = WRITESF_STATE_IDLE;
    x->sf_threadRequest     = WRITESF_REQUEST_NOTHING;
    x->sf_fileDescriptor    = -1;
    x->sf_numberOfChannels  = n;
    x->sf_bufferSize        = size;
    x->sf_buffer            = (char *)PD_MEMORY_GET (x->sf_bufferSize);
    x->sf_owner             = instance_contextGetCurrent();
    
    for (i = 1; i < x->sf_numberOfChannels; i++) { inlet_newSignal (cast_object (x)); }

    err |= (pthread_mutex_init (&x->sf_mutex, NULL) != 0);
    err |= (pthread_cond_init (&x->sf_condRequest, NULL) != 0);
    err |= (pthread_cond_init (&x->sf_condAnswer, NULL) != 0);
    err |= (pthread_create (&x->sf_thread, NULL, writesf_tilde_thread, (void *)x) != 0);

    PD_UNUSED (err); PD_ASSERT (!err);
    
    return x;
}

static void writesf_tilde_free (t_writesf_tilde *x)
{
    void *dummy = NULL;
    
    /* Release the worker thread. */
    
    pthread_mutex_lock (&x->sf_mutex);
    
        x->sf_threadRequest = WRITESF_REQUEST_QUIT;
        pthread_cond_signal (&x->sf_condRequest);
        
        while (x->sf_threadRequest != WRITESF_REQUEST_NOTHING) {
            pthread_cond_signal (&x->sf_condRequest);
            pthread_cond_wait (&x->sf_condAnswer, &x->sf_mutex);
        }
    
    pthread_mutex_unlock (&x->sf_mutex);
    
    if (pthread_join (x->sf_thread, &dummy)) { PD_BUG; }
    
    /* Once done, free the object. */
    
    pthread_cond_destroy (&x->sf_condRequest);
    pthread_cond_destroy (&x->sf_condAnswer);
    pthread_mutex_destroy (&x->sf_mutex);
    
    PD_MEMORY_FREE (x->sf_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void writesf_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_writesf__tilde__,
            (t_newmethod)writesf_tilde_new, 
            (t_method)writesf_tilde_free,
            sizeof (t_writesf_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
        
    CLASS_SIGNAL (c, t_writesf_tilde, sf_f);
    
    class_addDSP (c, (t_method)writesf_tilde_dsp);
        
    class_addMethod (c, (t_method)writesf_tilde_start,  sym_start,  A_NULL);
    class_addMethod (c, (t_method)writesf_tilde_stop,   sym_stop,   A_NULL);
    class_addMethod (c, (t_method)writesf_tilde_open,   sym_open,   A_GIMME, A_NULL);

    writesf_tilde_class = c;
}

void writesf_tilde_destroy (void)
{
    class_free (writesf_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
