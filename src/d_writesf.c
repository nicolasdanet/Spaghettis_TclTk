
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that this object use a nasty mutex inside the DSP method. */

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

extern t_class *garray_class;

/******************************* writesf *******************/
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *writesf_class;

typedef struct _writesf {
    t_object            sf_obj;                 /* Must be the first. */
    t_float             sf_f;
    t_float             sf_sampleRate;
    int                 sf_fileType;
    int                 sf_headerSize;
    int                 sf_numberOfChannels;
    int                 sf_bytesPerSample;
    int                 sf_isBigEndian;
    int                 sf_needToSwap;
    int                 sf_onset;
    t_float             sf_sampleRateOfInput;
    int                 sf_maximumBytesToRead;
    int                 sf_bufferSize;
    int                 sf_numberOfAudioOutlets;
    int                 sf_vectorSize;
    int                 sf_state;
    int                 sf_request;
    int                 sf_error;
    int                 sf_fileDescriptor;
    int                 sf_fifoSize;
    int                 sf_fifoHead;
    int                 sf_fifoTail;
    int                 sf_isEndOfFile;
    int                 sf_count;
    int                 sf_period;
    int                 sf_itemsWritten;
    pthread_mutex_t     sf_mutex;
    pthread_cond_t      sf_condRequest;
    pthread_cond_t      sf_condAnswer;
    pthread_t           sf_thread;
    t_sample            *(sf_vectorsOut[SOUNDFILE_MAXIMUM_CHANNELS]);
    char                *sf_buffer;
    t_symbol            *sf_fileName;
    t_symbol            *sf_fileExtension;
    t_glist             *sf_owner;
    t_clock             *sf_clock;
    t_outlet            *sf_outlet;
    } t_writesf;
    
/************** the child thread which performs file I/O ***********/

