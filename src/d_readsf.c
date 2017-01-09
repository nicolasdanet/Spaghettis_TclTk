
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

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
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_soundfile.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *readsf_tilde_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _readsf_tilde {
    t_object            sf_obj;                 /* Must be the first. */
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
    int                 sf_numberOfAudioOutlets;
    int                 sf_bufferSize;
    char                *sf_buffer;
    t_glist             *sf_owner;
    t_clock             *sf_clock;
    t_sample            *(sf_vectorsOut[SOUNDFILE_CHANNELS]);
    t_outlet            *(sf_audioOutlets[SOUNDFILE_CHANNELS]);
    t_outlet            *sf_outletTopRight;
    pthread_mutex_t     sf_mutex;
    pthread_cond_t      sf_condRequest;
    pthread_cond_t      sf_condAnswer;
    pthread_t           sf_thread;
    } t_readsf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void readsf_tilde_start  (t_readsf_tilde *);
static void readsf_tilde_stop   (t_readsf_tilde *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define READSF_BUFFER_SIZE          65536

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define READSF_REQUEST_NOTHING      0
#define READSF_REQUEST_OPEN         1
#define READSF_REQUEST_CLOSE        2
#define READSF_REQUEST_QUIT         3
#define READSF_REQUEST_BUSY         4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define READSF_STATE_IDLE           0
#define READSF_STATE_START          1
#define READSF_STATE_STREAM         2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define READSF_NO_REQUEST           (x->sf_threadRequest == READSF_REQUEST_BUSY)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void readsf_tilde_threadCloseFile (t_readsf_tilde * x)
{
    if (x->sf_fileDescriptor >= 0) {
    //
    int f = x->sf_fileDescriptor;
    
    x->sf_fileDescriptor = -1;
    
    pthread_mutex_unlock (&x->sf_mutex);
    
        close (f);
    
    pthread_mutex_lock (&x->sf_mutex);
    //
    }
}

static void readsf_tilde_threadCloseFileAndSignal (t_readsf_tilde * x, int n)
{
    readsf_tilde_threadCloseFile (x);
    
    if (n == READSF_REQUEST_QUIT || n == x->sf_threadRequest) {
        x->sf_threadRequest = READSF_REQUEST_NOTHING;
    }

    pthread_cond_signal (&x->sf_condAnswer);
}

static ssize_t readsf_tilde_threadOpenLoopRead (t_readsf_tilde * x, int n)
{
    char *t = x->sf_buffer + x->sf_fifoHead;
    int f   = x->sf_fileDescriptor;
    
    ssize_t bytes;
    
    pthread_mutex_unlock (&x->sf_mutex);
        
        bytes = read (f, t, (size_t)n);
        
    pthread_mutex_lock (&x->sf_mutex);
    
    return bytes;
}

static void readsf_tilde_threadOpenLoop (t_readsf_tilde * x)
{
    while (READSF_NO_REQUEST) {
    //
    if (x->sf_isEndOfFile) { break; }
    else {
    //
    int bytesToRead = 0;
    
    if (x->sf_fifoHead >= x->sf_fifoTail) {
    
    /* Avoid to completely fill the buffer and overwrite previously loaded samples. */
        
    if (x->sf_fifoTail || (x->sf_fifoSize - x->sf_fifoHead > READSF_BUFFER_SIZE)) {
        bytesToRead = x->sf_fifoSize - x->sf_fifoHead;
    }

    } else {
        bytesToRead = x->sf_fifoTail - x->sf_fifoHead - 1;
        bytesToRead = bytesToRead < READSF_BUFFER_SIZE ? 0 : READSF_BUFFER_SIZE;
    }

    if (bytesToRead > 0) { 
      
        ssize_t bytesRead;
        
        bytesToRead = PD_MIN (bytesToRead, READSF_BUFFER_SIZE);
        bytesToRead = PD_MIN (bytesToRead, x->sf_properties.ap_dataSizeInBytes);
        bytesRead   = readsf_tilde_threadOpenLoopRead (x, bytesToRead);

        if (bytesRead < 0)          { break; }
        else if (bytesRead == 0)    { x->sf_isEndOfFile = 1; }
        else if (READSF_NO_REQUEST) {
        
            x->sf_fifoHead += bytesRead;
            x->sf_properties.ap_dataSizeInBytes -= bytesRead;
                
            if (x->sf_fifoHead == x->sf_fifoSize) { x->sf_fifoHead = 0; }
            if (x->sf_properties.ap_dataSizeInBytes <= 0) { x->sf_isEndOfFile = 1; }
                
            pthread_cond_signal (&x->sf_condAnswer);
        }

    } else { pthread_cond_signal (&x->sf_condAnswer); pthread_cond_wait (&x->sf_condRequest, &x->sf_mutex); }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void readsf_tilde_threadNothing (t_readsf_tilde * x)
{ 
    pthread_cond_signal (&x->sf_condAnswer); pthread_cond_wait (&x->sf_condRequest, &x->sf_mutex);
}

static void readsf_tilde_threadOpen (t_readsf_tilde * x)
{
    x->sf_threadRequest = READSF_REQUEST_BUSY;
    x->sf_error = PD_ERROR_NONE;
    
    readsf_tilde_threadCloseFile (x);

    if (READSF_NO_REQUEST) {        /* The request could have been changed once releasing the lock. */
    //
    int f = -1;
    
    /* Make a local copy to used it unlocked. */
    
    t_audioproperties copy; soundfile_setPropertiesByCopy (&copy, &x->sf_properties);
    
    pthread_mutex_unlock (&x->sf_mutex);
    
        f = soundfile_readFileHeader (x->sf_owner, &copy);
        
    pthread_mutex_lock (&x->sf_mutex);

    x->sf_fileDescriptor = f;
    
    soundfile_setPropertiesByCopy (&x->sf_properties, &copy);
    
    if (x->sf_fileDescriptor < 0) { x->sf_error = PD_ERROR; }
    else if (READSF_NO_REQUEST) {

        int bytesPerFrame = x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels;
        int bytesPerTick  = bytesPerFrame * x->sf_vectorSize;
        
        /* The size of the fifo must be a multiple of the number of bytes per DSP tick. */
        /* The "request" condition will be signalled 16 times among the buffer. */
        
        x->sf_fifoHead   = 0;
        x->sf_fifoTail   = 0;
        x->sf_fifoSize   = x->sf_bufferSize - (x->sf_bufferSize % bytesPerTick);
        x->sf_fifoPeriod = x->sf_fifoSize / (16 * bytesPerTick);
        x->sf_fifoCount  = x->sf_fifoPeriod;
        
        /* Wait for the fifo to get hungry and feed it. */
        
        readsf_tilde_threadOpenLoop (x);
    }
    //
    }

    readsf_tilde_threadCloseFileAndSignal (x, READSF_REQUEST_BUSY);
}

static void readsf_tilde_threadClose (t_readsf_tilde * x)
{
    readsf_tilde_threadCloseFileAndSignal (x, READSF_REQUEST_CLOSE);
}

static void readsf_tilde_threadQuit (t_readsf_tilde * x)
{
    readsf_tilde_threadCloseFileAndSignal (x, READSF_REQUEST_QUIT);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *readsf_tilde_thread (void *z)
{
    t_readsf_tilde *x = z;

    pthread_mutex_lock (&x->sf_mutex);
    
        while (1) {
        //
        int t = x->sf_threadRequest;
        
        if (t == READSF_REQUEST_NOTHING)    { readsf_tilde_threadNothing (x); }
        else if (t == READSF_REQUEST_OPEN)  { readsf_tilde_threadOpen (x);    }        
        else if (t == READSF_REQUEST_CLOSE) { readsf_tilde_threadClose (x);   }
        else if (t == READSF_REQUEST_QUIT)  {
        //
        readsf_tilde_threadQuit (x);
        break;
        //
        }
        //
        }

    pthread_mutex_unlock (&x->sf_mutex);
    
    return (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void readsf_tilde_task (t_readsf_tilde *x)
{
    outlet_bang (x->sf_outletTopRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void readsf_tilde_float (t_readsf_tilde *x, t_float f)
{
    if (f != 0.0) { readsf_tilde_start (x); }
    else { 
        readsf_tilde_stop (x);
    }
}

static void readsf_tilde_start (t_readsf_tilde *x)
{
    if (x->sf_threadState == READSF_STATE_START) { x->sf_threadState = READSF_STATE_STREAM; }
    else { 
        error_unexpected (sym_readsf__tilde__, sym_start);
    }
}

static void readsf_tilde_stop (t_readsf_tilde *x)
{
    x->sf_threadState = READSF_STATE_IDLE;
    
    pthread_mutex_lock (&x->sf_mutex);
    
        x->sf_threadRequest = READSF_REQUEST_CLOSE;
    
    pthread_cond_signal (&x->sf_condRequest);
    pthread_mutex_unlock (&x->sf_mutex);
}

static void readsf_tilde_open (t_readsf_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_audioproperties properties; soundfile_initProperties (&properties);
    
    t_error err = soundfile_readFileParse (sym_readsf__tilde__, &argc, &argv, &properties);
    
    if (!err) {
    //
    if (canvas_openFileExist (x->sf_owner, 
        properties.ap_fileName->s_name, 
        properties.ap_fileExtension->s_name)) {

        pthread_mutex_lock (&x->sf_mutex);
        
            soundfile_setPropertiesByCopy (&x->sf_properties, &properties);
            
            x->sf_threadState            = READSF_STATE_START;
            x->sf_threadRequest          = READSF_REQUEST_OPEN;
            x->sf_fifoTail               = 0;
            x->sf_fifoHead               = 0;
            x->sf_isEndOfFile            = 0;
        
        pthread_cond_signal (&x->sf_condRequest);
        pthread_mutex_unlock (&x->sf_mutex);

    } else { error_canNotFind (sym_readsf__tilde__, properties.ap_fileName); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int readsf_tilde_performGetBytesPerTick (t_readsf_tilde * x)
{
    return (x->sf_properties.ap_numberOfChannels * x->sf_properties.ap_bytesPerSample * x->sf_vectorSize);
}

static inline int readsf_tilde_performIsAlmostEmpty (t_readsf_tilde * x, int n)
{
    return (x->sf_fifoHead >= x->sf_fifoTail && x->sf_fifoHead < x->sf_fifoTail + n - 1);
}

static inline void readsf_tilde_performLoadIfNecessary (t_readsf_tilde * x)
{
    x->sf_fifoCount--;
    
    if (x->sf_fifoCount <= 0) {
        pthread_cond_signal (&x->sf_condRequest); x->sf_fifoCount = x->sf_fifoPeriod;
    }
}

static inline void readsf_tilde_performZero (t_readsf_tilde * x, int onset)
{
    int i, j;
    
    for (i = 0; i < x->sf_numberOfAudioOutlets; i++) {

        PD_RESTRICTED out = x->sf_vectorsOut[i] + onset;
        for (j = 0; j < (x->sf_vectorSize - onset); j++) {
            *out++ = 0.0;
        }
    }
}

static inline void readsf_tilde_performEnd (t_readsf_tilde * x)
{
    int bytesPerFrames = x->sf_properties.ap_numberOfChannels * x->sf_properties.ap_bytesPerSample;
    int framesRemains  = (x->sf_fifoHead - x->sf_fifoTail + 1) / bytesPerFrames;
    
    clock_delay (x->sf_clock, 0.0);
    
    x->sf_threadState = READSF_STATE_IDLE;

    if (framesRemains) {
    
        soundfile_decode (x->sf_properties.ap_numberOfChannels,
            x->sf_vectorsOut,
            (unsigned char *)(x->sf_buffer + x->sf_fifoTail),
            framesRemains,
            0,
            x->sf_properties.ap_bytesPerSample,
            x->sf_properties.ap_isBigEndian,
            1,
            x->sf_numberOfAudioOutlets);
    }
    
    readsf_tilde_performZero (x, framesRemains);
    
    pthread_cond_signal (&x->sf_condRequest);
}

static inline void readsf_tilde_performRead (t_readsf_tilde * x, int n)
{
    soundfile_decode (x->sf_properties.ap_numberOfChannels,
        x->sf_vectorsOut,
        (unsigned char *)(x->sf_buffer + x->sf_fifoTail),
        x->sf_vectorSize,
        0,
        x->sf_properties.ap_bytesPerSample,
        x->sf_properties.ap_isBigEndian,
        1,
        x->sf_numberOfAudioOutlets);
    
    x->sf_fifoTail += n;
    
    if (x->sf_fifoTail >= x->sf_fifoSize) { x->sf_fifoTail = 0; }
    
    readsf_tilde_performLoadIfNecessary (x);
}

static t_int *readsf_tilde_perform (t_int *w)
{
    t_readsf_tilde *x = (t_readsf_tilde *)(w[1]);
    
    pthread_mutex_lock (&x->sf_mutex);
    
    if (x->sf_error || x->sf_threadState != READSF_STATE_STREAM) { readsf_tilde_performZero (x, 0); }
    else {

        int eof, bytesToRead = readsf_tilde_performGetBytesPerTick (x);
        
        while (!x->sf_isEndOfFile && readsf_tilde_performIsAlmostEmpty (x, bytesToRead)) {
        //
        pthread_cond_signal (&x->sf_condRequest);
        pthread_cond_wait (&x->sf_condAnswer, &x->sf_mutex);
            
        bytesToRead = readsf_tilde_performGetBytesPerTick (x);
        //
        }
        
        eof = (x->sf_isEndOfFile && readsf_tilde_performIsAlmostEmpty (x, bytesToRead));
        
        if (eof) { readsf_tilde_performEnd (x); }
        else {
            readsf_tilde_performRead (x, bytesToRead);
        }
    //
    }
    
    pthread_mutex_unlock (&x->sf_mutex);
    
    return (w + 2);
}

static void readsf_tilde_dsp (t_readsf_tilde *x, t_signal **sp)
{
    int i;
    
    PD_ASSERT (sp[0]->s_vectorSize == AUDIO_DEFAULT_BLOCKSIZE);     /* Not implemented yet. */
    PD_ABORT  (sp[0]->s_vectorSize != AUDIO_DEFAULT_BLOCKSIZE);
    
    pthread_mutex_lock (&x->sf_mutex);
    
        for (i = 0; i < x->sf_numberOfAudioOutlets; i++) { x->sf_vectorsOut[i] = sp[i]->s_vector; }
    
    pthread_mutex_unlock (&x->sf_mutex);
    
    dsp_add (readsf_tilde_perform, 1, x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *readsf_tilde_new (t_float f1, t_float f2)
{
    t_error err = PD_ERROR_NONE;
    
    int i, n = PD_CLAMP ((int)f1, 1, SOUNDFILE_CHANNELS);
    int size = PD_CLAMP ((int)f2, READSF_BUFFER_SIZE * 4 * n, READSF_BUFFER_SIZE * 256 * n);
    
    t_readsf_tilde *x = (t_readsf_tilde *)pd_new (readsf_tilde_class);
    
    soundfile_initProperties (&x->sf_properties);
    
    x->sf_vectorSize            = AUDIO_DEFAULT_BLOCKSIZE;
    x->sf_threadState           = READSF_STATE_IDLE;
    x->sf_threadRequest         = READSF_REQUEST_NOTHING;
    x->sf_fileDescriptor        = -1;
    x->sf_numberOfAudioOutlets  = n;
    x->sf_bufferSize            = size;
    x->sf_buffer                = PD_MEMORY_GET (x->sf_bufferSize);
    x->sf_owner                 = canvas_getCurrent();
    x->sf_clock                 = clock_new ((void *)x, (t_method)readsf_tilde_task);
    
    for (i = 0; i < n; i++) { x->sf_audioOutlets[i] = outlet_new (cast_object (x), &s_signal); }
    
    x->sf_outletTopRight = outlet_new (cast_object (x), &s_bang);
    
    err |= (pthread_mutex_init (&x->sf_mutex, NULL) != 0);
    err |= (pthread_cond_init (&x->sf_condRequest, NULL) != 0);
    err |= (pthread_cond_init (&x->sf_condAnswer, NULL) != 0);
    err |= (pthread_create (&x->sf_thread, NULL, readsf_tilde_thread, (void *)x) != 0);
    
    PD_ASSERT (!err);
    
    return x;
}

static void readsf_tilde_free (t_readsf_tilde *x)
{
    void *dummy = NULL;
    
    /* Release the worker thread. */
    
    pthread_mutex_lock (&x->sf_mutex);
    
        x->sf_threadRequest = READSF_REQUEST_QUIT;
        pthread_cond_signal (&x->sf_condRequest);
        
        while (x->sf_threadRequest != READSF_REQUEST_NOTHING) {
            pthread_cond_signal (&x->sf_condRequest); 
            pthread_cond_wait (&x->sf_condAnswer, &x->sf_mutex);
        }
    
    pthread_mutex_unlock (&x->sf_mutex);
    
    if (pthread_join (x->sf_thread, &dummy)) { PD_BUG; }
    
    /* Once done, free the object. */
    
    pthread_cond_destroy (&x->sf_condRequest);
    pthread_cond_destroy (&x->sf_condAnswer);
    pthread_mutex_destroy (&x->sf_mutex);
    
    clock_free (x->sf_clock);
    
    PD_MEMORY_FREE (x->sf_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void readsf_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_readsf__tilde__,
            (t_newmethod)readsf_tilde_new, 
            (t_method)readsf_tilde_free,
            sizeof (t_readsf_tilde),
            CLASS_DEFAULT, 
            A_DEFFLOAT, 
            A_DEFFLOAT,
            A_NULL);
    
    class_addDSP (c, (t_method)readsf_tilde_dsp);
    class_addFloat (c, (t_method)readsf_tilde_float);
    
    class_addMethod (c, (t_method)readsf_tilde_start,   sym_start,  A_NULL);
    class_addMethod (c, (t_method)readsf_tilde_stop,    sym_stop,   A_NULL);
    class_addMethod (c, (t_method)readsf_tilde_open,    sym_open,   A_GIMME, A_NULL);
    
    readsf_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
