
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

#define SOUNDFILE_BUFFER_SIZE           65536

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_REQUEST_NOTHING       0
#define SOUNDFILE_REQUEST_OPEN          1
#define SOUNDFILE_REQUEST_CLOSE         2
#define SOUNDFILE_REQUEST_QUIT          3
#define SOUNDFILE_REQUEST_BUSY          4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_STATE_IDLE            0
#define SOUNDFILE_STATE_START           1
#define SOUNDFILE_STATE_STREAM          2

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

/* < https://en.wikipedia.org/wiki/Extended_precision#x86_extended_precision_format > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* First, representation of 44100.0 as a IEEE double precision. */
/* Second, the result (unsigned integer) of frexp multiply by (2 ^ 32). */

// 0 10000001110 0101 10001000 10000000 00000000 00000000 00000000 00000000
//              10101 10001000 10000000 00000000 000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline void soundfile_makeAiff80BitFloat (double sampleRate, unsigned char *s)
{
    int e;
    unsigned int m = (unsigned int)ldexp (frexp (sampleRate, &e), 32);
    
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

static inline void soundfile_setPropertiesByCopy (t_audioproperties *args, t_audioproperties *from)
{
    args->ap_fileName           = from->ap_fileName;
    args->ap_fileExtension      = from->ap_fileExtension;
    args->ap_sampleRate         = from->ap_sampleRate;
    args->ap_fileType           = from->ap_fileType;
    args->ap_headerSize         = from->ap_headerSize;
    args->ap_numberOfChannels   = from->ap_numberOfChannels;
    args->ap_bytesPerSample     = from->ap_bytesPerSample;
    args->ap_isBigEndian        = from->ap_isBigEndian;
    args->ap_needToSwap         = from->ap_needToSwap;
    args->ap_dataSizeInBytes    = from->ap_dataSizeInBytes;
    args->ap_onset              = from->ap_onset;
    args->ap_numberOfFrames     = from->ap_numberOfFrames;
    args->ap_needToNormalize    = from->ap_needToNormalize;
    args->ap_needToResize       = from->ap_needToResize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_HELPER_SIZE           PD_STRING

#define SOUNDFILE_HELPER_INIT(x)        { *((x)->h_c) = 0; (x)->h_bytesSet = 0; }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _headerhelper {
    unsigned char   h_c[SOUNDFILE_HELPER_SIZE];
    int             h_bytesSet;
    } t_headerhelper;

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
