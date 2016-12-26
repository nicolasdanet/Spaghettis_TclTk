
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

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



// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/************************* readsf object ******************************/

/* READSF uses the Posix threads package; for the moment we're Linux
only although this should be portable to the other platforms.

Each instance of readsf~ owns a "child" thread for doing the unix (MSW?) file
reading.  The parent thread signals the child each time:
    (1) a file wants opening or closing;
    (2) we've eaten another 1/16 of the shared buffer (so that the
        child thread should check if it's time to read some more.)
The child signals the parent whenever a read has completed.  Signalling
is done by setting "conditions" and putting data in mutex-controlled common
areas.
*/



static t_class *readsf_class;




/************** the child thread which performs file I/O ***********/

#if 0
static void pute(char *s)   /* debug routine */
{
    write(2, s, strlen(s));
}
#define DEBUG_SOUNDFILE
#endif

static void *readsf_child_main(void *zz)
{
    t_readsf_tilde *x = zz;
#ifdef DEBUG_SOUNDFILE
    pute("1\n");
#endif
    pthread_mutex_lock(&x->sf_mutex);
    while (1)
    {
        int fd, fifohead;
        char *buf;
#ifdef DEBUG_SOUNDFILE
        pute("0\n");
#endif
        if (x->sf_request == SOUNDFILE_NOTHING)
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
        else if (x->sf_request == SOUNDFILE_OPEN)
        {
            char boo[80];
            int sysrtn, wantbytes;
            
                /* copy file stuff out of the data structure so we can
                relinquish the mutex while we're in open_soundfile(). */
            long onsetframes = x->sf_numberOfFramesToSkip;
            long bytelimit = PD_INT_MAX;
            int skipheaderbytes = x->sf_headerSize;
            int bytespersample = x->sf_bytesPerSample;
            int sfchannels = x->sf_numberOfChannels;
            int bigendian = x->sf_isFileBigEndian;
            char *filename = x->sf_fileName;
            // char *dirname = canvas_getEnvironment (x->sf_owner)->ce_directory->s_name;
                /* alter the request code so that an ensuing "open" will get
                noticed. */
#ifdef DEBUG_SOUNDFILE
            pute("4\n");
#endif
            x->sf_request = SOUNDFILE_BUSY;
            x->sf_error = 0;

                /* if there's already a file open, close it */
            if (x->sf_fileDescriptor >= 0)
            {
                fd = x->sf_fileDescriptor;
                pthread_mutex_unlock(&x->sf_mutex);
                close (fd);
                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;
                if (x->sf_request != SOUNDFILE_BUSY)
                    goto lost;
            }
                /* open the soundfile with the mutex unlocked */
            pthread_mutex_unlock(&x->sf_mutex);
            
            t_audioproperties args;
            args.ap_headerSize = skipheaderbytes;
            args.ap_isBigEndian = bigendian;
            args.ap_bytesPerSample = bytespersample;
            args.ap_numberOfChannels = sfchannels;
            args.ap_dataSizeInBytes = bytelimit;
            args.ap_onset = onsetframes;
            
            fd = soundfile_readFile (x->sf_owner, filename, &args);
                
            skipheaderbytes = args.ap_headerSize;
            bigendian = args.ap_isBigEndian;
            bytespersample = args.ap_bytesPerSample;
            sfchannels = args.ap_numberOfChannels;
            bytelimit = args.ap_dataSizeInBytes;
         
            pthread_mutex_lock(&x->sf_mutex);

#ifdef DEBUG_SOUNDFILE
            pute("5\n");
#endif
                /* copy back into the instance structure. */
            x->sf_bytesPerSample = bytespersample;
            x->sf_numberOfChannels = sfchannels;
            x->sf_isFileBigEndian = bigendian;
            x->sf_fileDescriptor = fd;
            x->sf_maximumBytesToRead = bytelimit;
            if (fd < 0)
            {
                x->sf_error = errno;
                x->sf_isEndOfFile = 1;
#ifdef DEBUG_SOUNDFILE
                pute("open failed\n");
                pute(filename);
                pute(dirname);
#endif
                goto lost;
            }
                /* check if another request has been made; if so, field it */
            if (x->sf_request != SOUNDFILE_BUSY)
                goto lost;
#ifdef DEBUG_SOUNDFILE
            pute("6\n");
#endif
            x->sf_fifoHead = 0;
                    /* set fifosize from bufsize.  fifosize must be a
                    multiple of the number of bytes eaten for each DSP
                    tick.  We pessimistically assume SOUNDFILE_SIZE_VECTOR samples
                    per tick since that could change.  There could be a
                    problem here if the vector size increases while a
                    soundfile is being played...  */
            x->sf_fifoSize = x->sf_bufferSize - (x->sf_bufferSize %
                (x->sf_bytesPerSample * x->sf_numberOfChannels * SOUNDFILE_SIZE_VECTOR));
                    /* arrange for the "request" condition to be signalled 16
                    times per buffer */
#ifdef DEBUG_SOUNDFILE
            sprintf(boo, "fifosize %d\n", 
                x->sf_fifoSize);
            pute(boo);
#endif
            x->sf_count = x->sf_period =
                (x->sf_fifoSize /
                    (16 * x->sf_bytesPerSample * x->sf_numberOfChannels *
                        x->sf_vectorSize));
                /* in a loop, wait for the fifo to get hungry and feed it */

            while (x->sf_request == SOUNDFILE_BUSY)
            {
                int fifosize = x->sf_fifoSize;
#ifdef DEBUG_SOUNDFILE
                pute("77\n");
#endif
                if (x->sf_isEndOfFile)
                    break;
                if (x->sf_fifoHead >= x->sf_fifoTail)
                {
                        /* if the head is >= the tail, we can immediately read
                        to the end of the fifo.  Unless, that is, we would
                        read all the way to the end of the buffer and the 
                        "tail" is zero; this would fill the buffer completely
                        which isn't allowed because you can't tell a completely
                        full buffer from an empty one. */
                    if (x->sf_fifoTail || (fifosize - x->sf_fifoHead > SOUNDFILE_SIZE_READ))
                    {
                        wantbytes = fifosize - x->sf_fifoHead;
                        if (wantbytes > SOUNDFILE_SIZE_READ)
                            wantbytes = SOUNDFILE_SIZE_READ;
                        if (wantbytes > x->sf_maximumBytesToRead)
                            wantbytes = x->sf_maximumBytesToRead;
#ifdef DEBUG_SOUNDFILE
                        sprintf(boo, "head %d, tail %d, size %d\n", 
                            x->sf_fifoHead, x->sf_fifoTail, wantbytes);
                        pute(boo);
#endif
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
                }
                else
                {
                        /* otherwise check if there are at least SOUNDFILE_SIZE_READ
                        bytes to read.  If not, wait and loop back. */
                    wantbytes =  x->sf_fifoTail - x->sf_fifoHead - 1;
                    if (wantbytes < SOUNDFILE_SIZE_READ)
                    {
#ifdef DEBUG_SOUNDFILE
                        pute("wait 7...\n");
#endif
                        pthread_cond_signal(&x->sf_condAnswer);
                        pthread_cond_wait(&x->sf_condRequest,
                            &x->sf_mutex);
#ifdef DEBUG_SOUNDFILE
                        pute("7 done\n");
#endif
                        continue;
                    }
                    else wantbytes = SOUNDFILE_SIZE_READ;
                    if (wantbytes > x->sf_maximumBytesToRead)
                        wantbytes = x->sf_maximumBytesToRead;
                }
#ifdef DEBUG_SOUNDFILE
                pute("8\n");
#endif
                fd = x->sf_fileDescriptor;
                buf = x->sf_buffer;
                fifohead = x->sf_fifoHead;
                pthread_mutex_unlock(&x->sf_mutex);
                sysrtn = read(fd, buf + fifohead, wantbytes);
                pthread_mutex_lock(&x->sf_mutex);
                if (x->sf_request != SOUNDFILE_BUSY)
                    break;
                if (sysrtn < 0)
                {
#ifdef DEBUG_SOUNDFILE
                    pute("fileerror\n");
#endif
                    x->sf_error = errno;
                    break;
                }
                else if (sysrtn == 0)
                {
                    x->sf_isEndOfFile = 1;
                    break;
                }
                else
                {
                    x->sf_fifoHead += sysrtn;
                    x->sf_maximumBytesToRead -= sysrtn;
                    if (x->sf_fifoHead == fifosize)
                        x->sf_fifoHead = 0;
                    if (x->sf_maximumBytesToRead <= 0)
                    {
                        x->sf_isEndOfFile = 1;
                        break;
                    }
                }
#ifdef DEBUG_SOUNDFILE
                sprintf(boo, "after: head %d, tail %d\n", 
                    x->sf_fifoHead, x->sf_fifoTail);
                pute(boo);
#endif
                    /* signal parent in case it's waiting for data */
                pthread_cond_signal(&x->sf_condAnswer);
            }
        lost:

            if (x->sf_request == SOUNDFILE_BUSY)
                x->sf_request = SOUNDFILE_NOTHING;
                /* fell out of read loop: close file if necessary,
                set EOF and signal once more */
            if (x->sf_fileDescriptor >= 0)
            {
                fd = x->sf_fileDescriptor;
                pthread_mutex_unlock(&x->sf_mutex);
                close (fd);
                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;
            }
            pthread_cond_signal(&x->sf_condAnswer);

        }
        else if (x->sf_request == SOUNDFILE_CLOSE)
        {
            if (x->sf_fileDescriptor >= 0)
            {
                fd = x->sf_fileDescriptor;
                pthread_mutex_unlock(&x->sf_mutex);
                close (fd);
                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;
            }
            if (x->sf_request == SOUNDFILE_CLOSE)
                x->sf_request = SOUNDFILE_NOTHING;
            pthread_cond_signal(&x->sf_condAnswer);
        }
        else if (x->sf_request == SOUNDFILE_QUIT)
        {
            if (x->sf_fileDescriptor >= 0)
            {
                fd = x->sf_fileDescriptor;
                pthread_mutex_unlock(&x->sf_mutex);
                close (fd);
                pthread_mutex_lock(&x->sf_mutex);
                x->sf_fileDescriptor = -1;
            }
            x->sf_request = SOUNDFILE_NOTHING;
            pthread_cond_signal(&x->sf_condAnswer);
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

static void readsf_tick(t_readsf_tilde *x);

static void *readsf_new(t_float fnchannels, t_float fbufsize)
{
    t_readsf_tilde *x;
    int nchannels = fnchannels, bufsize = fbufsize, i;
    char *buf;
    
    if (nchannels < 1)
        nchannels = 1;
    else if (nchannels > SOUNDFILE_MAXIMUM_CHANNELS)
        nchannels = SOUNDFILE_MAXIMUM_CHANNELS;
    if (bufsize <= 0) bufsize = SOUNDFILE_BUFFER_MINIMUM * nchannels;
    else if (bufsize < SOUNDFILE_BUFFER_MINIMUM)
        bufsize = SOUNDFILE_BUFFER_MINIMUM;
    else if (bufsize > SOUNDFILE_BUFFER_MAXIMUM)
        bufsize = SOUNDFILE_BUFFER_MAXIMUM;
    buf = PD_MEMORY_GET(bufsize);
    if (!buf) return (0);
    
    x = (t_readsf_tilde *)pd_new(readsf_class);
    
    for (i = 0; i < nchannels; i++)
        outlet_new (cast_object (x), &s_signal);
    x->sf_numberOfAudioOutlets = nchannels;
    x->sf_outlet = outlet_new (cast_object (x), &s_bang);
    pthread_mutex_init(&x->sf_mutex, 0);
    pthread_cond_init(&x->sf_condRequest, 0);
    pthread_cond_init(&x->sf_condAnswer, 0);
    x->sf_vectorSize = SOUNDFILE_SIZE_VECTOR;
    x->sf_state = SOUNDFILE_IDLE;
    x->sf_clock = clock_new(x, (t_method)readsf_tick);
    x->sf_owner = canvas_getCurrent();
    x->sf_bytesPerSample = 2;
    x->sf_numberOfChannels = 1;
    x->sf_fileDescriptor = -1;
    x->sf_buffer = buf;
    x->sf_bufferSize = bufsize;
    x->sf_fifoSize = x->sf_fifoHead = x->sf_fifoTail = x->sf_request = 0;
    pthread_create(&x->sf_thread, 0, readsf_child_main, x);
    return x;
}

static void readsf_tick(t_readsf_tilde *x)
{
    outlet_bang(x->sf_outlet);
}

static t_int *readsf_perform(t_int *w)
{
    t_readsf_tilde *x = (t_readsf_tilde *)(w[1]);
    int vecsize = x->sf_vectorSize, noutlets = x->sf_numberOfAudioOutlets, i, j,
        bytespersample = x->sf_bytesPerSample,
        bigendian = x->sf_isFileBigEndian;
    t_sample *fp;
    if (x->sf_state == SOUNDFILE_STREAM)
    {
        int wantbytes, nchannels, sfchannels = x->sf_numberOfChannels;
        pthread_mutex_lock(&x->sf_mutex);
        wantbytes = sfchannels * vecsize * bytespersample;
        while (
            !x->sf_isEndOfFile && x->sf_fifoHead >= x->sf_fifoTail &&
                x->sf_fifoHead < x->sf_fifoTail + wantbytes-1)
        {
#ifdef DEBUG_SOUNDFILE
            pute("wait...\n");
#endif
            pthread_cond_signal(&x->sf_condRequest);
            pthread_cond_wait(&x->sf_condAnswer, &x->sf_mutex);
                /* resync local cariables -- bug fix thanks to Shahrokh */
            vecsize = x->sf_vectorSize;
            bytespersample = x->sf_bytesPerSample;
            sfchannels = x->sf_numberOfChannels;
            wantbytes = sfchannels * vecsize * bytespersample;
            bigendian = x->sf_isFileBigEndian;
#ifdef DEBUG_SOUNDFILE
            pute("done\n");
#endif
        }
        if (x->sf_isEndOfFile && x->sf_fifoHead >= x->sf_fifoTail &&
            x->sf_fifoHead < x->sf_fifoTail + wantbytes-1)
        {
            int xfersize;
            if (x->sf_error)
            {
                post_error ("dsp: %s: %s", x->sf_fileName,
                    (x->sf_error == EIO ?
                        "unknown or bad header format" :
                            strerror(x->sf_error)));
            }
            clock_delay(x->sf_clock, 0);
            x->sf_state = SOUNDFILE_IDLE;

                /* if there's a partial buffer left, copy it out. */
            xfersize = (x->sf_fifoHead - x->sf_fifoTail + 1) /
                (sfchannels * bytespersample);
            if (xfersize)
            {
                soundfile_decode(sfchannels, x->sf_vectorsOut,
                    (unsigned char *)(x->sf_buffer + x->sf_fifoTail), xfersize, 0,
                        bytespersample, bigendian, 1, noutlets);
                vecsize -= xfersize;
            }
                /* then zero out the (rest of the) output */
            for (i = 0; i < noutlets; i++)
                for (j = vecsize, fp = x->sf_vectorsOut[i] + xfersize; j--; )
                    *fp++ = 0;

            pthread_cond_signal(&x->sf_condRequest);
            pthread_mutex_unlock(&x->sf_mutex);
            return (w+2); 
        }

        soundfile_decode(sfchannels, x->sf_vectorsOut,
            (unsigned char *)(x->sf_buffer + x->sf_fifoTail), vecsize, 0,
                bytespersample, bigendian, 1, noutlets);
        
        x->sf_fifoTail += wantbytes;
        if (x->sf_fifoTail >= x->sf_fifoSize)
            x->sf_fifoTail = 0;
        if ((--x->sf_count) <= 0)
        {
            pthread_cond_signal(&x->sf_condRequest);
            x->sf_count = x->sf_period;
        }
        pthread_mutex_unlock(&x->sf_mutex);
    }
    else
    {
        for (i = 0; i < noutlets; i++)
            for (j = vecsize, fp = x->sf_vectorsOut[i]; j--; )
                *fp++ = 0;
    }
    return (w+2);
}

static void readsf_start(t_readsf_tilde *x)
{
    /* start making output.  If we're in the "startup" state change
    to the "running" state. */
    if (x->sf_state == SOUNDFILE_START)
        x->sf_state = SOUNDFILE_STREAM;
    else post_error ("readsf: start requested with no prior 'open'");
}

static void readsf_stop(t_readsf_tilde *x)
{
        /* LATER rethink whether you need the mutex just to set a variable? */
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_state = SOUNDFILE_IDLE;
    x->sf_request = SOUNDFILE_CLOSE;
    pthread_cond_signal(&x->sf_condRequest);
    pthread_mutex_unlock(&x->sf_mutex);
}

static void readsf_float(t_readsf_tilde *x, t_float f)
{
    if (f != 0)
        readsf_start(x);
    else readsf_stop(x);
}

    /* open method.  Called as:
    open filename [skipframes headersize channels bytespersamp endianness]
        (if headersize is zero, header is taken to be automatically
        detected; thus, use the special "-1" to mean a truly headerless file.)
    */

static void readsf_open(t_readsf_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *filesym = atom_getSymbolAtIndex(0, argc, argv);
    t_float onsetframes = atom_getFloatAtIndex(1, argc, argv);
    t_float headerbytes = atom_getFloatAtIndex(2, argc, argv);
    t_float channels = atom_getFloatAtIndex(3, argc, argv);
    t_float bytespersamp = atom_getFloatAtIndex(4, argc, argv);
    t_symbol *endian = atom_getSymbolAtIndex(5, argc, argv);
    if (!*filesym->s_name)
        return;
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_request = SOUNDFILE_OPEN;
    x->sf_fileName = filesym->s_name;
    x->sf_fifoTail = 0;
    x->sf_fifoHead = 0;
    if (*endian->s_name == 'b')
         x->sf_isFileBigEndian = 1;
    else if (*endian->s_name == 'l')
         x->sf_isFileBigEndian = 0;
    else if (*endian->s_name)
        post_error ("endianness neither 'b' nor 'l'");
    else x->sf_isFileBigEndian = soundfile_systemIsBigEndian();
    x->sf_numberOfFramesToSkip = (onsetframes > 0 ? onsetframes : 0);
    x->sf_headerSize = (headerbytes > 0 ? headerbytes : 
        (headerbytes == 0 ? -1 : 0));
    x->sf_numberOfChannels = (channels >= 1 ? channels : 1);
    x->sf_bytesPerSample = (bytespersamp > 2 ? bytespersamp : 2);
    x->sf_isEndOfFile = 0;
    x->sf_error = 0;
    x->sf_state = SOUNDFILE_START;
    pthread_cond_signal(&x->sf_condRequest);
    pthread_mutex_unlock(&x->sf_mutex);
}

static void readsf_dsp(t_readsf_tilde *x, t_signal **sp)
{
    int i, noutlets = x->sf_numberOfAudioOutlets;
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_vectorSize = sp[0]->s_vectorSize;
    
    x->sf_period = (x->sf_fifoSize /
        (x->sf_bytesPerSample * x->sf_numberOfChannels * x->sf_vectorSize));
    for (i = 0; i < noutlets; i++)
        x->sf_vectorsOut[i] = sp[i]->s_vector;
    pthread_mutex_unlock(&x->sf_mutex);
    dsp_add(readsf_perform, 1, x);
}

static void readsf_print(t_readsf_tilde *x)
{
    post("state %d", x->sf_state);
    post("fifo head %d", x->sf_fifoHead);
    post("fifo tail %d", x->sf_fifoTail);
    post("fifo size %d", x->sf_fifoSize);
    post("fd %d", x->sf_fileDescriptor);
    post("eof %d", x->sf_isEndOfFile);
}

static void readsf_free(t_readsf_tilde *x)
{
        /* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->sf_mutex);
    x->sf_request = SOUNDFILE_QUIT;
    pthread_cond_signal(&x->sf_condRequest);
    while (x->sf_request != SOUNDFILE_NOTHING)
    {
        pthread_cond_signal(&x->sf_condRequest);
        pthread_cond_wait(&x->sf_condAnswer, &x->sf_mutex);
    }
    pthread_mutex_unlock(&x->sf_mutex);
    if (pthread_join(x->sf_thread, &threadrtn))
        post_error ("readsf_free: join failed");
    
    pthread_cond_destroy(&x->sf_condRequest);
    pthread_cond_destroy(&x->sf_condAnswer);
    pthread_mutex_destroy(&x->sf_mutex);
    PD_MEMORY_FREE(x->sf_buffer);
    clock_free(x->sf_clock);
}

void readsf_setup(void)
{
    readsf_class = class_new(sym_readsf__tilde__, (t_newmethod)readsf_new, 
        (t_method)readsf_free, sizeof(t_readsf_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(readsf_class, (t_method)readsf_float);
    class_addMethod(readsf_class, (t_method)readsf_start, sym_start, 0);
    class_addMethod(readsf_class, (t_method)readsf_stop, sym_stop, 0);
    class_addMethod(readsf_class, (t_method)readsf_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(readsf_class, (t_method)readsf_open, sym_open, 
        A_GIMME, 0);
    class_addMethod(readsf_class, (t_method)readsf_print, sym_print, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