static void *writesf_child_main(void *zz)
{
    t_writesf *x = zz;
#ifdef DEBUG_SOUNDFILE
    pute("1\n");
#endif
    pthread_mutex_lock(&x->sf_mutex);
    while (1)
    {
#ifdef DEBUG_SOUNDFILE
        pute("0\n");
#endif
        if (x->sf_request == SOUNDFILE_REQUEST_NOTHING)
        {
#ifdef DEBUG_SOUNDFILE
            pute("wait 2\n");
#endif
            pthread_cond_signal(&x->sf_condAnswer);
            pthread_cond_wait(&x->sf_condRequest, &x->sf_mutex);
#ifdef DEBUG_SOUNDFILE
            pute("3\n");
#endif
        }
        else if (x->sf_request == SOUNDFILE_REQUEST_OPEN)
        {
            char boo[80];
            int fd, sysrtn, writebytes;
            
                /* copy file stuff out of the data structure so we can
                relinquish the mutex while we're in open_soundfile(). */
            long onsetframes = x->sf_onset;
            long bytelimit = SOUNDFILE_UNKNOWN;
            int skipheaderbytes = x->sf_headerSize;
            int bytespersample = x->sf_bytesPerSample;
            int sfchannels = x->sf_numberOfChannels;
            int bigendian = x->sf_isBigEndian;
            int filetype = x->sf_fileType;
            char *filename = x->sf_fileName->s_name;
            char *fileExtension = x->sf_fileExtension->s_name;
            t_glist *canvas = x->sf_owner;
            t_float samplerate = x->sf_sampleRate;

                /* alter the request code so that an ensuing "open" will get
                noticed. */
#ifdef DEBUG_SOUNDFILE
            pute("4\n");
#endif
            x->sf_request = SOUNDFILE_REQUEST_BUSY;
            x->sf_error = 0;

                /* if there's already a file open, close it.  This
                should never happen since writesf_open() calls stop if
                needed and then waits until we're idle. */
            if (x->sf_fileDescriptor >= 0)
            {
                int bytesperframe = x->sf_bytesPerSample * x->sf_numberOfChannels;
                int bigendian = x->sf_isBigEndian;
                char *filename = x->sf_fileName->s_name;
                char *fileExtension = x->sf_fileExtension->s_name;
                int fd = x->sf_fileDescriptor;
                int filetype = x->sf_fileType;
                int itemswritten = x->sf_itemsWritten;
                int swap = x->sf_needToSwap;
                pthread_mutex_unlock(&x->sf_mutex);
                
                t_audioproperties toto;  soundfile_initProperties (&toto);
                
                toto.ap_fileType = x->sf_fileType;
                toto.ap_numberOfFrames = SOUNDFILE_UNKNOWN;
                toto.ap_numberOfChannels = x->sf_numberOfChannels;
                toto.ap_bytesPerSample = x->sf_bytesPerSample;
                toto.ap_isBigEndian = x->sf_isBigEndian;
                toto.ap_needToSwap = x->sf_needToSwap;
                
                soundfile_writeFileClose (fd, itemswritten, &toto);
                close (fd);

                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;
#ifdef DEBUG_SOUNDFILE
                {
                    char s[1000];
                    sprintf(s, "bug ditched %d\n", itemswritten);
                    pute(s);
                }
#endif  
                if (x->sf_request != SOUNDFILE_REQUEST_BUSY)
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
#ifdef DEBUG_SOUNDFILE
            pute("5\n");
#endif

            if (fd < 0)
            {
                x->sf_fileDescriptor = -1;
                x->sf_isEndOfFile = 1;
                x->sf_error = errno;
#ifdef DEBUG_SOUNDFILE
                pute("open failed\n");
                pute(filename);
#endif
                x->sf_request = SOUNDFILE_REQUEST_NOTHING;
                continue;
            }
            /* check if another request has been made; if so, field it */
            if (x->sf_request != SOUNDFILE_REQUEST_BUSY)
                continue;
#ifdef DEBUG_SOUNDFILE
            pute("6\n");
#endif
            x->sf_fileDescriptor = fd;
            x->sf_fifoTail = 0;
            x->sf_itemsWritten = 0;
            x->sf_needToSwap = soundfile_systemIsBigEndian() != bigendian;      
                /* in a loop, wait for the fifo to have data and write it
                    to disk */
            while (x->sf_request == SOUNDFILE_REQUEST_BUSY ||
                (x->sf_request == SOUNDFILE_REQUEST_CLOSE &&
                    x->sf_fifoHead != x->sf_fifoTail))
            {
                int fifosize = x->sf_fifoSize, fifotail;
                char *buf = x->sf_buffer;
#ifdef DEBUG_SOUNDFILE
                pute("77\n");
#endif
                    /* if the head is < the tail, we can immediately write
                    from tail to end of fifo to disk; otherwise we hold off
                    writing until there are at least SOUNDFILE_CHUNK_SIZE bytes in the
                    buffer */
                if (x->sf_fifoHead < x->sf_fifoTail ||
                    x->sf_fifoHead >= x->sf_fifoTail + SOUNDFILE_CHUNK_SIZE
                    || (x->sf_request == SOUNDFILE_REQUEST_CLOSE &&
                        x->sf_fifoHead != x->sf_fifoTail))
                {
                    writebytes = (x->sf_fifoHead < x->sf_fifoTail ?
                        fifosize : x->sf_fifoHead) - x->sf_fifoTail;
                    if (writebytes > SOUNDFILE_CHUNK_SIZE)
                        writebytes = SOUNDFILE_CHUNK_SIZE;
                }
                else
                {
#ifdef DEBUG_SOUNDFILE
                    pute("wait 7a ...\n");
#endif
                    pthread_cond_signal(&x->sf_condAnswer);
#ifdef DEBUG_SOUNDFILE
                    pute("signalled\n");
#endif
                    pthread_cond_wait(&x->sf_condRequest,
                        &x->sf_mutex);
#ifdef DEBUG_SOUNDFILE
                    pute("7a done\n");
#endif
                    continue;
                }
#ifdef DEBUG_SOUNDFILE
                pute("8\n");
#endif
                fifotail = x->sf_fifoTail;
                fd = x->sf_fileDescriptor;
                pthread_mutex_unlock(&x->sf_mutex);
                sysrtn = write(fd, buf + fifotail, writebytes);
                pthread_mutex_lock(&x->sf_mutex);
                if (x->sf_request != SOUNDFILE_REQUEST_BUSY &&
                    x->sf_request != SOUNDFILE_REQUEST_CLOSE)
                        break;
                if (sysrtn < writebytes)
                {
#ifdef DEBUG_SOUNDFILE
                    pute("fileerror\n");
#endif
                    x->sf_error = errno;
                    break;
                }
                else
                {
                    x->sf_fifoTail += sysrtn;
                    if (x->sf_fifoTail == fifosize)
                        x->sf_fifoTail = 0;
                }
                x->sf_itemsWritten +=
                    sysrtn / (x->sf_bytesPerSample * x->sf_numberOfChannels);
#ifdef DEBUG_SOUNDFILE
                sprintf(boo, "after: head %d, tail %d written %d\n", 
                    x->sf_fifoHead, x->sf_fifoTail, x->sf_itemsWritten);
                pute(boo);
#endif
                    /* signal parent in case it's waiting for data */
                pthread_cond_signal(&x->sf_condAnswer);
            }
        }
        else if (x->sf_request == SOUNDFILE_REQUEST_CLOSE ||
            x->sf_request == SOUNDFILE_REQUEST_QUIT)
        {
            int quit = (x->sf_request == SOUNDFILE_REQUEST_QUIT);
            if (x->sf_fileDescriptor >= 0)
            {
                int bytesperframe = x->sf_bytesPerSample * x->sf_numberOfChannels;
                int bigendian = x->sf_isBigEndian;
                char *filename = x->sf_fileName->s_name;
                int fd = x->sf_fileDescriptor;
                int filetype = x->sf_fileType;
                int itemswritten = x->sf_itemsWritten;
                int swap = x->sf_needToSwap;
                
                pthread_mutex_unlock(&x->sf_mutex);

                t_audioproperties toto;  soundfile_initProperties (&toto);
                
                toto.ap_fileType = x->sf_fileType;
                toto.ap_numberOfFrames = SOUNDFILE_UNKNOWN;
                toto.ap_numberOfChannels = x->sf_numberOfChannels;
                toto.ap_bytesPerSample = x->sf_bytesPerSample;
                toto.ap_isBigEndian = x->sf_isBigEndian;
                toto.ap_needToSwap = x->sf_needToSwap;

    
                soundfile_writeFileClose (fd, itemswritten, &toto);
                close (fd);

                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;
            }
            x->sf_request = SOUNDFILE_REQUEST_NOTHING;
            pthread_cond_signal(&x->sf_condAnswer);
            if (quit)
                break;
        }
        else
        {
#ifdef DEBUG_SOUNDFILE
            pute("13\n");
#endif
        }
    }
#ifdef DEBUG_SOUNDFILE
    pute("thread exit\n");
#endif
    pthread_mutex_unlock(&x->sf_mutex);
    return (0);
}

/******** the object proper runs in the calling (parent) thread ****/

static void writesf_tick(t_writesf *x);

static void *writesf_new(t_float fnchannels, t_float fbufsize)
{
    t_writesf *x;
    int nchannels = fnchannels, bufsize = fbufsize, i;
    char *buf;
    
    if (nchannels < 1)
        nchannels = 1;
    else if (nchannels > SOUNDFILE_MAXIMUM_CHANNELS)
        nchannels = SOUNDFILE_MAXIMUM_CHANNELS;
    if (bufsize <= 0) bufsize = SOUNDFILE_CHUNK_SIZE * 4 * nchannels;
    else if (bufsize < SOUNDFILE_CHUNK_SIZE * 4)
        bufsize = SOUNDFILE_CHUNK_SIZE * 4;
    else if (bufsize > SOUNDFILE_CHUNK_SIZE * 256)
        bufsize = SOUNDFILE_CHUNK_SIZE * 256;
    buf = PD_MEMORY_GET(bufsize);
    if (!buf) return (0);
    
    x = (t_writesf *)pd_new(writesf_class);
    
    for (i = 1; i < nchannels; i++)
        inlet_new (cast_object (x), cast_pd (x), &s_signal, &s_signal);

    x->sf_f = 0;
    x->sf_numberOfChannels = nchannels;
    pthread_mutex_init(&x->sf_mutex, 0);
    pthread_cond_init(&x->sf_condRequest, 0);
    pthread_cond_init(&x->sf_condAnswer, 0);
    x->sf_vectorSize = AUDIO_DEFAULT_BLOCKSIZE;
    x->sf_sampleRateOfInput = x->sf_sampleRate = 0;
    x->sf_state = SOUNDFILE_STATE_IDLE;
    x->sf_clock = 0;     /* no callback needed here */
    x->sf_owner = canvas_getCurrent();
    x->sf_bytesPerSample = 2;
    x->sf_fileDescriptor = -1;
    x->sf_buffer = buf;
    x->sf_bufferSize = bufsize;
    x->sf_fifoSize = x->sf_fifoHead = x->sf_fifoTail = x->sf_request = 0;
    pthread_create(&x->sf_thread, 0, writesf_child_main, x);
    return x;
}

static t_int *writesf_perform(t_int *w)
{
    t_writesf *x = (t_writesf *)(w[1]);
    int vecsize = x->sf_vectorSize, sfchannels = x->sf_numberOfChannels, i, j,
        bytespersample = x->sf_bytesPerSample,
        bigendian = x->sf_isBigEndian;
    t_sample *fp;
    if (x->sf_state == SOUNDFILE_STATE_STREAM)
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
        if ((--x->sf_count) <= 0)
        {
#ifdef DEBUG_SOUNDFILE
            pute("signal 1\n");
#endif
            pthread_cond_signal(&x->sf_condRequest);
            x->sf_count = x->sf_period;
        }
        pthread_mutex_unlock(&x->sf_mutex);
    }
    return (w+2);
}

