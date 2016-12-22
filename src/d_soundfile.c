
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

/* Basic (linear PCM only) audio files handling. */
/* Note that for now unsupported sub-chunks are not preserved at save. */

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
    char            a_fmtID[4];
    uint32_t        a_fmtChunkSize;
    uint16_t        a_numberOfChannels;
    uint16_t        a_numberOfFramesHigh;
    uint16_t        a_numberOfFramesLow;
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

#define SOUNDFILE_HEADER_NEXT           28
#define SOUNDFILE_HEADER_WAVE           44
#define SOUNDFILE_HEADER_AIFF           54

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_SCALE                 (1.0 / (1024.0 * 1024.0 * 1024.0 * 2.0))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SOUNDFILE_BUFFER                PD_STRING

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _checkheader {
    char    ch_c[SOUNDFILE_BUFFER];
    int     ch_bytesRead;
    } t_checkheader;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void soundfile_initialize (void)
{
    PD_ASSERT (SOUNDFILE_BUFFER > 16);
    PD_ASSERT (SOUNDFILE_BUFFER > SOUNDFILE_HEADER_WAVE);
    PD_ASSERT (SOUNDFILE_BUFFER > SOUNDFILE_HEADER_AIFF);
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

static t_error soundfile_openFilePerformWAVE (int f, int swap, t_checkheader *t, t_audioproperties *args)
{
    t_error err = PD_ERROR;

    if (t->ch_bytesRead > SOUNDFILE_HEADER_WAVE) {
    //
    if (!strncmp (t->ch_c + 12, "fmt ", 4)) {
    if (!strncmp (t->ch_c + 36, "data", 4)) {
    //
    int fmtSize          = (int)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 16)), swap);
    int audioFormat      = (int)soundfile_swap2BytesIfNecessary (*((uint16_t *)(t->ch_c + 20)), swap);
    int numberOfChannels = (int)soundfile_swap2BytesIfNecessary (*((uint16_t *)(t->ch_c + 22)), swap);
    int bitsPerSample    = (int)soundfile_swap2BytesIfNecessary (*((uint16_t *)(t->ch_c + 34)), swap);
    int64_t dataSize     = (int64_t)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 40)), swap);
    
    PD_ASSERT (fmtSize == 16);
    PD_ASSERT (audioFormat == 1);   /* Linear PCM. */

    if (audioFormat == 1) {
    //
    if (bitsPerSample == 16 || bitsPerSample == 24 || bitsPerSample == 32) {
    //
    args->ap_headerSize        = SOUNDFILE_HEADER_WAVE;
    args->ap_bytesPerSample    = bitsPerSample / 8;
    args->ap_numberOfChannels  = numberOfChannels;
    args->ap_dataSizeInBytes   = (int64_t)dataSize;
    
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

static t_error soundfile_openFilePerformAIFF (int f, int swap, t_checkheader *t, t_audioproperties *args)
{
    t_error err = PD_ERROR;
    
    if (t->ch_bytesRead > SOUNDFILE_HEADER_AIFF) {
    //
    if (!strncmp (t->ch_c + 12, "COMM", 4)) {
    if (!strncmp (t->ch_c + 38, "SSND", 4)) {
    //
    int numberOfChannels = (int)soundfile_swap2BytesIfNecessary (*((uint16_t *)(t->ch_c + 20)), swap);
    int bitsPerSample    = (int)soundfile_swap2BytesIfNecessary (*((uint16_t *)(t->ch_c + 26)), swap);
    int64_t dataSize     = (int64_t)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 42)), swap);
    int offset           = (int)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 46)), swap);
    int blockAlign       = (int)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 50)), swap);
    
    PD_ASSERT (offset == 0);
    PD_ASSERT (blockAlign == 0);

    if (bitsPerSample == 16 || bitsPerSample == 24 || bitsPerSample == 32) {
    //
    args->ap_headerSize        = SOUNDFILE_HEADER_AIFF;
    args->ap_bytesPerSample    = bitsPerSample / 8;
    args->ap_numberOfChannels  = numberOfChannels;
    args->ap_dataSizeInBytes   = (int64_t)dataSize;
    
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

static t_error soundfile_openFilePerformNEXT (int f, int swap, t_checkheader *t, t_audioproperties *args)
{
    t_error err = PD_ERROR;
    
    if (t->ch_bytesRead > SOUNDFILE_HEADER_NEXT) {
    //
    int dataLocation     = (int)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 4)), swap);
    int64_t dataSize     = (int64_t)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 8)), swap);
    int audioFormat      = (int)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 12)), swap);
    int numberOfChannels = (int)soundfile_swap4BytesIfNecessary (*((uint32_t *)(t->ch_c + 20)), swap);
    int bytesPerSample   = 0;
    
    /* Mu-law format NOT handled. */
    
    /* < https://en.wikipedia.org/wiki/Mu-law > */
    
    if (audioFormat == NS_FORMAT_LINEAR_16)      { bytesPerSample = 2; }
    else if (audioFormat == NS_FORMAT_LINEAR_24) { bytesPerSample = 3; }
    else if (audioFormat == NS_FORMAT_FLOAT)     { bytesPerSample = 4; }
            
    if (bytesPerSample) {
    //
    args->ap_headerSize        = dataLocation;
    args->ap_bytesPerSample    = bytesPerSample;
    args->ap_numberOfChannels  = numberOfChannels;
    args->ap_dataSizeInBytes   = (int64_t)dataSize;
    
    err = PD_ERROR_NONE;
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error soundfile_openFilePerformParseFormat (int f, t_audioproperties *args)
{
    t_error err = PD_ERROR;
    
    t_checkheader t;

    t.ch_bytesRead = read (f, t.ch_c, SOUNDFILE_BUFFER);

    if (t.ch_bytesRead >= 4) {
    //
    int format = SOUNDFILE_NONE;
    
    if (!strncmp (t.ch_c,         ".snd", 4)) { format = SOUNDFILE_NEXT; args->ap_isBigEndian = 1; }
    else if (!strncmp (t.ch_c,    "dns.", 4)) { format = SOUNDFILE_NEXT; args->ap_isBigEndian = 0; }
    else if (t.ch_bytesRead >= 12) {
    //
    if (!strncmp (t.ch_c,         "RIFF", 4)) {
        if (!strncmp (t.ch_c + 8, "WAVE", 4)) { format = SOUNDFILE_WAVE; args->ap_isBigEndian = 0; }
    } else if (!strncmp (t.ch_c,  "FORM", 4)) {
        if (!strncmp (t.ch_c + 8, "AIFF", 4)) { format = SOUNDFILE_AIFF; args->ap_isBigEndian = 1; }
    }
    //
    }
    
    if (format != SOUNDFILE_NONE) {
        int swap = (args->ap_isBigEndian != soundfile_systemIsBigEndian());
        if (format == SOUNDFILE_WAVE) { err = soundfile_openFilePerformWAVE (f, swap, &t, args); }
        if (format == SOUNDFILE_AIFF) { err = soundfile_openFilePerformAIFF (f, swap, &t, args); }
        if (format == SOUNDFILE_NEXT) { err = soundfile_openFilePerformNEXT (f, swap, &t, args); }
    }
    //
    }

    return err;
}
        
static int soundfile_openFilePerform (int f, long skipFrames, t_audioproperties *args)
{
    t_error err = PD_ERROR_NONE;
    
    if (args->ap_headerSize < 0) { err = soundfile_openFilePerformParseFormat (f, args); }
    
    if (!err) {
    //
    int m   = args->ap_numberOfChannels * args->ap_bytesPerSample * skipFrames;
    off_t n = args->ap_headerSize + m;

    err = (lseek (f, n, SEEK_SET) != n);
        
    if (!err) {
        args->ap_dataSizeInBytes = PD_MAX (0, args->ap_dataSizeInBytes - m);
        return f;
    }
    //
    }
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int soundfile_openFile (t_glist *glist, const char *name, long skipFrames, t_audioproperties *args)
{
    char t[PD_STRING] = { 0 };
    char *s;
    
    int f = canvas_openFile (glist, name, "", t, &s, PD_STRING);
    
    if (f >= 0) { return soundfile_openFilePerform (f, skipFrames, args); }
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error soundfile_writeFileParse (t_symbol *s,
    int     *ac,
    t_atom   **av,
    t_symbol **fileName,
    int      *fileType,
    int      *bytesPerSample,
    int      *needToSwap,
    int      *isBigEndian,
    int      *needToNormalize,
    long     *onset,
    long     *numberOfFrames,
    t_float  *sampleRate)
{
    t_error err = PD_ERROR_NONE;
    
    int argc        = *ac;
    t_atom *argv    = *av;
    t_symbol *name  = NULL;
    int type        = SOUNDFILE_NONE;
    int bytes       = 2;
    int swap        = 0;
    int big         = 0;
    int normalize   = 0;
    long skip       = 0;
    long frames     = PD_INT_MAX;
    t_float rate    = -1.0;
    
    int endianness = soundfile_systemIsBigEndian();
        
    while (argc > 0) {
    //
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
    
    if (argc > 1 && (t == sym___dash__s || t == sym___dash__skip)) {
        skip = atom_getFloat (argv + 1);
        skip = PD_MAX (0, skip);
        argc -= 2; argv += 2;
        
    } else if (argc > 1 && (t == sym___dash__f || t == sym___dash__frames || t == sym___dash__nframes)) {
        frames = atom_getFloat (argv + 1);
        frames = PD_MAX (0, frames);
        argc -= 2; argv += 2;
        
    } else if (argc > 1 && (t == sym___dash__b || t == sym___dash__bytes)) {
        bytes = atom_getFloat (argv + 1);
        bytes = PD_CLAMP (bytes, 2, 4);
        argc -= 2; argv += 2;
        
    } else if (argc > 1 && (t == sym___dash__r || t == sym___dash__rate || t == sym___dash__samplerate)) {
        rate = atom_getFloat (argv + 1);
        rate = PD_MAX (1.0, rate);
        argc -= 2; argv += 2;
        
    } else if (t == sym___dash__n || t == sym___dash__normalize) {
        normalize = 1;
        argc --; argv++;
    
    } else if (t == sym___dash__nextstep)   {
        type = SOUNDFILE_NEXT;
        argc --; argv++;
        
    } else if (t == sym___dash__wave)   {
        type = SOUNDFILE_WAVE;
        argc --; argv++;
        
    } else if (t == sym___dash__aiff)   {
        type = SOUNDFILE_AIFF;
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
    name = GET_SYMBOL (argv); 
    
    argc--; argv++;
    
    if (type == SOUNDFILE_NONE) {

        if (string_endWith (name->s_name, ".aif"))          { type = SOUNDFILE_AIFF; }
        else if (string_endWith (name->s_name, ".AIF"))     { type = SOUNDFILE_AIFF; }
        else if (string_endWith (name->s_name, ".aiff"))    { type = SOUNDFILE_AIFF; }
        else if (string_endWith (name->s_name, ".AIFF"))    { type = SOUNDFILE_AIFF; }
        else if (string_endWith (name->s_name, ".snd"))     { type = SOUNDFILE_NEXT; }
        else if (string_endWith (name->s_name, ".SND"))     { type = SOUNDFILE_NEXT; }
        else if (string_endWith (name->s_name, ".au"))      { type = SOUNDFILE_NEXT; }
        else if (string_endWith (name->s_name, ".AU"))      { type = SOUNDFILE_NEXT; }
        else {
            type = SOUNDFILE_WAVE;
        }
    }

    if (bytes == 4 && type == SOUNDFILE_AIFF) { PD_BUG; return PD_ERROR; }
    if (type == SOUNDFILE_WAVE)      { big = 0; }
    else if (type == SOUNDFILE_AIFF) { big = 1; }
    else {
        big = endianness;
    }
    
    swap = (big != soundfile_systemIsBigEndian());
    
    *ac              = argc;
    *av              = argv;
    *fileName        = name;
    *fileType        = type;
    *bytesPerSample  = bytes;
    *needToSwap      = swap;
    *needToNormalize = normalize;
    *onset           = skip;
    *numberOfFrames  = frames;
    *isBigEndian     = big;
    *sampleRate      = rate;
    //
    }
    
    return err;
}

int soundfile_writeFile (t_glist *canvas, const char *filename,
    int filetype, int nframes, int bytespersamp,
    int bigendian, int nchannels, int swap, t_float samplerate)
{
    char filenamebuf[PD_STRING], buf2[PD_STRING];
    char headerbuf[SOUNDFILE_BUFFER];
    t_wave *wavehdr = (t_wave *)headerbuf;
    t_nextstep *nexthdr = (t_nextstep *)headerbuf;
    t_aiff *aiffhdr = (t_aiff *)headerbuf;
    int fd, headersize = 0;
    
    strncpy(filenamebuf, filename, PD_STRING-10);
    filenamebuf[PD_STRING-10] = 0;

    if (filetype == SOUNDFILE_NEXT)
    {
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".snd"))
            strcat(filenamebuf, ".snd");
        if (bigendian)
            strncpy(nexthdr->ns_magic, ".snd", 4);
        else strncpy(nexthdr->ns_magic, "dns.", 4);
        nexthdr->ns_dataLocation = soundfile_swap4BytesIfNecessary(sizeof(*nexthdr), swap);
        nexthdr->ns_dataSize = 0;
        nexthdr->ns_dataFormat = soundfile_swap4BytesIfNecessary((bytespersamp == 3 ? NS_FORMAT_LINEAR_24 :
           (bytespersamp == 4 ? NS_FORMAT_FLOAT : NS_FORMAT_LINEAR_16)), swap);
        nexthdr->ns_samplingRate = soundfile_swap4BytesIfNecessary(samplerate, swap);
        nexthdr->ns_channelCount = soundfile_swap4BytesIfNecessary(nchannels, swap);
        strcpy(nexthdr->ns_info, PD_NAME_SHORT " ");
        soundfile_swapInfoIfNecessary(nexthdr->ns_info, swap);
        headersize = sizeof(t_nextstep);
    }
    else if (filetype == SOUNDFILE_AIFF)
    {
        long datasize = nframes * nchannels * bytespersamp;
        long longtmp;
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".aif") &&
            strcmp(filenamebuf + strlen(filenamebuf)-5, ".aiff"))
                strcat(filenamebuf, ".aif");
        strncpy(aiffhdr->a_fileID, "FORM", 4);
        aiffhdr->a_chunkSize = soundfile_swap4BytesIfNecessary(datasize + sizeof(*aiffhdr) + 4, swap);
        strncpy(aiffhdr->a_aiffID, "AIFF", 4);
        strncpy(aiffhdr->a_fmtID, "COMM", 4);
        aiffhdr->a_fmtChunkSize = soundfile_swap4BytesIfNecessary(18, swap);
        aiffhdr->a_numberOfChannels = soundfile_swap2BytesIfNecessary(nchannels, swap);
        longtmp = soundfile_swap4BytesIfNecessary(nframes, swap);
        memcpy(&aiffhdr->a_numberOfFramesHigh, &longtmp, 4);
        aiffhdr->a_bitsPerSample = soundfile_swap2BytesIfNecessary(8 * bytespersamp, swap);
        soundfile_makeAiff80BitFloat(samplerate, aiffhdr->a_sampleRate);
        strncpy(((char *)(&aiffhdr->a_sampleRate))+10, "SSND", 4);
        longtmp = soundfile_swap4BytesIfNecessary(datasize + 8, swap);
        memcpy(((char *)(&aiffhdr->a_sampleRate))+14, &longtmp, 4);
        memset(((char *)(&aiffhdr->a_sampleRate))+18, 0, 8);
        headersize = SOUNDFILE_HEADER_AIFF;
    }
    else    /* WAVE format */
    {
        long datasize = nframes * nchannels * bytespersamp;
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".wav"))
            strcat(filenamebuf, ".wav");
        strncpy(wavehdr->w_fileID, "RIFF", 4);
        wavehdr->w_chunkSize = soundfile_swap4BytesIfNecessary(datasize + sizeof(*wavehdr) - 8, swap);
        strncpy(wavehdr->w_waveID, "WAVE", 4);
        strncpy(wavehdr->w_fmtID, "fmt ", 4);
        wavehdr->w_fmtChunkSize = soundfile_swap4BytesIfNecessary(16, swap);
        wavehdr->w_audioFormat =
            soundfile_swap2BytesIfNecessary((bytespersamp == 4 ? 3 : 1), swap);
        wavehdr->w_numberOfChannels = soundfile_swap2BytesIfNecessary(nchannels, swap);
        wavehdr->w_samplesPerSecond = soundfile_swap4BytesIfNecessary(samplerate, swap);
        wavehdr->w_bytesPerSecond =
            soundfile_swap4BytesIfNecessary((int)(samplerate * nchannels * bytespersamp), swap);
        wavehdr->w_blockAlign = soundfile_swap2BytesIfNecessary(nchannels * bytespersamp, swap);
        wavehdr->w_bitsPerSample = soundfile_swap2BytesIfNecessary(8 * bytespersamp, swap);
        strncpy(wavehdr->w_dataChunkID, "data", 4);
        wavehdr->w_dataChunkSize = soundfile_swap4BytesIfNecessary(datasize, swap);
        headersize = sizeof(t_wave);
    }

    canvas_makeFilePath(canvas, filenamebuf, buf2, PD_STRING);
    if ((fd = file_openRaw(buf2, O_CREAT | O_TRUNC | O_WRONLY)) < 0)
        return (-1);

    if (write(fd, headerbuf, headersize) < headersize)
    {
        close (fd);
        return (-1);
    }
    return (fd);
}

