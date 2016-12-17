
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
    t_readsf *x = zz;
#ifdef DEBUG_SOUNDFILE
    pute("1\n");
#endif
    pthread_mutex_lock(&x->x_mutex);
    while (1)
    {
        int fd, fifohead;
        char *buf;
#ifdef DEBUG_SOUNDFILE
        pute("0\n");
#endif
        if (x->x_requestcode == REQUEST_NOTHING)
        {
#ifdef DEBUG_SOUNDFILE
            pute("wait 2\n");
#endif
            sfread_cond_signal(&x->x_answercondition);
            sfread_cond_wait(&x->x_requestcondition, &x->x_mutex);
#ifdef DEBUG_SOUNDFILE
            pute("3\n");
#endif
        }
        else if (x->x_requestcode == REQUEST_OPEN)
        {
            char boo[80];
            int sysrtn, wantbytes;
            
                /* copy file stuff out of the data structure so we can
                relinquish the mutex while we're in open_soundfile(). */
            long onsetframes = x->x_onsetframes;
            long bytelimit = PD_INT_MAX;
            int skipheaderbytes = x->x_skipheaderbytes;
            int bytespersample = x->x_bytespersample;
            int sfchannels = x->x_sfchannels;
            int bigendian = x->x_bigendian;
            char *filename = x->x_filename;
            char *dirname = canvas_getEnvironment (x->x_canvas)->ce_directory->s_name;
                /* alter the request code so that an ensuing "open" will get
                noticed. */
#ifdef DEBUG_SOUNDFILE
            pute("4\n");
#endif
            x->x_requestcode = REQUEST_BUSY;
            x->x_fileerror = 0;

                /* if there's already a file open, close it */
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
                if (x->x_requestcode != REQUEST_BUSY)
                    goto lost;
            }
                /* open the soundfile with the mutex unlocked */
            pthread_mutex_unlock(&x->x_mutex);
            fd = open_soundfile(dirname, filename,
                skipheaderbytes, &bytespersample, &bigendian,
                &sfchannels, &bytelimit, onsetframes);      
            pthread_mutex_lock(&x->x_mutex);

#ifdef DEBUG_SOUNDFILE
            pute("5\n");
#endif
                /* copy back into the instance structure. */
            x->x_bytespersample = bytespersample;
            x->x_sfchannels = sfchannels;
            x->x_bigendian = bigendian;
            x->x_fd = fd;
            x->x_bytelimit = bytelimit;
            if (fd < 0)
            {
                x->x_fileerror = errno;
                x->x_eof = 1;
#ifdef DEBUG_SOUNDFILE
                pute("open failed\n");
                pute(filename);
                pute(dirname);
#endif
                goto lost;
            }
                /* check if another request has been made; if so, field it */
            if (x->x_requestcode != REQUEST_BUSY)
                goto lost;
#ifdef DEBUG_SOUNDFILE
            pute("6\n");
#endif
            x->x_fifohead = 0;
                    /* set fifosize from bufsize.  fifosize must be a
                    multiple of the number of bytes eaten for each DSP
                    tick.  We pessimistically assume MAXVECSIZE samples
                    per tick since that could change.  There could be a
                    problem here if the vector size increases while a
                    soundfile is being played...  */
            x->x_fifosize = x->x_bufsize - (x->x_bufsize %
                (x->x_bytespersample * x->x_sfchannels * MAXVECSIZE));
                    /* arrange for the "request" condition to be signalled 16
                    times per buffer */
#ifdef DEBUG_SOUNDFILE
            sprintf(boo, "fifosize %d\n", 
                x->x_fifosize);
            pute(boo);
#endif
            x->x_sigcountdown = x->x_sigperiod =
                (x->x_fifosize /
                    (16 * x->x_bytespersample * x->x_sfchannels *
                        x->x_vecsize));
                /* in a loop, wait for the fifo to get hungry and feed it */

            while (x->x_requestcode == REQUEST_BUSY)
            {
                int fifosize = x->x_fifosize;
#ifdef DEBUG_SOUNDFILE
                pute("77\n");
#endif
                if (x->x_eof)
                    break;
                if (x->x_fifohead >= x->x_fifotail)
                {
                        /* if the head is >= the tail, we can immediately read
                        to the end of the fifo.  Unless, that is, we would
                        read all the way to the end of the buffer and the 
                        "tail" is zero; this would fill the buffer completely
                        which isn't allowed because you can't tell a completely
                        full buffer from an empty one. */
                    if (x->x_fifotail || (fifosize - x->x_fifohead > READSIZE))
                    {
                        wantbytes = fifosize - x->x_fifohead;
                        if (wantbytes > READSIZE)
                            wantbytes = READSIZE;
                        if (wantbytes > x->x_bytelimit)
                            wantbytes = x->x_bytelimit;
#ifdef DEBUG_SOUNDFILE
                        sprintf(boo, "head %d, tail %d, size %d\n", 
                            x->x_fifohead, x->x_fifotail, wantbytes);
                        pute(boo);
#endif
                    }
                    else
                    {
#ifdef DEBUG_SOUNDFILE
                        pute("wait 7a ...\n");
#endif
                        sfread_cond_signal(&x->x_answercondition);
#ifdef DEBUG_SOUNDFILE
                        pute("signalled\n");
#endif
                        sfread_cond_wait(&x->x_requestcondition,
                            &x->x_mutex);
#ifdef DEBUG_SOUNDFILE
                        pute("7a done\n");
#endif
                        continue;
                    }
                }
                else
                {
                        /* otherwise check if there are at least READSIZE
                        bytes to read.  If not, wait and loop back. */
                    wantbytes =  x->x_fifotail - x->x_fifohead - 1;
                    if (wantbytes < READSIZE)
                    {
#ifdef DEBUG_SOUNDFILE
                        pute("wait 7...\n");
#endif
                        sfread_cond_signal(&x->x_answercondition);
                        sfread_cond_wait(&x->x_requestcondition,
                            &x->x_mutex);
#ifdef DEBUG_SOUNDFILE
                        pute("7 done\n");
#endif
                        continue;
                    }
                    else wantbytes = READSIZE;
                    if (wantbytes > x->x_bytelimit)
                        wantbytes = x->x_bytelimit;
                }
#ifdef DEBUG_SOUNDFILE
                pute("8\n");
#endif
                fd = x->x_fd;
                buf = x->x_buf;
                fifohead = x->x_fifohead;
                pthread_mutex_unlock(&x->x_mutex);
                sysrtn = read(fd, buf + fifohead, wantbytes);
                pthread_mutex_lock(&x->x_mutex);
                if (x->x_requestcode != REQUEST_BUSY)
                    break;
                if (sysrtn < 0)
                {
#ifdef DEBUG_SOUNDFILE
                    pute("fileerror\n");
#endif
                    x->x_fileerror = errno;
                    break;
                }
                else if (sysrtn == 0)
                {
                    x->x_eof = 1;
                    break;
                }
                else
                {
                    x->x_fifohead += sysrtn;
                    x->x_bytelimit -= sysrtn;
                    if (x->x_fifohead == fifosize)
                        x->x_fifohead = 0;
                    if (x->x_bytelimit <= 0)
                    {
                        x->x_eof = 1;
                        break;
                    }
                }
#ifdef DEBUG_SOUNDFILE
                sprintf(boo, "after: head %d, tail %d\n", 
                    x->x_fifohead, x->x_fifotail);
                pute(boo);
#endif
                    /* signal parent in case it's waiting for data */
                sfread_cond_signal(&x->x_answercondition);
            }
        lost:

            if (x->x_requestcode == REQUEST_BUSY)
                x->x_requestcode = REQUEST_NOTHING;
                /* fell out of read loop: close file if necessary,
                set EOF and signal once more */
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
            }
            sfread_cond_signal(&x->x_answercondition);

        }
        else if (x->x_requestcode == REQUEST_CLOSE)
        {
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
            }
            if (x->x_requestcode == REQUEST_CLOSE)
                x->x_requestcode = REQUEST_NOTHING;
            sfread_cond_signal(&x->x_answercondition);
        }
        else if (x->x_requestcode == REQUEST_QUIT)
        {
            if (x->x_fd >= 0)
            {
                fd = x->x_fd;
                pthread_mutex_unlock(&x->x_mutex);
                close (fd);
                pthread_mutex_lock(&x->x_mutex);
                x->x_fd = -1;
            }
            x->x_requestcode = REQUEST_NOTHING;
            sfread_cond_signal(&x->x_answercondition);
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

static void readsf_tick(t_readsf *x);

static void *readsf_new(t_float fnchannels, t_float fbufsize)
{
    t_readsf *x;
    int nchannels = fnchannels, bufsize = fbufsize, i;
    char *buf;
    
    if (nchannels < 1)
        nchannels = 1;
    else if (nchannels > MAXSFCHANS)
        nchannels = MAXSFCHANS;
    if (bufsize <= 0) bufsize = DEFBUFPERCHAN * nchannels;
    else if (bufsize < MINBUFSIZE)
        bufsize = MINBUFSIZE;
    else if (bufsize > MAXBUFSIZE)
        bufsize = MAXBUFSIZE;
    buf = PD_MEMORY_GET(bufsize);
    if (!buf) return (0);
    
    x = (t_readsf *)pd_new(readsf_class);
    
    for (i = 0; i < nchannels; i++)
        outlet_new(&x->x_obj, &s_signal);
    x->x_noutlets = nchannels;
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    pthread_mutex_init(&x->x_mutex, 0);
    pthread_cond_init(&x->x_requestcondition, 0);
    pthread_cond_init(&x->x_answercondition, 0);
    x->x_vecsize = MAXVECSIZE;
    x->x_state = STATE_IDLE;
    x->x_clock = clock_new(x, (t_method)readsf_tick);
    x->x_canvas = canvas_getCurrent();
    x->x_bytespersample = 2;
    x->x_sfchannels = 1;
    x->x_fd = -1;
    x->x_buf = buf;
    x->x_bufsize = bufsize;
    x->x_fifosize = x->x_fifohead = x->x_fifotail = x->x_requestcode = 0;
    pthread_create(&x->x_childthread, 0, readsf_child_main, x);
    return x;
}

static void readsf_tick(t_readsf *x)
{
    outlet_bang(x->x_bangout);
}

static t_int *readsf_perform(t_int *w)
{
    t_readsf *x = (t_readsf *)(w[1]);
    int vecsize = x->x_vecsize, noutlets = x->x_noutlets, i, j,
        bytespersample = x->x_bytespersample,
        bigendian = x->x_bigendian;
    t_sample *fp;
    if (x->x_state == STATE_STREAM)
    {
        int wantbytes, nchannels, sfchannels = x->x_sfchannels;
        pthread_mutex_lock(&x->x_mutex);
        wantbytes = sfchannels * vecsize * bytespersample;
        while (
            !x->x_eof && x->x_fifohead >= x->x_fifotail &&
                x->x_fifohead < x->x_fifotail + wantbytes-1)
        {
#ifdef DEBUG_SOUNDFILE
            pute("wait...\n");
#endif
            sfread_cond_signal(&x->x_requestcondition);
            sfread_cond_wait(&x->x_answercondition, &x->x_mutex);
                /* resync local cariables -- bug fix thanks to Shahrokh */
            vecsize = x->x_vecsize;
            bytespersample = x->x_bytespersample;
            sfchannels = x->x_sfchannels;
            wantbytes = sfchannels * vecsize * bytespersample;
            bigendian = x->x_bigendian;
#ifdef DEBUG_SOUNDFILE
            pute("done\n");
#endif
        }
        if (x->x_eof && x->x_fifohead >= x->x_fifotail &&
            x->x_fifohead < x->x_fifotail + wantbytes-1)
        {
            int xfersize;
            if (x->x_fileerror)
            {
                post_error ("dsp: %s: %s", x->x_filename,
                    (x->x_fileerror == EIO ?
                        "unknown or bad header format" :
                            strerror(x->x_fileerror)));
            }
            clock_delay(x->x_clock, 0);
            x->x_state = STATE_IDLE;

                /* if there's a partial buffer left, copy it out. */
            xfersize = (x->x_fifohead - x->x_fifotail + 1) /
                (sfchannels * bytespersample);
            if (xfersize)
            {
                soundfile_xferin_sample(sfchannels, noutlets, x->x_outvec, 0,
                    (unsigned char *)(x->x_buf + x->x_fifotail), xfersize,
                        bytespersample, bigendian, 1);
                vecsize -= xfersize;
            }
                /* then zero out the (rest of the) output */
            for (i = 0; i < noutlets; i++)
                for (j = vecsize, fp = x->x_outvec[i] + xfersize; j--; )
                    *fp++ = 0;

            sfread_cond_signal(&x->x_requestcondition);
            pthread_mutex_unlock(&x->x_mutex);
            return (w+2); 
        }

        soundfile_xferin_sample(sfchannels, noutlets, x->x_outvec, 0,
            (unsigned char *)(x->x_buf + x->x_fifotail), vecsize,
                bytespersample, bigendian, 1);
        
        x->x_fifotail += wantbytes;
        if (x->x_fifotail >= x->x_fifosize)
            x->x_fifotail = 0;
        if ((--x->x_sigcountdown) <= 0)
        {
            sfread_cond_signal(&x->x_requestcondition);
            x->x_sigcountdown = x->x_sigperiod;
        }
        pthread_mutex_unlock(&x->x_mutex);
    }
    else
    {
        for (i = 0; i < noutlets; i++)
            for (j = vecsize, fp = x->x_outvec[i]; j--; )
                *fp++ = 0;
    }
    return (w+2);
}

static void readsf_start(t_readsf *x)
{
    /* start making output.  If we're in the "startup" state change
    to the "running" state. */
    if (x->x_state == STATE_STARTUP)
        x->x_state = STATE_STREAM;
    else post_error ("readsf: start requested with no prior 'open'");
}

static void readsf_stop(t_readsf *x)
{
        /* LATER rethink whether you need the mutex just to set a variable? */
    pthread_mutex_lock(&x->x_mutex);
    x->x_state = STATE_IDLE;
    x->x_requestcode = REQUEST_CLOSE;
    sfread_cond_signal(&x->x_requestcondition);
    pthread_mutex_unlock(&x->x_mutex);
}

static void readsf_float(t_readsf *x, t_float f)
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

static void readsf_open(t_readsf *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *filesym = atom_getSymbolAtIndex(0, argc, argv);
    t_float onsetframes = atom_getFloatAtIndex(1, argc, argv);
    t_float headerbytes = atom_getFloatAtIndex(2, argc, argv);
    t_float channels = atom_getFloatAtIndex(3, argc, argv);
    t_float bytespersamp = atom_getFloatAtIndex(4, argc, argv);
    t_symbol *endian = atom_getSymbolAtIndex(5, argc, argv);
    if (!*filesym->s_name)
        return;
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = REQUEST_OPEN;
    x->x_filename = filesym->s_name;
    x->x_fifotail = 0;
    x->x_fifohead = 0;
    if (*endian->s_name == 'b')
         x->x_bigendian = 1;
    else if (*endian->s_name == 'l')
         x->x_bigendian = 0;
    else if (*endian->s_name)
        post_error ("endianness neither 'b' nor 'l'");
    else x->x_bigendian = garray_ambigendian();
    x->x_onsetframes = (onsetframes > 0 ? onsetframes : 0);
    x->x_skipheaderbytes = (headerbytes > 0 ? headerbytes : 
        (headerbytes == 0 ? -1 : 0));
    x->x_sfchannels = (channels >= 1 ? channels : 1);
    x->x_bytespersample = (bytespersamp > 2 ? bytespersamp : 2);
    x->x_eof = 0;
    x->x_fileerror = 0;
    x->x_state = STATE_STARTUP;
    sfread_cond_signal(&x->x_requestcondition);
    pthread_mutex_unlock(&x->x_mutex);
}

static void readsf_dsp(t_readsf *x, t_signal **sp)
{
    int i, noutlets = x->x_noutlets;
    pthread_mutex_lock(&x->x_mutex);
    x->x_vecsize = sp[0]->s_vectorSize;
    
    x->x_sigperiod = (x->x_fifosize /
        (x->x_bytespersample * x->x_sfchannels * x->x_vecsize));
    for (i = 0; i < noutlets; i++)
        x->x_outvec[i] = sp[i]->s_vector;
    pthread_mutex_unlock(&x->x_mutex);
    dsp_add(readsf_perform, 1, x);
}

static void readsf_print(t_readsf *x)
{
    post("state %d", x->x_state);
    post("fifo head %d", x->x_fifohead);
    post("fifo tail %d", x->x_fifotail);
    post("fifo size %d", x->x_fifosize);
    post("fd %d", x->x_fd);
    post("eof %d", x->x_eof);
}

static void readsf_free(t_readsf *x)
{
        /* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->x_mutex);
    x->x_requestcode = REQUEST_QUIT;
    sfread_cond_signal(&x->x_requestcondition);
    while (x->x_requestcode != REQUEST_NOTHING)
    {
        sfread_cond_signal(&x->x_requestcondition);
        sfread_cond_wait(&x->x_answercondition, &x->x_mutex);
    }
    pthread_mutex_unlock(&x->x_mutex);
    if (pthread_join(x->x_childthread, &threadrtn))
        post_error ("readsf_free: join failed");
    
    pthread_cond_destroy(&x->x_requestcondition);
    pthread_cond_destroy(&x->x_answercondition);
    pthread_mutex_destroy(&x->x_mutex);
    PD_MEMORY_FREE(x->x_buf);
    clock_free(x->x_clock);
}

void readsf_setup(void)
{
    readsf_class = class_new(sym_readsf__tilde__, (t_newmethod)readsf_new, 
        (t_method)readsf_free, sizeof(t_readsf), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
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
