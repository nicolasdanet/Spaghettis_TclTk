
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

#define SOUNDFILE_UNDEFINED            -1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_WAVE                  0
#define SOUNDFILE_AIFF                  1
#define SOUNDFILE_NEXT                  2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_UNKNOWN               PD_INT_MAX

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
    int                 sf_numberOfFramesToSkip;
    int                 sf_maximumBytesToRead;
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
    char                *sf_fileName;
    char                *sf_fileExtension;
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

static inline uint32_t soundfile_swap4Bytes (uint32_t n, int needToSwap)
{
    if (!needToSwap) { return n; }
    else {
        return (((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
    }
}

static inline uint16_t soundfile_swap2Bytes (uint16_t n, int needToSwap)
{
    if (!needToSwap) { return n; }
    else {
        return (((n & 0xff) << 8) | ((n & 0xff00) >> 8));
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

typedef struct _audioproperties {
    t_symbol    *ap_fileName;
    t_symbol    *ap_fileExtension;
    t_float     ap_sampleRate;
    int         ap_fileType;
    int         ap_headerSize;
    int         ap_numberOfChannels;
    int         ap_bytesPerSample;
    int         ap_isBigEndian;
    int         ap_needToSwap;
    int         ap_dataSizeInBytes;
    int         ap_onset;
    int         ap_numberOfFrames;
    int         ap_needToNormalize;
    int         ap_needToResize;
    } t_audioproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void soundfile_initProperties (t_audioproperties *args)
{
    args->ap_fileName           = &s_;
    args->ap_fileExtension      = &s_;
    args->ap_sampleRate         = SOUNDFILE_UNDEFINED;
    args->ap_fileType           = SOUNDFILE_UNDEFINED;
    args->ap_headerSize         = SOUNDFILE_UNDEFINED;
    args->ap_numberOfChannels   = SOUNDFILE_UNDEFINED;
    args->ap_bytesPerSample     = SOUNDFILE_UNDEFINED;
    args->ap_isBigEndian        = SOUNDFILE_UNDEFINED;
    args->ap_needToSwap         = SOUNDFILE_UNDEFINED;
    args->ap_dataSizeInBytes    = SOUNDFILE_UNDEFINED;
    args->ap_onset              = SOUNDFILE_UNDEFINED;
    args->ap_numberOfFrames     = SOUNDFILE_UNKNOWN;
    args->ap_needToNormalize    = SOUNDFILE_UNDEFINED;
    args->ap_needToResize       = SOUNDFILE_UNDEFINED;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error soundfile_readFileParse         (t_symbol *s, int *argc, t_atom **argv, t_audioproperties *args);
int     soundfile_readFileHeader        (t_glist *glist, t_audioproperties *args);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error soundfile_writeFileParse        (t_symbol *s, int *argc, t_atom **argv, t_audioproperties *args);
int     soundfile_writeFileHeader       (t_glist *glist, t_audioproperties *args);
t_error soundfile_writeFileClose        (int f, int itemsWritten, t_audioproperties *args);
                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    soundfile_encode                (int numberOfChannels,
                                            t_sample **v,
                                            unsigned char *t,
                                            int numberOfFrames,
                                            int onset,
                                            int bytesPerSamples,
                                            int isBigEndian,
                                            int spread, 
                                            t_sample normalFactor);
                                            
void    soundfile_decode                (int numberOfChannels,
                                            t_sample **v,
                                            unsigned char *t,
                                            int numberOfFrames,
                                            int onset,
                                            int bytesPerSamples,
                                            int isBigEndian,
                                            int spread, 
                                            int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_soundfile_h_