void soundfile_writeFileClose (char *filename, int fd,
    int filetype, long nframes, long itemswritten, int bytesperframe, int swap)
{
    if (itemswritten < nframes) 
    {
        if (nframes < PD_INT_MAX)
            post_error ("soundfiler_write: %ld out of %ld bytes written",
                itemswritten, nframes);
            /* try to fix size fields in header */
        if (filetype == SOUNDFILE_WAVE)
        {
            long datasize = itemswritten * bytesperframe, mofo;
            
            if (lseek(fd,
                ((char *)(&((t_wave *)0)->w_chunkSize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = soundfile_swap4BytesIfNecessary(datasize + sizeof(t_wave) - 8, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd,
                ((char *)(&((t_wave *)0)->w_dataChunkSize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = soundfile_swap4BytesIfNecessary(datasize, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
        if (filetype == SOUNDFILE_AIFF)
        {
            long mofo;
            if (lseek(fd,
                ((char *)(&((t_aiff *)0)->a_numberOfFramesHigh)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = soundfile_swap4BytesIfNecessary(itemswritten, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd,
                ((char *)(&((t_aiff *)0)->a_chunkSize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            /* SOUNDFILE_HEADER_AIFF ??? 38 ??? */
            mofo = soundfile_swap4BytesIfNecessary(itemswritten*bytesperframe+SOUNDFILE_HEADER_AIFF, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd, (SOUNDFILE_HEADER_AIFF+4), SEEK_SET) == 0)
                goto baddonewrite;
            mofo = soundfile_swap4BytesIfNecessary(itemswritten*bytesperframe, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
        if (filetype == SOUNDFILE_NEXT)
        {
            /* do it the lazy way: just set the size field to 'unknown size'*/
            uint32_t nextsize = 0xffffffff;
            if (lseek(fd, 8, SEEK_SET) == 0)
            {
                goto baddonewrite;
            }
            if (write(fd, &nextsize, 4) < 4)
            {
                goto baddonewrite;
            }
        }
    }
    return;
baddonewrite:
    post("%s: %s", filename, strerror (errno));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void soundfile_xferin_sample(int sfchannels, int nvecs, t_sample **vecs,
    long itemsread, unsigned char *buf, int nitems, int bytespersamp,
    int bigendian, int spread)
{
    int i, j;
    unsigned char *sp, *sp2;
    t_sample *fp;
    int nchannels = (sfchannels < nvecs ? sfchannels : nvecs);
    int bytesperframe = bytespersamp * sfchannels;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[0] << 24) | (sp2[1] << 16));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[1] << 24) | (sp2[0] << 16));
            }
        }
        else if (bytespersamp == 3)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[2] << 24) | (sp2[1] << 16)
                            | (sp2[0] << 8));
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *(long *)fp = ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8) | sp2[3]);
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *(long *)fp = ((sp2[3] << 24) | (sp2[2] << 16)
                            | (sp2[1] << 8) | sp2[0]);
            }
        }
    }
        /* zero out other outputs */
    for (i = sfchannels; i < nvecs; i++)
        for (j = nitems, fp = vecs[i]; j--; )
            *fp++ = 0;

}

void soundfile_xferin_float(int sfchannels, int nvecs, t_float **vecs,
    long itemsread, unsigned char *buf, int nitems, int bytespersamp,
    int bigendian, int spread)
{
    int i, j;
    unsigned char *sp, *sp2;
    t_float *fp;
    int nchannels = (sfchannels < nvecs ? sfchannels : nvecs);
    int bytesperframe = bytespersamp * sfchannels;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[0] << 24) | (sp2[1] << 16));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[1] << 24) | (sp2[0] << 16));
            }
        }
        else if (bytespersamp == 3)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SOUNDFILE_SCALE * ((sp2[2] << 24) | (sp2[1] << 16)
                            | (sp2[0] << 8));
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *(long *)fp = ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8) | sp2[3]);
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *(long *)fp = ((sp2[3] << 24) | (sp2[2] << 16)
                            | (sp2[1] << 8) | sp2[0]);
            }
        }
    }
        /* zero out other outputs */
    for (i = sfchannels; i < nvecs; i++)
        for (j = nitems, fp = vecs[i]; j--; )
            *fp++ = 0;

}

