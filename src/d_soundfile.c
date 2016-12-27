
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Basic audio files handling. */
/* Uncompressed 16-bit, 24-bit integer and 32-bit float. */
/* Mu-law, A-law formats and such NOT handled. */
/* Note that unsupported sub-chunks are not preserved at save. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://en.wikipedia.org/wiki/Mu-law > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SOUNDFILE_WAVE          0
#define SOUNDFILE_AIFF          1
#define SOUNDFILE_NEXT          2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SOUNDFILE_NONE          3

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
#pragma mark -

#define WAVE_FORMAT_PCM         1
#define WAVE_FORMAT_FLOAT       3
//#define WAVE_FORMAT_ALAW      6
//#define WAVE_FORMAT_MULAW     7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

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

#define NS_FORMAT_LINEAR_16             3
#define NS_FORMAT_LINEAR_24             4
#define NS_FORMAT_FLOAT                 6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_HEADER_WAVE           44
#define SOUNDFILE_HEADER_AIFF           54
#define SOUNDFILE_HEADER_NEXT           28

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_BUFFER                PD_STRING

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _soundfileheader {
    unsigned char   h_c[SOUNDFILE_BUFFER];
    int             h_bytesSet;
    } t_soundfileheader;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_HEADER_INIT(x)        { *((x)->h_c) = 0; (x)->h_bytesSet = 0; }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void soundfile_initialize (void)
{
    PD_ASSERT (SOUNDFILE_BUFFER > 16);
    PD_ASSERT (SOUNDFILE_BUFFER > SOUNDFILE_HEADER_WAVE);
    PD_ASSERT (SOUNDFILE_BUFFER > SOUNDFILE_HEADER_AIFF);
    PD_ASSERT (SOUNDFILE_BUFFER > SOUNDFILE_HEADER_NEXT);
    
    PD_ASSERT (sizeof (t_sample) == sizeof (t_float));          /* Required for encoding. */
    PD_ASSERT (sizeof (t_word) > sizeof (t_sample));
    PD_ASSERT (sizeof (t_word) % sizeof (t_sample) == 0);
}

void soundfile_release (void)
{

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Note that currently only the canonical file format is supported. */
/* A properly way to traverse and fetch sub-chunks should be implemented. */
/* < http://stackoverflow.com/a/19991594 > */

static t_error soundfile_readFileHeaderWAVE (int f, t_soundfileheader *t, t_audioproperties *args)
{
    t_error err = PD_ERROR;

    if (t->h_bytesSet > SOUNDFILE_HEADER_WAVE) {
    //
    if (!strncmp (t->h_c + 12, "fmt ", 4)) {
    if (!strncmp (t->h_c + 36, "data", 4)) {
    //
    int swap = args->ap_needToSwap;
    
    int fmtSize          = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 16)), swap);
    int audioFormat      = (int)soundfile_swap2Bytes (*((uint16_t *)(t->h_c + 20)), swap);
    int numberOfChannels = (int)soundfile_swap2Bytes (*((uint16_t *)(t->h_c + 22)), swap);
    int bitsPerSample    = (int)soundfile_swap2Bytes (*((uint16_t *)(t->h_c + 34)), swap);
    int dataSize         = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 40)), swap);
    
    PD_ASSERT (fmtSize == 16);
    PD_ASSERT (audioFormat == WAVE_FORMAT_PCM || audioFormat == WAVE_FORMAT_FLOAT);

    if (audioFormat == WAVE_FORMAT_PCM || audioFormat == WAVE_FORMAT_FLOAT) {
    //
    if (bitsPerSample == 16 || bitsPerSample == 24 || bitsPerSample == 32) {
    //
    args->ap_headerSize        = SOUNDFILE_HEADER_WAVE;
    args->ap_bytesPerSample    = bitsPerSample / 8;
    args->ap_numberOfChannels  = numberOfChannels;
    args->ap_dataSizeInBytes   = dataSize;
    
    err = PD_ERROR_NONE;
    //
    }
    //
    }
    //
    }
    }
    //
    }
    
    return err;
}