static void writesf_start(t_writesf *x)
{
    /* start making output.  If we're in the "startup" state change
    to the "running" state. */
    if (x->sf_state == SOUNDFILE_STATE_START)
        x->sf_state = SOUNDFILE_STATE_STREAM;
    else
        post_error ("writesf: start requested with no prior 'open'");
}

static void writesf_stop(t_writesf *x)
{
        /* LATER rethink whether you need the mutex just to set a Svariable? */
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_state = SOUNDFILE_STATE_IDLE;
    x->sf_request = SOUNDFILE_REQUEST_CLOSE;
#ifdef DEBUG_SOUNDFILE
    pute("signal 2\n");
#endif
    pthread_cond_signal(&x->sf_condRequest);
    pthread_mutex_unlock(&x->sf_mutex);
}

static void writesf_open(t_writesf *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *filesym;
    t_symbol *fileExtension;
    int filetype, bytespersamp, swap, bigendian, normalize;
    long onset, nframes;
    t_float samplerate;
    if (x->sf_state != SOUNDFILE_STATE_IDLE)
    {
        writesf_stop(x);
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
    while (x->sf_request != SOUNDFILE_REQUEST_NOTHING)
    {
        pthread_cond_signal(&x->sf_condRequest);
        pthread_cond_wait(&x->sf_condAnswer, &x->sf_mutex);
    }
    x->sf_bytesPerSample = bytespersamp;
    x->sf_needToSwap = swap;
    x->sf_isBigEndian = bigendian;
    x->sf_fileName = filesym;
    x->sf_fileExtension = fileExtension;
    x->sf_fileType = filetype;
    x->sf_itemsWritten = 0;
    x->sf_request = SOUNDFILE_REQUEST_OPEN;
    x->sf_fifoTail = 0;
    x->sf_fifoHead = 0;
    x->sf_isEndOfFile = 0;
    x->sf_error = 0;
    x->sf_state = SOUNDFILE_STATE_START;
    x->sf_bytesPerSample = (bytespersamp > 2 ? bytespersamp : 2);
    if (samplerate > 0)
        x->sf_sampleRate = samplerate;
    else if (x->sf_sampleRateOfInput > 0)
        x->sf_sampleRate = x->sf_sampleRateOfInput;
    else x->sf_sampleRate = audio_getSampleRate();
        /* set fifosize from bufsize.  fifosize must be a
        multiple of the number of bytes eaten for each DSP
        tick.  */
    x->sf_fifoSize = x->sf_bufferSize - (x->sf_bufferSize %
        (x->sf_bytesPerSample * x->sf_numberOfChannels * x->sf_vectorSize));
            /* arrange for the "request" condition to be signalled 16
            times per buffer */
    x->sf_count = x->sf_period = (x->sf_fifoSize /
            (16 * x->sf_bytesPerSample * x->sf_numberOfChannels * x->sf_vectorSize));
    pthread_cond_signal(&x->sf_condRequest);
    pthread_mutex_unlock(&x->sf_mutex);
}

static void writesf_dsp(t_writesf *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vectorSize == AUDIO_DEFAULT_BLOCKSIZE);

    int i, ninlets = x->sf_numberOfChannels;
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_vectorSize = sp[0]->s_vectorSize;
    x->sf_period = (x->sf_fifoSize /
            (16 * x->sf_bytesPerSample * x->sf_numberOfChannels * x->sf_vectorSize));
    for (i = 0; i < ninlets; i++)
        x->sf_vectorsOut[i] = sp[i]->s_vector;
    x->sf_sampleRateOfInput = sp[0]->s_sampleRate;
    pthread_mutex_unlock(&x->sf_mutex);
    dsp_add(writesf_perform, 1, x);
}

