
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

/* The WAVE header. */ 
/* All WAVE files are little-endian. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://soundfile.sapp.org/doc/WaveFormat/ > */
/* < http://tiny.systems/software/soundProgrammer/WavFormatDocs.pdf > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _wave {
    char        w_fileID[4];
    uint32_t    w_chunkSize;
    char        w_waveID[4];
    char        w_fmtID[4];
    uint32_t    w_fmtChunkSize;
    uint16_t    w_audioFormat;
    uint16_t    w_numberOfChannels;
    uint32_t    w_samplesPerSecond;
    uint32_t    w_bytesPerSecond;
    uint16_t    w_blockAlign;
    uint16_t    w_bitsPerSample;
    char        w_dataChunkID[4];
    uint32_t    w_dataChunkSize;
    } t_wave;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* The AIFF header. */
/* All AIFF files are big-endian. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://muratnkonar.com/aiff/index.html > */
/* < http://paulbourke.net/dataformats/audio/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _aiff {
    char            a_fileID[4];
    uint32_t        a_chunkSize;
    char            a_aiffID[4];
    char            a_commID[4];
    uint32_t        a_commChunkSize;
    uint16_t        a_numberOfChannels;
    uint32_t        a_numberOfFrames;
    uint16_t        a_bitsPerSample;
    unsigned char   a_sampleRate[10];
    char            a_dataChunkID[4];
    uint32_t        a_dataChunkSize;
    uint32_t        a_dataOffset;
    uint32_t        a_dataBlock;
    } t_aiff;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* The NeXTStep sound header. */ 
/* Can be big-endian or little-endian. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://soundfile.sapp.org/doc/NextFormat/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _nextstep {
    char        ns_magic[4];
    uint32_t    ns_dataLocation;
    uint32_t    ns_dataSize;
    uint32_t    ns_dataFormat;
    uint32_t    ns_samplingRate;
    uint32_t    ns_channelCount;
    char        ns_info[4];
    } t_nextstep;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define WAVE_FORMAT_PCM             1
#define WAVE_FORMAT_FLOAT           3
//#define WAVE_FORMAT_ALAW          6
//#define WAVE_FORMAT_MULAW         7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define NS_FORMAT_LINEAR_16         3
#define NS_FORMAT_LINEAR_24         4
#define NS_FORMAT_FLOAT             6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_CHANNELS          64
#define SOUNDFILE_UNKNOWN           PD_INT_MAX
#define SOUNDFILE_UNDEFINED         -1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_WAVE              0
#define SOUNDFILE_AIFF              1
#define SOUNDFILE_NEXT              2

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SOUNDFILE_HELPER_INIT(x)        {                           \
                                            *((x)->h_c)      = 0;   \
                                            (x)->h_bytesSet  = 0;   \
                                            (x)->h_onset     = 0;   \
                                            *((x)->h_ID)     = 0;   \
                                            (x)->h_chunkSize = 0;   \
                                        }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _headerhelper {
    unsigned char   h_c[SOUNDFILE_HELPER_SIZE];
    int             h_bytesSet;
    int             h_onset;
    char            h_ID[5];
    int             h_chunkSize;
    } t_headerhelper;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_error soundfile_helperRead (int f, t_headerhelper *t, off_t offset)
{
    t_error err = ((lseek (f, offset, SEEK_SET)) != offset);
    
    if (err) { t->h_bytesSet = 0; }
    else { 
        t->h_bytesSet = read (f, t->h_c, SOUNDFILE_HELPER_SIZE);
    }
    
    err = (t->h_bytesSet <= 0);
    
    return err;
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