void soundfile_xferout_sample(int nchannels, t_sample **vecs,
    unsigned char *buf, int nitems, long onset, int bytespersamp,
    int bigendian, t_sample normalfactor, int spread)
{
    int i, j;
    unsigned char *sp, *sp2;
    t_sample *fp;
    int bytesperframe = bytespersamp * nchannels;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            t_sample ff = normalfactor * 32768.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp = vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[0] = (xx >> 8);
                    sp2[1] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 3)
        {
            t_sample ff = normalfactor * 8388608.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[0] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[2] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[2] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    t_rawcast32 f2;
                    f2.z_f = *fp * normalfactor;
                    sp2[0] = (f2.z_i >> 24); sp2[1] = (f2.z_i >> 16);
                    sp2[2] = (f2.z_i >> 8); sp2[3] = f2.z_i;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    t_rawcast32 f2;
                    f2.z_f = *fp * normalfactor;
                    sp2[3] = (f2.z_i >> 24); sp2[2] = (f2.z_i >> 16);
                    sp2[1] = (f2.z_i >> 8); sp2[0] = f2.z_i;
                }
            }
        }
    }
}

void soundfile_xferout_float(int nchannels, t_float **vecs,
    unsigned char *buf, int nitems, long onset, int bytespersamp,
    int bigendian, t_sample normalfactor, int spread)
{
    int i, j;
    unsigned char *sp, *sp2;
    t_float *fp;
    int bytesperframe = bytespersamp * nchannels;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            t_sample ff = normalfactor * 32768.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp = vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[0] = (xx >> 8);
                    sp2[1] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 3)
        {
            t_sample ff = normalfactor * 8388608.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[0] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[2] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[2] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    t_rawcast32 f2;
                    f2.z_f = *fp * normalfactor;
                    sp2[0] = (f2.z_i >> 24); sp2[1] = (f2.z_i >> 16);
                    sp2[2] = (f2.z_i >> 8); sp2[3] = f2.z_i;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    t_rawcast32 f2;
                    f2.z_f = *fp * normalfactor;
                    sp2[3] = (f2.z_i >> 24); sp2[2] = (f2.z_i >> 16);
                    sp2[1] = (f2.z_i >> 8); sp2[0] = f2.z_i;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