/* See comments above. */

static t_error soundfile_readFileHeaderAIFF (int f, t_soundfileheader *t, t_audioproperties *args)
{
    t_error err = PD_ERROR;
    
    if (t->h_bytesSet > SOUNDFILE_HEADER_AIFF) {
    //
    if (!strncmp (t->h_c + 12, "COMM", 4)) {
    if (!strncmp (t->h_c + 38, "SSND", 4)) {
    //
    int swap = args->ap_needToSwap;
    
    int numberOfChannels = (int)soundfile_swap2Bytes (*((uint16_t *)(t->h_c + 20)), swap);
    int bitsPerSample    = (int)soundfile_swap2Bytes (*((uint16_t *)(t->h_c + 26)), swap);
    int dataSize         = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 42)), swap);
    int offset           = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 46)), swap);
    int blockAlign       = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 50)), swap);
    
    PD_ASSERT (offset == 0);        /* Not tested nor implemented for now. */
    PD_ASSERT (blockAlign == 0);    /* Ditto. */

    if (bitsPerSample == 16 || bitsPerSample == 24 || bitsPerSample == 32) {
    //
    args->ap_headerSize        = SOUNDFILE_HEADER_AIFF;
    args->ap_bytesPerSample    = bitsPerSample / 8;
    args->ap_numberOfChannels  = numberOfChannels;
    args->ap_dataSizeInBytes   = dataSize;
    
    err = PD_ERROR_NONE;
    //
    }
    //
    }
    }
    //
    }
    
    return err;
}

static t_error soundfile_readFileHeaderNEXT (int f, t_soundfileheader *t, t_audioproperties *args)
{
    t_error err = PD_ERROR;
    
    if (t->h_bytesSet > SOUNDFILE_HEADER_NEXT) {
    //
    int swap = args->ap_needToSwap;
    
    int dataLocation     = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 4)), swap);
    int dataSize         = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 8)), swap);
    int audioFormat      = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 12)), swap);
    int numberOfChannels = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + 20)), swap);
    int bytesPerSample   = 0;
    
    if (audioFormat == NS_FORMAT_LINEAR_16)      { bytesPerSample = 2; }
    else if (audioFormat == NS_FORMAT_LINEAR_24) { bytesPerSample = 3; }
    else if (audioFormat == NS_FORMAT_FLOAT)     { bytesPerSample = 4; }
            
    if (bytesPerSample) {
    //
    args->ap_headerSize        = dataLocation;
    args->ap_bytesPerSample    = bytesPerSample;
    args->ap_numberOfChannels  = numberOfChannels;
    args->ap_dataSizeInBytes   = dataSize;
    
    err = PD_ERROR_NONE;
    //
    }
    //
    }
    
    return err;
}

