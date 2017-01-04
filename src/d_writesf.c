
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

/* < http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing */
/* < http://atastypixel.com/blog/four-common-mistakes-in-audio-development/ */

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

static t_class *writesf_tilde_class;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _writesf_tilde {
    t_object            sf_obj;                     /* Must be the first. */
    t_float             sf_f;
    t_float             sf_sampleRateOfSignal;
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
    int                 sf_bufferSize;
    char                *sf_buffer;
    t_glist             *sf_owner;
    t_sample            *(sf_vectorsOut[SOUNDFILE_MAXIMUM_CHANNELS]);
    pthread_mutex_t     sf_mutex;
    pthread_cond_t      sf_condRequest;
    pthread_cond_t      sf_condAnswer;
    pthread_t           sf_thread;
    } t_writesf_tilde;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *writesf_tilde_thread(void *zz)
{
    t_writesf_tilde *x = zz;

    pthread_mutex_lock(&x->sf_mutex);
    while (1)
    {
        if (x->sf_threadRequest == SOUNDFILE_REQUEST_NOTHING)
        {
            pthread_cond_signal(&x->sf_condAnswer);
            pthread_cond_wait(&x->sf_condRequest, &x->sf_mutex);
        }
        else if (x->sf_threadRequest == SOUNDFILE_REQUEST_OPEN)
        {
            char boo[80];
            int fd, sysrtn, writebytes;
            
                /* copy file stuff out of the data structure so we can
                relinquish the mutex while we're in open_soundfile(). */
            long onsetframes = x->sf_properties.ap_onset;
            long bytelimit = SOUNDFILE_UNKNOWN;
            int skipheaderbytes = x->sf_properties.ap_headerSize;
            int bytespersample = x->sf_properties.ap_bytesPerSample;
            int sfchannels = x->sf_properties.ap_numberOfChannels;
            int bigendian = x->sf_properties.ap_isBigEndian;
            int filetype = x->sf_properties.ap_fileType;
            char *filename = x->sf_properties.ap_fileName->s_name;
            char *fileExtension = x->sf_properties.ap_fileExtension->s_name;
            t_glist *canvas = x->sf_owner;
            t_float samplerate = x->sf_properties.ap_sampleRate;

                /* alter the request code so that an ensuing "open" will get
                noticed. */
            x->sf_threadRequest = SOUNDFILE_REQUEST_BUSY;
            //x->sf_error = 0;

                /* if there's already a file open, close it.  This
                should never happen since writesf_tilde_open() calls stop if
                needed and then waits until we're idle. */
            if (x->sf_fileDescriptor >= 0)
            {
                int bytesperframe = x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels;
                int bigendian = x->sf_properties.ap_isBigEndian;
                char *filename = x->sf_properties.ap_fileName->s_name;
                char *fileExtension = x->sf_properties.ap_fileExtension->s_name;
                int fd = x->sf_fileDescriptor;
                int filetype = x->sf_properties.ap_fileType;
                int itemswritten = x->sf_itemsWritten;
                int swap = x->sf_properties.ap_needToSwap;
                pthread_mutex_unlock(&x->sf_mutex);
                
                t_audioproperties toto;  soundfile_initProperties (&toto);
                
                toto.ap_fileType = x->sf_properties.ap_fileType;
                toto.ap_numberOfFrames = SOUNDFILE_UNKNOWN;
                toto.ap_numberOfChannels = x->sf_properties.ap_numberOfChannels;
                toto.ap_bytesPerSample = x->sf_properties.ap_bytesPerSample;
                toto.ap_isBigEndian = x->sf_properties.ap_isBigEndian;
                toto.ap_needToSwap = x->sf_properties.ap_needToSwap;
                
                soundfile_writeFileClose (fd, itemswritten, &toto);
                close (fd);

                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;

                if (x->sf_threadRequest != SOUNDFILE_REQUEST_BUSY)
                    continue;
            }
                /* open the soundfile with the mutex unlocked */
            pthread_mutex_unlock(&x->sf_mutex);
            
            t_audioproperties prop; soundfile_initProperties (&prop);
            
            prop.ap_fileName = gensym (filename);
            prop.ap_fileExtension = gensym (fileExtension);
            prop.ap_sampleRate = samplerate;
            prop.ap_fileType = filetype;
            prop.ap_numberOfChannels = sfchannels;
            prop.ap_bytesPerSample = bytespersample;
            prop.ap_isBigEndian = bigendian;
            prop.ap_needToSwap = soundfile_systemIsBigEndian() != bigendian;
            prop.ap_numberOfFrames = 0;
    
            fd = soundfile_writeFileHeader (canvas, &prop);
                    
            pthread_mutex_lock(&x->sf_mutex);

            if (fd < 0)
            {
                x->sf_fileDescriptor = -1;
                x->sf_isEndOfFile = 1;
                //x->sf_error = errno;
                x->sf_threadRequest = SOUNDFILE_REQUEST_NOTHING;
                continue;
            }
            /* check if another request has been made; if so, field it */
            if (x->sf_threadRequest != SOUNDFILE_REQUEST_BUSY)
                continue;

            x->sf_fileDescriptor = fd;
            x->sf_fifoTail = 0;
            x->sf_itemsWritten = 0;
            x->sf_properties.ap_needToSwap = soundfile_systemIsBigEndian() != bigendian;      
                /* in a loop, wait for the fifo to have data and write it
                    to disk */
            while (x->sf_threadRequest == SOUNDFILE_REQUEST_BUSY ||
                (x->sf_threadRequest == SOUNDFILE_REQUEST_CLOSE &&
                    x->sf_fifoHead != x->sf_fifoTail))
            {
                int fifosize = x->sf_fifoSize, fifotail;
                char *buf = x->sf_buffer;

                    /* if the head is < the tail, we can immediately write
                    from tail to end of fifo to disk; otherwise we hold off
                    writing until there are at least SOUNDFILE_CHUNK_SIZE bytes in the
                    buffer */
                if (x->sf_fifoHead < x->sf_fifoTail ||
                    x->sf_fifoHead >= x->sf_fifoTail + SOUNDFILE_CHUNK_SIZE
                    || (x->sf_threadRequest == SOUNDFILE_REQUEST_CLOSE &&
                        x->sf_fifoHead != x->sf_fifoTail))
                {
                    writebytes = (x->sf_fifoHead < x->sf_fifoTail ?
                        fifosize : x->sf_fifoHead) - x->sf_fifoTail;
                    if (writebytes > SOUNDFILE_CHUNK_SIZE)
                        writebytes = SOUNDFILE_CHUNK_SIZE;
                }
                else
                {

                    pthread_cond_signal(&x->sf_condAnswer);

                    pthread_cond_wait(&x->sf_condRequest,
                        &x->sf_mutex);

                    continue;
                }
                fifotail = x->sf_fifoTail;
                fd = x->sf_fileDescriptor;
                pthread_mutex_unlock(&x->sf_mutex);
                sysrtn = write(fd, buf + fifotail, writebytes);
                pthread_mutex_lock(&x->sf_mutex);
                if (x->sf_threadRequest != SOUNDFILE_REQUEST_BUSY &&
                    x->sf_threadRequest != SOUNDFILE_REQUEST_CLOSE)
                        break;
                if (sysrtn < writebytes)
                {
                    //x->sf_error = errno;
                    break;
                }
                else
                {
                    x->sf_fifoTail += sysrtn;
                    if (x->sf_fifoTail == fifosize)
                        x->sf_fifoTail = 0;
                }
                x->sf_itemsWritten +=
                    sysrtn / (x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels);

                    /* signal parent in case it's waiting for data */
                pthread_cond_signal(&x->sf_condAnswer);
            }
        }
        else if (x->sf_threadRequest == SOUNDFILE_REQUEST_CLOSE ||
            x->sf_threadRequest == SOUNDFILE_REQUEST_QUIT)
        {
            int quit = (x->sf_threadRequest == SOUNDFILE_REQUEST_QUIT);
            if (x->sf_fileDescriptor >= 0)
            {
                int bytesperframe = x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels;
                int bigendian = x->sf_properties.ap_isBigEndian;
                char *filename = x->sf_properties.ap_fileName->s_name;
                int fd = x->sf_fileDescriptor;
                int filetype = x->sf_properties.ap_fileType;
                int itemswritten = x->sf_itemsWritten;
                int swap = x->sf_properties.ap_needToSwap;
                
                pthread_mutex_unlock(&x->sf_mutex);

                t_audioproperties toto;  soundfile_initProperties (&toto);
                
                toto.ap_fileType = x->sf_properties.ap_fileType;
                toto.ap_numberOfFrames = SOUNDFILE_UNKNOWN;
                toto.ap_numberOfChannels = x->sf_properties.ap_numberOfChannels;
                toto.ap_bytesPerSample = x->sf_properties.ap_bytesPerSample;
                toto.ap_isBigEndian = x->sf_properties.ap_isBigEndian;
                toto.ap_needToSwap = x->sf_properties.ap_needToSwap;

    
                soundfile_writeFileClose (fd, itemswritten, &toto);
                close (fd);

                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;
            }
            x->sf_threadRequest = SOUNDFILE_REQUEST_NOTHING;
            pthread_cond_signal(&x->sf_condAnswer);
            if (quit)
                break;
        }
        else
        {

        }
    }

    pthread_mutex_unlock(&x->sf_mutex);
    return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void writesf_tilde_start (t_writesf_tilde *x)
{
    if (x->sf_threadState == SOUNDFILE_STATE_START) { x->sf_threadState = SOUNDFILE_STATE_STREAM; }
    else {
        error_unexpected (sym_writesf__tilde__, sym_start);
    }
}

static void writesf_tilde_stop (t_writesf_tilde *x)
{
    pthread_mutex_lock (&x->sf_mutex);
    
        x->sf_threadState   = SOUNDFILE_STATE_IDLE;
        x->sf_threadRequest = SOUNDFILE_REQUEST_CLOSE;

    pthread_cond_signal (&x->sf_condRequest);
    pthread_mutex_unlock (&x->sf_mutex);
}

static void writesf_tilde_open (t_writesf_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *filesym;
    t_symbol *fileExtension;
    int filetype, bytespersamp, swap, bigendian, normalize;
    long onset, nframes;
    t_float samplerate;
    if (x->sf_threadState != SOUNDFILE_STATE_IDLE)
    {
        writesf_tilde_stop(x);
    }
    
    t_audioproperties prop; soundfile_initProperties (&prop);
    
    if (soundfile_writeFileParse(sym_writesf__tilde__, &argc, &argv, &prop) == PD_ERROR)
    {
        post_error ("writesf~: usage: open [-bytes [234]] [-wave,-nextstep,-aiff] ...");
        post("... [-big,-little] [-rate ####] filename");
        return;
    }
    
    filesym = prop.ap_fileName;
    fileExtension = prop.ap_fileExtension;
    samplerate = prop.ap_sampleRate;
    filetype = prop.ap_fileType;
    bytespersamp = prop.ap_bytesPerSample;
    bigendian = prop.ap_isBigEndian;
    swap = prop.ap_needToSwap;
    onset = prop.ap_onset;
    nframes = prop.ap_numberOfFrames;
    normalize = prop.ap_needToNormalize;
    
    if (normalize || onset || (nframes != SOUNDFILE_UNKNOWN))
        post_error ("normalize/onset/nframes argument to writesf~: ignored");
    if (argc)
        post_error ("extra argument(s) to writesf~: ignored");
    pthread_mutex_lock(&x->sf_mutex);
    while (x->sf_threadRequest != SOUNDFILE_REQUEST_NOTHING)
    {
        pthread_cond_signal(&x->sf_condRequest);
        pthread_cond_wait(&x->sf_condAnswer, &x->sf_mutex);
    }
    x->sf_properties.ap_bytesPerSample = bytespersamp;
    x->sf_properties.ap_needToSwap = swap;
    x->sf_properties.ap_isBigEndian = bigendian;
    x->sf_properties.ap_fileName = filesym;
    x->sf_properties.ap_fileExtension = fileExtension;
    x->sf_properties.ap_fileType = filetype;
    x->sf_itemsWritten = 0;
    x->sf_threadRequest = SOUNDFILE_REQUEST_OPEN;
    x->sf_fifoTail = 0;
    x->sf_fifoHead = 0;
    x->sf_isEndOfFile = 0;
    //x->sf_error = 0;
    x->sf_threadState = SOUNDFILE_STATE_START;
    x->sf_properties.ap_bytesPerSample = (bytespersamp > 2 ? bytespersamp : 2);
    if (samplerate > 0)
        x->sf_properties.ap_sampleRate = samplerate;
    else if (x->sf_sampleRateOfSignal > 0)
        x->sf_properties.ap_sampleRate = x->sf_sampleRateOfSignal;
    else x->sf_properties.ap_sampleRate = audio_getSampleRate();
        /* set fifosize from bufsize.  fifosize must be a
        multiple of the number of bytes eaten for each DSP
        tick.  */
    x->sf_fifoSize = x->sf_bufferSize - (x->sf_bufferSize %
        (x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels * x->sf_vectorSize));
            /* arrange for the "request" condition to be signalled 16
            times per buffer */
    x->sf_fifoCount = x->sf_fifoPeriod = (x->sf_fifoSize /
            (16 * x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels * x->sf_vectorSize));
    pthread_cond_signal(&x->sf_condRequest);
    pthread_mutex_unlock(&x->sf_mutex);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *writesf_tilde_perform(t_int *w)
{
    t_writesf_tilde *x = (t_writesf_tilde *)(w[1]);
    int vecsize = x->sf_vectorSize, sfchannels = x->sf_properties.ap_numberOfChannels, i, j,
        bytespersample = x->sf_properties.ap_bytesPerSample,
        bigendian = x->sf_properties.ap_isBigEndian;
    t_sample *fp;
    if (x->sf_threadState == SOUNDFILE_STATE_STREAM)
    {
        int wantbytes, roominfifo;
        pthread_mutex_lock(&x->sf_mutex);
        wantbytes = sfchannels * vecsize * bytespersample;
        roominfifo = x->sf_fifoTail - x->sf_fifoHead;
        if (roominfifo <= 0)
            roominfifo += x->sf_fifoSize;
        while (roominfifo < wantbytes + 1)
        {
            fprintf(stderr, "writesf waiting for disk write..\n");
            fprintf(stderr, "(head %d, tail %d, room %d, want %d)\n",
                x->sf_fifoHead, x->sf_fifoTail, roominfifo, wantbytes);
            pthread_cond_signal(&x->sf_condRequest);
            pthread_cond_wait(&x->sf_condAnswer, &x->sf_mutex);
            fprintf(stderr, "... done waiting.\n");
            roominfifo = x->sf_fifoTail - x->sf_fifoHead;
            if (roominfifo <= 0)
                roominfifo += x->sf_fifoSize;
        }

        soundfile_encode (sfchannels, x->sf_vectorsOut,
            (unsigned char *)(x->sf_buffer + x->sf_fifoHead), vecsize, 0,
                bytespersample, bigendian, 1, 1.);
        
        x->sf_fifoHead += wantbytes;
        if (x->sf_fifoHead >= x->sf_fifoSize)
            x->sf_fifoHead = 0;
        if ((--x->sf_fifoCount) <= 0)
        {
            pthread_cond_signal(&x->sf_condRequest);
            x->sf_fifoCount = x->sf_fifoPeriod;
        }
        pthread_mutex_unlock(&x->sf_mutex);
    }
    return (w+2);
}

static void writesf_tilde_dsp(t_writesf_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vectorSize == AUDIO_DEFAULT_BLOCKSIZE);

    int i, ninlets = x->sf_properties.ap_numberOfChannels;
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_vectorSize = sp[0]->s_vectorSize;
    x->sf_fifoPeriod = (x->sf_fifoSize /
            (16 * x->sf_properties.ap_bytesPerSample * x->sf_properties.ap_numberOfChannels * x->sf_vectorSize));
    for (i = 0; i < ninlets; i++)
        x->sf_vectorsOut[i] = sp[i]->s_vector;
    x->sf_sampleRateOfSignal = sp[0]->s_sampleRate;
    pthread_mutex_unlock(&x->sf_mutex);
    dsp_add(writesf_tilde_perform, 1, x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *writesf_tilde_new (t_float f1, t_float f2)
{
    t_error err = PD_ERROR_NONE;
    
    int i, n = PD_CLAMP ((int)f1, 1, SOUNDFILE_MAXIMUM_CHANNELS);
    int size = PD_CLAMP ((int)f2, SOUNDFILE_CHUNK_SIZE * 4 * n, SOUNDFILE_CHUNK_SIZE * 256 * n);

    t_writesf_tilde *x = (t_writesf_tilde *)pd_new (writesf_tilde_class);
    
    soundfile_initProperties (&x->sf_properties);
    
    x->sf_properties.ap_bytesPerSample   = 2;       /* ??? */
    x->sf_properties.ap_numberOfChannels = n;
    
    x->sf_vectorSize        = AUDIO_DEFAULT_BLOCKSIZE;
    x->sf_threadState       = SOUNDFILE_STATE_IDLE;
    x->sf_threadRequest     = SOUNDFILE_REQUEST_NOTHING;
    x->sf_fileDescriptor    = -1;
    x->sf_bufferSize        = size;
    x->sf_buffer            = PD_MEMORY_GET (x->sf_bufferSize);
    x->sf_owner             = canvas_getCurrent();
    
    for (i = 1; i < n; i++) { inlet_newSignal (cast_object (x)); }

    err |= (pthread_mutex_init (&x->sf_mutex, NULL) != 0);
    err |= (pthread_cond_init (&x->sf_condRequest, NULL) != 0);
    err |= (pthread_cond_init (&x->sf_condAnswer, NULL) != 0);
    err |= (pthread_create (&x->sf_thread, NULL, writesf_tilde_thread, (void *)x) != 0);

    PD_ASSERT (!err);
    
    return x;
}

static void writesf_tilde_free (t_writesf_tilde *x)
{
    void *dummy = NULL;
    
    /* Release the worker thread. */
    
    pthread_mutex_lock (&x->sf_mutex);
    
        x->sf_threadRequest = SOUNDFILE_REQUEST_QUIT;
        pthread_cond_signal (&x->sf_condRequest);
        
        while (x->sf_threadRequest != SOUNDFILE_REQUEST_NOTHING) {
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
#pragma mark -

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
