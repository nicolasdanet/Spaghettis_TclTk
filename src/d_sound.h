
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_sound_h_
#define __d_sound_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MAXSFCHANS 64

#define MAXBYTESPERSAMPLE 4
#define MAXVECSIZE 128

#define READSIZE 65536
#define WRITESIZE 65536
#define DEFBUFPERCHAN 262144
#define MINBUFSIZE (4 * READSIZE)
#define MAXBUFSIZE 16777216     /* arbitrary; just don't want to hang malloc */

#define REQUEST_NOTHING 0
#define REQUEST_OPEN 1
#define REQUEST_CLOSE 2
#define REQUEST_QUIT 3
#define REQUEST_BUSY 4

#define STATE_IDLE 0
#define STATE_STARTUP 1
#define STATE_STREAM 2
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define sfread_cond_wait pthread_cond_wait
#define sfread_cond_signal pthread_cond_signal

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _readsf
{
    t_object x_obj;
    t_glist *x_canvas;
    t_clock *x_clock;
    char *x_buf;                            /* soundfile buffer */
    int x_bufsize;                          /* buffer size in bytes */
    int x_noutlets;                         /* number of audio outlets */
    t_sample *(x_outvec[MAXSFCHANS]);       /* audio vectors */
    int x_vecsize;                          /* vector size for transfers */
    t_outlet *x_bangout;                    /* bang-on-done outlet */
    int x_state;                            /* opened, running, or idle */
    t_float x_insamplerate;   /* sample rate of input signal if known */
        /* parameters to communicate with subthread */
    int x_requestcode;      /* pending request from parent to I/O thread */
    char *x_filename;       /* file to open (string is permanently allocated) */
    int x_fileerror;        /* slot for "errno" return */
    int x_skipheaderbytes;  /* size of header we'll skip */
    int x_bytespersample;   /* bytes per sample (2 or 3) */
    int x_bigendian;        /* true if file is big-endian */
    int x_sfchannels;       /* number of channels in soundfile */
    t_float x_samplerate;     /* sample rate of soundfile */
    long x_onsetframes;     /* number of sample frames to skip */
    long x_bytelimit;       /* max number of data bytes to read */
    int x_fd;               /* filedesc */
    int x_fifosize;         /* buffer size appropriately rounded down */            
    int x_fifohead;         /* index of next byte to get from file */
    int x_fifotail;         /* index of next byte the ugen will read */
    int x_eof;              /* true if fifohead has stopped changing */
    int x_sigcountdown;     /* counter for signalling child for more data */
    int x_sigperiod;        /* number of ticks per signal */
    int x_filetype;         /* writesf~ only; type of file to create */
    int x_itemswritten;     /* writesf~ only; items writen */
    int x_swap;             /* writesf~ only; true if byte swapping */
    t_float x_f;              /* writesf~ only; scalar for signal inlet */
    pthread_mutex_t x_mutex;
    pthread_cond_t x_requestcondition;
    pthread_cond_t x_answercondition;
    pthread_t x_childthread;
} t_readsf;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int garray_ambigendian(void);
int soundfiler_writeargparse(void *obj, int *p_argc, t_atom **p_argv,
    t_symbol **p_filesym,
    int *p_filetype, int *p_bytespersamp, int *p_swap, int *p_bigendian,
    int *p_normalize, long *p_onset, long *p_nframes, t_float *p_rate);
void soundfile_finishwrite(void *obj, char *filename, int fd,
    int filetype, long nframes, long itemswritten, int bytesperframe, int swap);
void soundfile_xferout_sample(int nchannels, t_sample **vecs,
    unsigned char *buf, int nitems, long onset, int bytespersamp,
    int bigendian, t_sample normalfactor, int spread);
int create_soundfile(t_glist *canvas, const char *filename,
    int filetype, int nframes, int bytespersamp,
    int bigendian, int nchannels, int swap, t_float samplerate);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_sound_h_
