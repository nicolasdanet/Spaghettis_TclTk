
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_soundfile_h_
#define __d_soundfile_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_MAXIMUM_CHANNELS      64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_SIZE_VECTOR           128
#define SOUNDFILE_SIZE_READ             65536
#define SOUNDFILE_SIZE_WRITE            65536

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_BUFFER_MINIMUM        (SOUNDFILE_SIZE_READ * 4)
#define SOUNDFILE_BUFFER_MAXIMUM        (SOUNDFILE_SIZE_READ * 256)             /* Arbitrary. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_NOTHING               0
#define SOUNDFILE_OPEN                  1
#define SOUNDFILE_CLOSE                 2
#define SOUNDFILE_QUIT                  3
#define SOUNDFILE_BUSY                  4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_IDLE                  0
#define SOUNDFILE_START                 1
#define SOUNDFILE_STREAM                2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _readsf_tilde {
    t_object x_obj;
    t_glist *x_canvas;
    t_clock *x_clock;
    char *x_buf;                            /* soundfile buffer */
    int x_bufsize;                          /* buffer size in bytes */
    int x_noutlets;                         /* number of audio outlets */
    t_sample *(x_outvec[SOUNDFILE_MAXIMUM_CHANNELS]);       /* audio vectors */
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
    } t_readsf_tilde;

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
void soundfile_xferin_sample(int sfchannels, int nvecs, t_sample **vecs,
    long itemsread, unsigned char *buf, int nitems, int bytespersamp,
    int bigendian, int spread);
void soundfile_xferin_float(int sfchannels, int nvecs, t_float **vecs,
    long itemsread, unsigned char *buf, int nitems, int bytespersamp,
    int bigendian, int spread);
void soundfile_xferout_float(int nchannels, t_float **vecs,
    unsigned char *buf, int nitems, long onset, int bytespersamp,
    int bigendian, t_sample normalfactor, int spread);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_soundfile_h_