static t_error soundfile_readFileHeaderFormat (int f, t_audioproperties *args)
{
    t_error err = PD_ERROR;
    
    t_soundfileheader t; SOUNDFILE_HEADER_INIT (&t); 
    
    t.h_bytesSet = read (f, t.h_c, SOUNDFILE_BUFFER);

    if (t.h_bytesSet >= 4) {
    //
    int format = SOUNDFILE_NONE;
    
    if (!strncmp (t.h_c,         ".snd", 4)) { format = SOUNDFILE_NEXT; args->ap_isBigEndian = 1; }
    else if (!strncmp (t.h_c,    "dns.", 4)) { format = SOUNDFILE_NEXT; args->ap_isBigEndian = 0; }
    else if (t.h_bytesSet >= 12) {
    //
    if (!strncmp (t.h_c,         "RIFF", 4)) {
        if (!strncmp (t.h_c + 8, "WAVE", 4)) { format = SOUNDFILE_WAVE; args->ap_isBigEndian = 0; }
    } else if (!strncmp (t.h_c,  "FORM", 4)) {
        if (!strncmp (t.h_c + 8, "AIFF", 4)) { format = SOUNDFILE_AIFF; args->ap_isBigEndian = 1; }
    }
    //
    }
    
    if (format != SOUNDFILE_NONE) {
    
        args->ap_needToSwap = (args->ap_isBigEndian != soundfile_systemIsBigEndian());
        
        if (format == SOUNDFILE_WAVE) { err = soundfile_readFileHeaderWAVE (f, &t, args); }
        if (format == SOUNDFILE_AIFF) { err = soundfile_readFileHeaderAIFF (f, &t, args); }
        if (format == SOUNDFILE_NEXT) { err = soundfile_readFileHeaderNEXT (f, &t, args); }
    }
    //
    }

    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int soundfile_readFileHeaderPerform (int f, t_audioproperties *args)
{
    t_error err = PD_ERROR_NONE;
    
    if (args->ap_headerSize < 0) { err = soundfile_readFileHeaderFormat (f, args); }
    
    if (!err) {
    //
    int m = args->ap_numberOfChannels * args->ap_bytesPerSample * args->ap_onset;
    off_t n = args->ap_headerSize + m;

    err = ((lseek (f, n, SEEK_SET)) != n);
        
    if (!err) {
        args->ap_dataSizeInBytes = PD_MAX (0, args->ap_dataSizeInBytes - m);
        return f;
    }
    //
    }
    
    return -1;
}

int soundfile_readFileHeader (t_glist *glist, t_audioproperties *args)
{
    char t[PD_STRING] = { 0 };
    char *s;
    
    PD_ASSERT (args->ap_fileName);
    PD_ASSERT (args->ap_fileExtension);
    
    int f = canvas_openFile (glist, 
                args->ap_fileName->s_name, 
                args->ap_fileExtension->s_name,
                t,
                &s,
                PD_STRING);
    
    if (f >= 0) { return soundfile_readFileHeaderPerform (f, args); }
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error soundfile_writeFileParse (t_symbol *s, int *ac, t_atom **av, t_audioproperties *args)
{
    t_error err = PD_ERROR_NONE;
    
    int argc                = *ac;
    t_atom *argv            = *av;
    t_symbol *fileName      = &s_;
    t_symbol *fileExtension = &s_;
    t_float sampleRate      = -1.0;
    int fileType            = SOUNDFILE_NONE;
    int bytesPerSample      = 2;
    int isBigEndian         = 0;
    int needToSwap          = 0;
    int onset               = 0;
    int numberOfFrames      = PD_INT_MAX;
    int needToNormalize     = 0;
    
    int endianness = 1;     /* Default is big-endian (used only by NeXT/Sun soundfile format). */
        
    while (argc > 0) {
    //
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
    
    if (argc > 1 && (t == sym___dash__s || t == sym___dash__skip)) {
        onset = (int)atom_getFloat (argv + 1);
        onset = PD_MAX (0, onset);
        argc -= 2; argv += 2;
        
    } else if (argc > 1 && (t == sym___dash__f || t == sym___dash__frames || t == sym___dash__nframes)) {
        numberOfFrames = (int)atom_getFloat (argv + 1);
        numberOfFrames = PD_MAX (0, numberOfFrames);
        argc -= 2; argv += 2;
        
    } else if (argc > 1 && (t == sym___dash__b || t == sym___dash__bytes)) {
        bytesPerSample = (int)atom_getFloat (argv + 1);
        bytesPerSample = PD_CLAMP (bytesPerSample, 2, 4);
        argc -= 2; argv += 2;
        
    } else if (argc > 1 && (t == sym___dash__r || t == sym___dash__rate || t == sym___dash__samplerate)) {
        sampleRate = atom_getFloat (argv + 1);
        sampleRate = PD_MAX (1.0, sampleRate);
        argc -= 2; argv += 2;
        
    } else if (t == sym___dash__n || t == sym___dash__normalize) {
        needToNormalize = 1;
        argc --; argv++;
    
    } else if (t == sym___dash__nextstep)   {
        fileType = SOUNDFILE_NEXT; fileExtension = sym___point__snd;
        argc --; argv++;
        
    } else if (t == sym___dash__wave)   {
        fileType = SOUNDFILE_WAVE; fileExtension = sym___point__wav;
        argc --; argv++;
        
    } else if (t == sym___dash__aiff)   {
        fileType = SOUNDFILE_AIFF; fileExtension = sym___point__aif;
        argc --; argv++;
        
    } else if (t == sym___dash__big)    {
        endianness = 1;
        argc --; argv++;
        
    } else if (t == sym___dash__little) {
        endianness = 0;
        argc --; argv++;
        
    } else { break; }
    //
    }
    
    if (!err) { err = (error__options (s, argc, argv) != 0); }
    if (!err) { err = (!argc || !IS_SYMBOL (argv)); }
    
    if (!err) {
    //
    fileName = GET_SYMBOL (argv); 
    
    argc--; argv++;
    
    if (fileType == SOUNDFILE_NONE) {

        if (string_endWith (fileName->s_name, ".wav"))          { fileType = SOUNDFILE_WAVE; }
        else if (string_endWith (fileName->s_name, ".WAV"))     { fileType = SOUNDFILE_WAVE; }
        else if (string_endWith (fileName->s_name, ".aif"))     { fileType = SOUNDFILE_AIFF; }
        else if (string_endWith (fileName->s_name, ".AIF"))     { fileType = SOUNDFILE_AIFF; }
        else if (string_endWith (fileName->s_name, ".aiff"))    { fileType = SOUNDFILE_AIFF; }
        else if (string_endWith (fileName->s_name, ".AIFF"))    { fileType = SOUNDFILE_AIFF; }
        else if (string_endWith (fileName->s_name, ".snd"))     { fileType = SOUNDFILE_NEXT; }
        else if (string_endWith (fileName->s_name, ".SND"))     { fileType = SOUNDFILE_NEXT; }
        else if (string_endWith (fileName->s_name, ".au"))      { fileType = SOUNDFILE_NEXT; }
        else if (string_endWith (fileName->s_name, ".AU"))      { fileType = SOUNDFILE_NEXT; }
        else {
            fileType = SOUNDFILE_WAVE; fileExtension = sym___point__wav;
        }
    }

    if (bytesPerSample == 4 && fileType == SOUNDFILE_AIFF) { PD_BUG; return PD_ERROR; }
    if (fileType == SOUNDFILE_WAVE)      { isBigEndian = 0; }
    else if (fileType == SOUNDFILE_AIFF) { isBigEndian = 1; }
    else {
        isBigEndian = endianness;
    }
    
    needToSwap = (isBigEndian != soundfile_systemIsBigEndian());
    
    args->ap_fileName           = fileName;
    args->ap_fileExtension      = fileExtension;
    args->ap_sampleRate         = sampleRate;
    args->ap_fileType           = fileType;
    args->ap_bytesPerSample     = bytesPerSample;
    args->ap_isBigEndian        = isBigEndian;
    args->ap_needToSwap         = needToSwap;
    args->ap_onset              = onset;
    args->ap_numberOfFrames     = numberOfFrames;
    args->ap_needToNormalize    = needToNormalize;
    
    *ac = argc;
    *av = argv;
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error soundfile_writeFileHeaderWAVE (t_soundfileheader *t, t_audioproperties *args)
{
    int dataSize    = args->ap_numberOfChannels * args->ap_bytesPerSample * args->ap_numberOfFrames;
    int byteRate    = args->ap_numberOfChannels * args->ap_bytesPerSample * args->ap_sampleRate;
    int blockAlign  = args->ap_numberOfChannels * args->ap_bytesPerSample;
    int audioFormat = WAVE_FORMAT_PCM;
    
    int swap = args->ap_needToSwap;
    
    t_wave h;
    
    if (args->ap_bytesPerSample == 4) { audioFormat = WAVE_FORMAT_FLOAT; }
    
    h.w_chunkSize        = soundfile_swap4Bytes ((uint32_t)(SOUNDFILE_HEADER_WAVE - 8 + dataSize), swap);
    h.w_fmtChunkSize     = soundfile_swap4Bytes ((uint32_t)(16), swap);
    h.w_audioFormat      = soundfile_swap2Bytes ((uint16_t)audioFormat, swap);
    h.w_numberOfChannels = soundfile_swap2Bytes ((uint16_t)args->ap_numberOfChannels, swap);
    h.w_samplesPerSecond = soundfile_swap4Bytes ((uint32_t)args->ap_sampleRate, swap);
    h.w_bytesPerSecond   = soundfile_swap4Bytes ((uint32_t)byteRate, swap);
    h.w_blockAlign       = soundfile_swap2Bytes ((uint16_t)blockAlign, swap);
    h.w_bitsPerSample    = soundfile_swap2Bytes ((uint16_t)(8 * args->ap_bytesPerSample), swap);
    h.w_dataChunkSize    = soundfile_swap4Bytes ((uint32_t)dataSize, swap);
    
    strncpy (t->h_c + 0,  "RIFF", 4);
    memcpy  (t->h_c + 4,  &h.w_chunkSize, 4);
    strncpy (t->h_c + 8,  "WAVE", 4);
    strncpy (t->h_c + 12, "fmt ", 4);
    memcpy  (t->h_c + 16, &h.w_fmtChunkSize, 4);
    memcpy  (t->h_c + 20, &h.w_audioFormat, 2);
    memcpy  (t->h_c + 22, &h.w_numberOfChannels, 2);
    memcpy  (t->h_c + 24, &h.w_samplesPerSecond, 4);
    memcpy  (t->h_c + 28, &h.w_bytesPerSecond, 4);
    memcpy  (t->h_c + 32, &h.w_blockAlign, 2);
    memcpy  (t->h_c + 34, &h.w_bitsPerSample, 2);
    strncpy (t->h_c + 36, "data", 4);
    memcpy  (t->h_c + 40, &h.w_dataChunkSize, 4);

    t->h_bytesSet = SOUNDFILE_HEADER_WAVE;
    
    return PD_ERROR_NONE;
}

static t_error soundfile_writeFileHeaderAIFF (t_soundfileheader *t, t_audioproperties *args)
{
    int dataSize = args->ap_numberOfChannels * args->ap_bytesPerSample * args->ap_numberOfFrames;
    
    int swap = args->ap_needToSwap;
    
    t_aiff h;
    
    h.a_chunkSize           = soundfile_swap4Bytes ((uint32_t)(SOUNDFILE_HEADER_AIFF - 8 + dataSize), swap);
    h.a_commChunkSize       = soundfile_swap4Bytes ((uint32_t)(18), swap);
    h.a_numberOfChannels    = soundfile_swap2Bytes ((uint16_t)args->ap_numberOfChannels, swap);
    h.a_numberOfFrames      = soundfile_swap4Bytes ((uint32_t)args->ap_numberOfFrames, swap);
    h.a_bitsPerSample       = soundfile_swap2Bytes ((uint16_t)(8 * args->ap_bytesPerSample), swap);
    h.a_dataChunkSize       = soundfile_swap4Bytes ((uint32_t)(dataSize + 8), swap);
    h.a_dataOffset          = (uint32_t)(0);
    h.a_dataBlock           = (uint32_t)(0);
    
    strncpy (t->h_c + 0,  "FORM", 4);
    memcpy  (t->h_c + 4,  &h.a_chunkSize, 4);
    strncpy (t->h_c + 8,  "AIFF", 4);
    strncpy (t->h_c + 12, "COMM", 4);
    memcpy  (t->h_c + 16,  &h.a_commChunkSize, 4);
    memcpy  (t->h_c + 20,  &h.a_numberOfChannels, 2);
    memcpy  (t->h_c + 22,  &h.a_numberOfFrames, 4);
    memcpy  (t->h_c + 26,  &h.a_bitsPerSample, 2);
    soundfile_makeAiff80BitFloat (args->ap_sampleRate, t->h_c + 28);
    strncpy (t->h_c + 38, "SSND", 4);
    memcpy  (t->h_c + 42,  &h.a_dataChunkSize, 4);
    memcpy  (t->h_c + 46,  &h.a_dataOffset, 4);
    memcpy  (t->h_c + 50,  &h.a_dataBlock, 4);
    
    t->h_bytesSet = SOUNDFILE_HEADER_AIFF;

    return PD_ERROR_NONE;
}

static t_error soundfile_writeFileHeaderNEXT (t_soundfileheader *t, t_audioproperties *args)
{
    int dataSize    = args->ap_numberOfChannels * args->ap_bytesPerSample * args->ap_numberOfFrames;
    int audioFormat = NS_FORMAT_LINEAR_16;
    
    int swap = args->ap_needToSwap;
    
    t_nextstep h;

    if (args->ap_bytesPerSample == 3) { audioFormat = NS_FORMAT_LINEAR_24; }
    if (args->ap_bytesPerSample == 4) { audioFormat = NS_FORMAT_FLOAT; }
    
    h.ns_dataLocation = soundfile_swap4Bytes ((uint32_t)SOUNDFILE_HEADER_NEXT, swap);
    h.ns_dataSize     = soundfile_swap4Bytes ((uint32_t)dataSize, swap);
    h.ns_dataFormat   = soundfile_swap4Bytes ((uint32_t)audioFormat, swap);
    h.ns_samplingRate = soundfile_swap4Bytes ((uint32_t)args->ap_sampleRate, swap);
    h.ns_channelCount = soundfile_swap4Bytes ((uint32_t)args->ap_numberOfChannels, swap);
    
    if (args->ap_isBigEndian) { 
        strncpy (t->h_c + 0, ".snd", 4); 
    } else {
        strncpy (t->h_c + 0, "dns.", 4);
    }
    
    memcpy (t->h_c + 4,  &h.ns_dataLocation, 4);
    memcpy (t->h_c + 8,  &h.ns_dataSize, 4);
    memcpy (t->h_c + 12, &h.ns_dataFormat, 4);
    memcpy (t->h_c + 16, &h.ns_samplingRate, 4);
    memcpy (t->h_c + 20, &h.ns_channelCount, 4);
    memset (t->h_c + 24, 0, 4);
    
    t->h_bytesSet = SOUNDFILE_HEADER_NEXT;

    return PD_ERROR_NONE;
}

int soundfile_writeFileHeader (t_glist *glist, t_audioproperties *args)
{
    t_error err = PD_ERROR_NONE;
    char name[PD_STRING] = { 0 };
    int f = -1;
    
    t_soundfileheader t; SOUNDFILE_HEADER_INIT (&t);
    
    PD_ASSERT (args->ap_fileName);
    PD_ASSERT (args->ap_fileExtension);
    
    err = string_copy (name, PD_STRING, args->ap_fileName->s_name);
    err |= string_add (name, PD_STRING, args->ap_fileExtension->s_name);
    
    if (!err) {
        if (args->ap_fileType == SOUNDFILE_WAVE)      { err = soundfile_writeFileHeaderWAVE (&t, args); }
        else if (args->ap_fileType == SOUNDFILE_AIFF) { err = soundfile_writeFileHeaderAIFF (&t, args); }
        else if (args->ap_fileType == SOUNDFILE_NEXT) { err = soundfile_writeFileHeaderNEXT (&t, args); }
        else {
            err = PD_ERROR;
        }
    }
    
    if (!err) {
    //
    char filepath[PD_STRING] = { 0 };
    
    canvas_makeFilePath (glist, name, filepath, PD_STRING);
    
    f = file_openRaw (filepath, O_CREAT | O_TRUNC | O_WRONLY);
    
    if (f >= 0) { 
        if (write (f, t.h_c, t.h_bytesSet) < t.h_bytesSet) { close (f); f = -1; PD_BUG; }
    }
    //
    }
    
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error soundfile_writeFileCloseWAVE (int f, int itemsWritten, t_audioproperties *args)
{
    int dataSize = itemsWritten * args->ap_bytesPerSample * args->ap_numberOfChannels;
    int swap = args->ap_needToSwap;
    uint32_t t;
    
    if (lseek (f, 4, SEEK_SET) == 4)  { 
        t = soundfile_swap4Bytes ((uint32_t)(SOUNDFILE_HEADER_WAVE - 8 + dataSize), swap);
        if (write (f, (char *)(&t), 4) == 4) {
            if (lseek (f, 40, SEEK_SET) == 40) {
                t = soundfile_swap4Bytes ((uint32_t)dataSize, swap);
                if (write (f, (char *)(&t), 4) == 4) { return PD_ERROR_NONE; }
            }
        }
    }
    
    return PD_ERROR;
}

t_error soundfile_writeFileCloseAIFF (int f, int itemsWritten, t_audioproperties *args)
{
    int dataSize = itemsWritten * args->ap_bytesPerSample * args->ap_numberOfChannels;
    int swap = args->ap_needToSwap;
    uint32_t t;
    
    if (lseek (f, 4, SEEK_SET) == 4)  { 
        t = soundfile_swap4Bytes ((uint32_t)(SOUNDFILE_HEADER_AIFF - 8 + dataSize), swap);
        if (write (f, (char *)(&t), 4) == 4) {
            if (lseek (f, 42, SEEK_SET) == 42) {
                t = soundfile_swap4Bytes ((uint32_t)(dataSize + 8), swap);
                if (write (f, (char *)(&t), 4) == 4) { return PD_ERROR_NONE; }
            }
        }
    }
    
    return PD_ERROR;
}

t_error soundfile_writeFileCloseNEXT (int f, int itemsWritten, t_audioproperties *args)
{
    int dataSize = itemsWritten * args->ap_bytesPerSample * args->ap_numberOfChannels;
    int swap = args->ap_needToSwap;
    uint32_t t;
    
    if (lseek (f, 8, SEEK_SET) == 8)  { 
        t = soundfile_swap4Bytes ((uint32_t)dataSize, swap);
        if (write (f, (char *)(&t), 4) == 4) { return PD_ERROR_NONE; }
    }
    
    return PD_ERROR;
}

t_error soundfile_writeFileClose (int f, int itemsWritten, t_audioproperties *args)
{
    t_error err = PD_ERROR_NONE;
    
    if (itemsWritten < args->ap_numberOfFrames) {                   /* Report properly the data size. */
    //
    err = (args->ap_numberOfFrames != PD_INT_MAX);
    
    if (args->ap_fileType == SOUNDFILE_WAVE) {
        err = soundfile_writeFileCloseWAVE (f, itemsWritten, args);
        
    } else if (args->ap_fileType == SOUNDFILE_AIFF) {
        err = soundfile_writeFileCloseAIFF (f, itemsWritten, args);
        
    } else if (args->ap_fileType == SOUNDFILE_NEXT) {
        err = soundfile_writeFileCloseNEXT (f, itemsWritten, args);
        
    } else {
        err = PD_ERROR;
    }
    
    PD_ASSERT (!err);
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
