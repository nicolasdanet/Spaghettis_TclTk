
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

/******************************* writesf *******************/
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *writesf_class;

#define t_writesf t_readsf_tilde      /* just re-use the structure */

/************** the child thread which performs file I/O ***********/

static void *writesf_child_main(void *zz)
{
    t_writesf *x = zz;
#ifdef DEBUG_SOUNDFILE
    pute("1\n");
#endif
    pthread_mutex_lock(&x->x_mutex);
    while (1)
    {
#ifdef DEBUG_SOUNDFILE
        pute("0\n");
#endif
        if (x->x_requestcode == SOUNDFILE_NOTHING)
        {
#ifdef DEBUG_SOUNDFILE
            pute("wait 2\n");
#endif
            pthread_cond_signal(&x->x_answercondition);
            pthread_cond_wait(&x->x_requestcondition, &x->x_mutex);
#ifdef DEBUG_SOUNDFILE
            pute("3\n");
#endif
        }
        else if (x->x_requestcode == SOUNDFILE_OPEN)
        {
            char boo[80];
            int fd, sysrtn, writebytes;
            
                /* copy file stuff out of the data structure so we can
                relinquish the mutex while we're in open_soundfile(). */
            long onsetframes = x->x_onsetframes;
            long bytelimit = PD_INT_MAX;
            int skipheaderbytes = x->x_skipheaderbytes;
            int bytespersample = x->x_bytespersample;
            int sfchannels = x->x_sfchannels;
            int bigendian = x->x_bigendian;
            int filetype = x->x_filetype;
            char *filename = x->x_filename;
            t_glist *canvas = x->x_canvas;
            t_float samplerate = x->x_samplerate;

                /* alter the request code so that an ensuing "open" will get
                noticed. */
#ifdef DEBUG_SOUNDFILE
            pute("4\n");
#endif
            x->x_requestcode = SOUNDFILE_BUSY;
            x->x_fileerror = 0;

                /* if there's already a file open, close it.  This
                should never happen since writesf_open() calls stop if
                needed and then waits until we're idle. */
            if (x->x_fd >= 0)
            {
                int bytesperframe = x->x_bytespersample * x->x_sfchannels;
                int bigendian = x->x_bigendian;
                char *filename = x->x_filename;
                int fd = x->x_fd;
                int filetype = x->x_filetype;
                int itemswritten = x->x_itemswritten;
                int swap = x->x_swap;
                pthread_mutex_unlock(&x->x_mutex);
                
                soundfile_finishwrite(x, filename, fd,
                    filetype, PD_INT_MAX, itemswritten,
                    bytesperframe, swap);
                close (fd);

                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
#ifdef DEBUG_SOUNDFILE
                {
                    char s[1000];
                    sprintf(s, "bug ditched %d\n", itemswritten);
                    pute(s);
                }
#endif  
                if (x->x_requestcode != SOUNDFILE_BUSY)
                    continue;
            }
                /* open the soundfile with the mutex unlocked */
            pthread_mutex_unlock(&x->x_mutex);
            fd = create_soundfile(canvas, filename, filetype, 0,
                    bytespersample, bigendian, sfchannels, 
                        garray_ambigendian() != bigendian, samplerate);
            pthread_mutex_lock(&x->x_mutex);
#ifdef DEBUG_SOUNDFILE
            pute("5\n");
#endif

            if (fd < 0)
            {
                x->x_fd = -1;
                x->x_eof = 1;
                x->x_fileerror = errno;
#ifdef DEBUG_SOUNDFILE
                pute("open failed\n");
                pute(filename);
#endif
                x->x_requestcode = SOUNDFILE_NOTHING;
                continue;
            }
            /* check if another request has been made; if so, field it */
            if (x->x_requestcode != SOUNDFILE_BUSY)
                continue;
#ifdef DEBUG_SOUNDFILE
            pute("6\n");
#endif
            x->x_fd = fd;
            x->x_fifotail = 0;
            x->x_itemswritten = 0;
            x->x_swap = garray_ambigendian() != bigendian;      
                /* in a loop, wait for the fifo to have data and write it
                    to disk */
            while (x->x_requestcode == SOUNDFILE_BUSY ||
                (x->x_requestcode == SOUNDFILE_CLOSE &&
                    x->x_fifohead != x->x_fifotail))
            {
                int fifosize = x->x_fifosize, fifotail;
                char *buf = x->x_buf;
#ifdef DEBUG_SOUNDFILE
                pute("77\n");
#endif
                    /* if the head is < the tail, we can immediately write
                    from tail to end of fifo to disk; otherwise we hold off
                    writing until there are at least SOUNDFILE_SIZE_WRITE bytes in the
                    buffer */
                if (x->x_fifohead < x->x_fifotail ||
                    x->x_fifohead >= x->x_fifotail + SOUNDFILE_SIZE_WRITE
                    || (x->x_requestcode == SOUNDFILE_CLOSE &&
                        x->x_fifohead != x->x_fifotail))
                {
                    writebytes = (x->x_fifohead < x->x_fifotail ?
                        fifosize : x->x_fifohead) - x->x_fifotail;
                    if (writebytes > SOUNDFILE_SIZE_READ)
                        writebytes = SOUNDFILE_SIZE_READ;
                }
                else
                {
#ifdef DEBUG_SOUNDFILE
                    pute("wait 7a ...\n");
#endif
                    pthread_cond_signal(&x->x_answercondition);
#ifdef DEBUG_SOUNDFILE
                    pute("signalled\n");
#endif
                    pthread_cond_wait(&x->x_requestcondition,
                        &x->x_mutex);
#ifdef DEBUG_SOUNDFILE
                    pute("7a done\n");
#endif
                    continue;
                }
#ifdef DEBUG_SOUNDFILE
                pute("8\n");
#endif
                fifotail = x->x_fifotail;
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                sysrtn = write(fd, buf + fifotail, writebytes);
                pthread_mutex_lock(&x->x_mutex);
                if (x->x_requestcode != SOUNDFILE_BUSY &&
                    x->x_requestcode != SOUNDFILE_CLOSE)
                        break;
                if (sysrtn < writebytes)
                {
#ifdef DEBUG_SOUNDFILE
                    pute("fileerror\n");
#endif
                    x->x_fileerror = errno;
                    break;
                }
                else
                {
                    x->x_fifotail += sysrtn;
                    if (x->x_fifotail == fifosize)
                        x->x_fifotail = 0;
                }
                x->x_itemswritten +=
                    sysrtn / (x->x_bytespersample * x->x_sfchannels);
#ifdef DEBUG_SOUNDFILE
                sprintf(boo, "after: head %d, tail %d written %d\n", 
                    x->x_fifohead, x->x_fifotail, x->x_itemswritten);
                pute(boo);
#endif
                    /* signal parent in case it's waiting for data */
                pthread_cond_signal(&x->x_answercondition);
            }
        }
        else if (x->x_requestcode == SOUNDFILE_CLOSE ||
            x->x_requestcode == SOUNDFILE_QUIT)
        {
            int quit = (x->x_requestcode == SOUNDFILE_QUIT);
            if (x->x_fd >= 0)
            {
                int bytesperframe = x->x_bytespersample * x->x_sfchannels;
                int bigendian = x->x_bigendian;
                char *filename = x->x_filename;
                int fd = x->x_fd;
                int filetype = x->x_filetype;
                int itemswritten = x->x_itemswritten;
                int swap = x->x_swap;
                pthread_mutex_unlock(&x->x_mutex);

                soundfile_finishwrite(x, filename, fd,
                    filetype, PD_INT_MAX, itemswritten,
                    bytesperframe, swap);
                close (fd);

                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
            }
            x->x_requestcode = SOUNDFILE_NOTHING;
            pthread_cond_signal(&x->x_answercondition);
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
    pthread_mutex_unlock(&x->x_mutex);
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
    if (bufsize <= 0) bufsize = SOUNDFILE_BUFFER_MINIMUM * nchannels;
    else if (bufsize < SOUNDFILE_BUFFER_MINIMUM)
        bufsize = SOUNDFILE_BUFFER_MINIMUM;
    else if (bufsize > SOUNDFILE_BUFFER_MAXIMUM)
        bufsize = SOUNDFILE_BUFFER_MAXIMUM;
    buf = PD_MEMORY_GET(bufsize);
    if (!buf) return (0);
    
    x = (t_writesf *)pd_new(writesf_class);
    
    for (i = 1; i < nchannels; i++)
        inlet_new(&x->x_obj,  &x->x_obj.te_g.g_pd, &s_signal, &s_signal);

    x->x_f = 0;
    x->x_sfchannels = nchannels;
    pthread_mutex_init(&x->x_mutex, 0);
    pthread_cond_init(&x->x_requestcondition, 0);
    pthread_cond_init(&x->x_answercondition, 0);
    x->x_vecsize = SOUNDFILE_SIZE_VECTOR;
    x->x_insamplerate = x->x_samplerate = 0;
    x->x_state = SOUNDFILE_IDLE;
    x->x_clock = 0;     /* no callback needed here */
    x->x_canvas = canvas_getCurrent();
    x->x_bytespersample = 2;
    x->x_fd = -1;
    x->x_buf = buf;
    x->x_bufsize = bufsize;
    x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_requestcode = 0;
    pthread_create(&x->x_childthread, 0, writesf_child_main, x);
    return x;
}

static t_int *writesf_perform(t_int *w)
{
    t_writesf *x = (t_writesf *)(w[1]);
    int vecsize = x->x_vecsize, sfchannels = x->x_sfchannels, i, j,
        bytespersample = x->x_bytespersample,
        bigendian = x->x_bigendian;
    t_sample *fp;
    if (x->x_state == SOUNDFILE_STREAM)
    {
        int wantbytes, roominfifo;
        pthread_mutex_lock(&x->x_mutex);
        wantbytes = sfchannels * vecsize * bytespersample;
        roominfifo = x->x_fifotail - x->x_fifohead;
        if (roominfifo <= 0)
            roominfifo += x->x_fifosize;
        while (roominfifo < wantbytes + 1)
        {
            fprintf(stderr, "writesf waiting for disk write..\n");
            fprintf(stderr, "(head %d, tail %d, room %d, want %d)\n",
                x->x_fifohead, x->x_fifotail, roominfifo, wantbytes);
            pthread_cond_signal(&x->x_requestcondition);
            pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
            fprintf(stderr, "... done waiting.\n");
            roominfifo = x->x_fifotail - x->x_fifohead;
            if (roominfifo <= 0)
                roominfifo += x->x_fifosize;
        }

        soundfile_xferout_sample(sfchannels, x->x_outvec,
            (unsigned char *)(x->x_buf + x->x_fifohead), vecsize, 0,
                bytespersample, bigendian, 1., 1);
        
        x->x_fifohead += wantbytes;
        if (x->x_fifohead >= x->x_fifosize)
            x->x_fifohead = 0;
        if ((--x->x_sigcountdown) <= 0)
        {
#ifdef DEBUG_SOUNDFILE
            pute("signal 1\n");
#endif
            pthread_cond_signal(&x->x_requestcondition);
            x->x_sigcountdown = x->x_sigperiod;
        }
        pthread_mutex_unlock(&x->x_mutex);
    }
    return (w+2);
}

static void writesf_start(t_writesf *x)
{
    /* start making output.  If we're in the "startup" state change
    to the "running" state. */
    if (x->x_state == SOUNDFILE_START)
        x->x_state = SOUNDFILE_STREAM;
    else
        post_error ("writesf: start requested with no prior 'open'");
}

static void writesf_stop(t_writesf *x)
{
        /* LATER rethink whether you need the mutex just to set a Svariable? */
    pthread_mutex_lock(&x->x_mutex);
    x->x_state = SOUNDFILE_IDLE;
    x->x_requestcode = SOUNDFILE_CLOSE;
#ifdef DEBUG_SOUNDFILE
    pute("signal 2\n");
#endif
    pthread_cond_signal(&x->x_requestcondition);
    pthread_mutex_unlock(&x->x_mutex);
}


    /* open method.  Called as: open [args] filename with args as in
        soundfiler_writeargparse().
    */

static void writesf_open(t_writesf *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *filesym;
    int filetype, bytespersamp, swap, bigendian, normalize;
    long onset, nframes;
    t_float samplerate;
    if (x->x_state != SOUNDFILE_IDLE)
    {
        writesf_stop(x);
    }
    if (soundfiler_writeargparse(x, &argc,
        &argv, &filesym, &filetype, &bytespersamp, &swap, &bigendian,
        &normalize, &onset, &nframes, &samplerate))
    {
        post_error ("writesf~: usage: open [-bytes [234]] [-wave,-nextstep,-aiff] ...");
        post("... [-big,-little] [-rate ####] filename");
        return;
    }
    if (normalize || onset || (nframes != PD_INT_MAX))
        post_error ("normalize/onset/nframes argument to writesf~: ignored");
    if (argc)
        post_error ("extra argument(s) to writesf~: ignored");
    pthread_mutex_lock(&x->x_mutex);
    while (x->x_requestcode != SOUNDFILE_NOTHING)
    {
        pthread_cond_signal(&x->x_requestcondition);
        pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    x->x_bytespersample = bytespersamp;
    x->x_swap = swap;
    x->x_bigendian = bigendian;
    x->x_filename = filesym->s_name;
    x->x_filetype = filetype;
    x->x_itemswritten = 0;
    x->x_requestcode = SOUNDFILE_OPEN;
    x->x_fifotail = 0;
    x->x_fifohead = 0;
    x->x_eof = 0;
    x->x_fileerror = 0;
    x->x_state = SOUNDFILE_START;
    x->x_bytespersample = (bytespersamp > 2 ? bytespersamp : 2);
    if (samplerate > 0)
        x->x_samplerate = samplerate;
    else if (x->x_insamplerate > 0)
        x->x_samplerate = x->x_insamplerate;
    else x->x_samplerate = audio_getSampleRate();
        /* set fifosize from bufsize.  fifosize must be a
        multiple of the number of bytes eaten for each DSP
        tick.  */
    x->x_fifosize = x->x_bufsize - (x->x_bufsize %
        (x->x_bytespersample * x->x_sfchannels * SOUNDFILE_SIZE_VECTOR));
            /* arrange for the "request" condition to be signalled 16
            times per buffer */
    x->x_sigcountdown = x->x_sigperiod = (x->x_fifosize /
            (16 * x->x_bytespersample * x->x_sfchannels * x->x_vecsize));
    pthread_cond_signal(&x->x_requestcondition);
    pthread_mutex_unlock(&x->x_mutex);
}

static void writesf_dsp(t_writesf *x, t_signal **sp)
{
    int i, ninlets = x->x_sfchannels;
    pthread_mutex_lock(&x->x_mutex);
    x->x_vecsize = sp[0]->s_vectorSize;
    x->x_sigperiod = (x->x_fifosize /
            (16 * x->x_bytespersample * x->x_sfchannels * x->x_vecsize));
    for (i = 0; i < ninlets; i++)
        x->x_outvec[i] = sp[i]->s_vector;
    x->x_insamplerate = sp[0]->s_sampleRate;
    pthread_mutex_unlock(&x->x_mutex);
    dsp_add(writesf_perform, 1, x);
}

static void writesf_print(t_writesf *x)
{
    post("state %d", x->x_state);
    post("fifo head %d", x->x_fifohead);
    post("fifo tail %d", x->x_fifotail);
    post("fifo size %d", x->x_fifosize);
    post("fd %d", x->x_fd);
    post("eof %d", x->x_eof);
}

static void writesf_free(t_writesf *x)
{
        /* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = SOUNDFILE_QUIT;
    /* post("stopping writesf thread..."); */
    pthread_cond_signal(&x->x_requestcondition);
    while (x->x_requestcode != SOUNDFILE_NOTHING)
    {
        /* post("signalling..."); */
        pthread_cond_signal(&x->x_requestcondition);
        pthread_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    pthread_mutex_unlock(&x->x_mutex);
    if (pthread_join(x->x_childthread, &threadrtn))
        post_error ("writesf_free: join failed");
    /* post("... done."); */
    
    pthread_cond_destroy(&x->x_requestcondition);
    pthread_cond_destroy(&x->x_answercondition);
    pthread_mutex_destroy(&x->x_mutex);
    PD_MEMORY_FREE(x->x_buf);
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
    CLASS_SIGNAL(writesf_class, t_writesf, x_f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
