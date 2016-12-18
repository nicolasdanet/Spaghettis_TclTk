
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
    t_object            sf_obj;                 /* Must be the first. */
    t_float             sf_f;
    t_float             sf_sampleRateOfInput;
    t_float             sf_sampleRate;
    int                 sf_bufferSize;
    int                 sf_numberOfAudioOutlets;
    int                 sf_vectorSize;
    int                 sf_state;
    int                 sf_request;
    int                 sf_error;
    int                 sf_headerSize;
    int                 sf_bytesPerSample;
    int                 sf_isFileBigEndian;
    int                 sf_numberOfChannels;
    long                sf_numberOfFramesToSkip;
    long                sf_maximumBytesToRead;
    int                 sf_fileDescriptor;
    int                 sf_fifoSize;
    int                 sf_fifoHead;
    int                 sf_fifoTail;
    int                 sf_isEndOfFile;
    int                 sf_count;
    int                 sf_period;
    int                 sf_fileType;
    int                 sf_itemsWritten;
    int                 sf_needToSwapBytes;
    pthread_mutex_t     sf_mutex;
    pthread_cond_t      sf_condRequest;
    pthread_cond_t      sf_condAnswer;
    pthread_t           sf_thread;
    t_sample            *(sf_vectorsOut[SOUNDFILE_MAXIMUM_CHANNELS]);
    char                *sf_buffer;
    char                *sf_filename;
    t_glist             *sf_owner;
    t_clock             *sf_clock;
    t_outlet            *sf_outlet;
    } t_readsf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int soundfile_systemIsBigEndian (void)
{
    #if PD_LITTLE_ENDIAN
    
        return 0;
        
    #else
    
        return 1;
        
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline uint32_t soundfile_swap4BytesIfNecessary (uint32_t n, int needToSwap)
{
    if (!needToSwap) { return n; }
    else {
        return (((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
    }
}

static inline uint16_t soundfile_swap2BytesIfNecessary (uint32_t n, int needToSwap)
{
    if (!needToSwap) { return n; }
    else {
        return (((n & 0xff) << 8) | ((n & 0xff00) >> 8));
    }
}

static inline void soundfile_swapInfoIfNecessary (char *s, int needToSwap)
{
    if (needToSwap) {
    //
    char a = s[0];
    char b = s[1];
    char c = s[2];
    char d = s[3];
    
    s[0] = d;
    s[1] = c;
    s[2] = b;
    s[3] = a;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void soundfile_makeAiff80BitFloat (double sampleRate, unsigned char *s)
{
    int e;
    unsigned long m = ldexp (frexp (sampleRate, &e), 32);
    
    s[0] = (e + 16382) >> 8;
    s[1] = (e + 16382);
    s[2] = m >> 24;
    s[3] = m >> 16;
    s[4] = m >> 8;
    s[5] = m;
    s[6] = 0;
    s[7] = 0;
    s[8] = 0;
    s[9] = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int     soundfiler_writeargparse        (void *obj,
                                            int *p_argc,
                                            t_atom **p_argv,
                                            t_symbol **p_filesym,
                                            int *p_filetype,
                                            int *p_bytespersamp,
                                            int *p_swap,
                                            int *p_bigendian,
                                            int *p_normalize,
                                            long *p_onset,
                                            long *p_nframes,
                                            t_float *p_rate);

void    soundfile_finishwrite           (void *obj,
                                            char *filename,
                                            int fd,
                                            int filetype,
                                            long nframes,
                                            long itemswritten,
                                            int bytesperframe,
                                            int swap);

int     create_soundfile                (t_glist *canvas,
                                            const char *filename,
                                            int filetype,
                                            int nframes,
                                            int bytespersamp,
                                            int bigendian,
                                            int nchannels,
                                            int swap,
                                            t_float samplerate);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    soundfile_xferin_sample         (int sfchannels,
                                            int nvecs,
                                            t_sample **vecs,
                                            long itemsread,
                                            unsigned char *buf,
                                            int nitems,
                                            int bytespersamp,
                                            int bigendian,
                                            int spread);

void    soundfile_xferin_float          (int sfchannels,
                                            int nvecs,
                                            t_float **vecs,
                                            long itemsread,
                                            unsigned char *buf,
                                            int nitems,
                                            int bytespersamp,
                                            int bigendian,
                                            int spread);

void    soundfile_xferout_sample        (int nchannels,
                                            t_sample **vecs,
                                            unsigned char *buf,
                                            int nitems,
                                            long onset,
                                            int bytespersamp,
                                            int bigendian,
                                            t_sample normalfactor,
                                            int spread);

void    soundfile_xferout_float         (int nchannels,
                                            t_float **vecs,
                                            unsigned char *buf,
                                            int nitems,
                                            long onset,
                                            int bytespersamp,
                                            int bigendian,
                                            t_sample normalfactor,
                                            int spread);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_soundfile_h_