static void writesf_print(t_writesf *x)
{
    post("state %d", x->sf_state);
    post("fifo head %d", x->sf_fifoHead);
    post("fifo tail %d", x->sf_fifoTail);
    post("fifo size %d", x->sf_fifoSize);
    post("fd %d", x->sf_fileDescriptor);
    post("eof %d", x->sf_isEndOfFile);
}

static void writesf_free(t_writesf *x)
{
        /* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_request = SOUNDFILE_REQUEST_QUIT;
    /* post("stopping writesf thread..."); */
    pthread_cond_signal(&x->sf_condRequest);
    while (x->sf_request != SOUNDFILE_REQUEST_NOTHING)
    {
        /* post("signalling..."); */
        pthread_cond_signal(&x->sf_condRequest);
        pthread_cond_wait(&x->sf_condAnswer, &x->sf_mutex);
    }
    pthread_mutex_unlock(&x->sf_mutex);
    if (pthread_join(x->sf_thread, &threadrtn))
        post_error ("writesf_free: join failed");
    /* post("... done."); */
    
    pthread_cond_destroy(&x->sf_condRequest);
    pthread_cond_destroy(&x->sf_condAnswer);
    pthread_mutex_destroy(&x->sf_mutex);
    PD_MEMORY_FREE(x->sf_buffer);
}

void writesf_setup(void)
{
    writesf_class = class_new(sym_writesf__tilde__, (t_newmethod)writesf_new, 
        (t_method)writesf_free, sizeof(t_writesf), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(writesf_class, (t_method)writesf_start, sym_start, 0);
    class_addMethod(writesf_class, (t_method)writesf_stop, sym_stop, 0);
    class_addMethod(writesf_class, (t_method)writesf_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(writesf_class, (t_method)writesf_open, sym_open, 
        A_GIMME, 0);
    class_addMethod(writesf_class, (t_method)writesf_print, sym_print, 0);
    CLASS_SIGNAL(writesf_class, t_writesf, sf_f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
