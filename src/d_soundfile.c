
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

#define SOUNDFILE_WAVE          0
#define SOUNDFILE_AIFF          1
#define SOUNDFILE_NEXT          2

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

#define NS_FORMAT_LINEAR_16     3
#define NS_FORMAT_LINEAR_24     4
#define NS_FORMAT_FLOAT         6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* The WAVE header. */ 
/* All WAVE files are little-endian. */
/* Assume that "fmt" chunk comes first. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://soundfile.sapp.org/doc/WaveFormat/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _wave {                      /* Contains fmt chunk for convenience. */
    char        w_fileID[4];
    uint32_t    w_chunkSize;
    char        w_waveID[4];
    char        w_fmtID[4];
    uint32_t    w_fmtChunkSize;
    uint16_t    w_fmtTag;
    uint16_t    w_numberOfChannels;
    uint32_t    w_samplesPerSecond;
    uint32_t    w_bytesPerSecond;
    uint16_t    w_blockAlign;
    uint16_t    w_bitsPerSample;
    char        w_dataChunkID[4];
    uint32_t    w_dataChunkSize;
    } t_wave;

typedef struct _fmt {
    uint16_t    f_fmtTag;
    uint16_t    f_numberOfChannels;
    uint32_t    f_samplesPerSecond;
    uint32_t    f_bytesPerSecond;
    uint16_t    f_blockAlign;
    uint16_t    f_bitsPerSamples;
    } t_fmt;

typedef struct _wavechunk {
    char        wc_ID[4];
    uint32_t    wc_size;
    } t_wavechunk;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define WAV_INT                 1
#define WAV_FLOAT               3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* The AIFF header. */
/* All WAVE files are big-endian. */
/* Assumed that "COMM" chunk comes first. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://muratnkonar.com/aiff/index.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _aiff {                      /* Contains COMM chunk for convenience. */
    char            a_fileID[4];
    uint32_t        a_chunkSize;
    char            a_aiffID[4];
    char            a_fmtID[4];
    uint32_t        a_fmtChunkSize;
    uint16_t        a_numberOfChannels;
    uint16_t        a_framesHigh;
    uint16_t        a_framesLow;
    uint16_t        a_bitsPerSample;
    unsigned char   a_sampleRate[10];
    } t_aiff;

typedef struct _comm {
    uint16_t        c_numberOfChannels;
    uint16_t        c_framesHigh;
    uint16_t        c_framesLow;
    uint16_t        c_bitsPerSamples;
    unsigned char   c_sampleRate[10];
    } t_comm;

typedef struct _aiffchunk {
    char            ac_ID[4];
    uint32_t        ac_size;
    uint32_t        ac_offset;
    uint32_t        ac_block;
    } t_aiffchunk;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_SIZE_NEXTSTEP         28
#define SOUNDFILE_SIZE_WAVE             44
#define SOUNDFILE_SIZE_WAVECHUNK        8
#define SOUNDFILE_SIZE_AIFF             38
#define SOUNDFILE_SIZE_AIFFCHUNK        16

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILE_SCALE                 (1.0 / (1024.0 * 1024.0 * 1024.0 * 2.0))

#define SOUNDFILE_STRING                PD_STRING

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void soundfile_initialize (void)
{
    PD_ASSERT (sizeof (t_nextstep)  == SOUNDFILE_SIZE_NEXTSTEP);
    PD_ASSERT (sizeof (t_wave)      == SOUNDFILE_SIZE_WAVE);
    PD_ASSERT (sizeof (t_wavechunk) == SOUNDFILE_SIZE_WAVECHUNK);
    //PD_ASSERT (sizeof (t_aiff)    == SOUNDFILE_SIZE_AIFF);
    PD_ASSERT (sizeof (t_aiffchunk) == SOUNDFILE_SIZE_AIFFCHUNK);
}

void soundfile_release (void)
{

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/******************** soundfile access routines **********************/

/* This routine opens a file, looks for either a nextstep or "wave" header,
* seeks to end of it, and fills in bytes per sample and number of channels.
* Only 2- and 3-byte fixed-point samples and 4-byte floating point samples
* are supported.  If "headersize" is nonzero, the
* caller should supply the number of channels, endinanness, and bytes per
* sample; the header is ignored.  Otherwise, the routine tries to read the
* header and fill in the properties.
*/

int open_soundfile_via_fd(int fd, int headersize,
    int *p_bytespersamp, int *p_bigendian, int *p_nchannels, long *p_bytelimit,
    long skipframes)
{
    int format, nchannels, bigendian, bytespersamp, swap, sysrtn;
    long bytelimit = PD_INT_MAX;
    errno = 0;
    if (headersize >= 0) /* header detection overridden */
    {
        bigendian = *p_bigendian;
        nchannels = *p_nchannels;
        bytespersamp = *p_bytespersamp;
        bytelimit = *p_bytelimit;
    }
    else
    {
        union
        {
            char b_c[SOUNDFILE_STRING];
            t_fmt b_fmt;
            t_nextstep b_nextstep;
            t_wavechunk b_wavechunk;
            t_aiffchunk b_datachunk;
            t_comm b_commchunk;
        } buf;
        
        int SOUNDFILE_HEADER_READ = (PD_MAX (16, (PD_MAX (sizeof (t_wave), sizeof (t_nextstep)) + 2)));
        int bytesread = read(fd, buf.b_c, SOUNDFILE_HEADER_READ);
        int format;
        if (bytesread < 4)
            goto badheader;
        if (!strncmp(buf.b_c, ".snd", 4))
            format = SOUNDFILE_NEXT, bigendian = 1;
        else if (!strncmp(buf.b_c, "dns.", 4))
            format = SOUNDFILE_NEXT, bigendian = 0;
        else if (!strncmp(buf.b_c, "RIFF", 4))
        {
            if (bytesread < 12 || strncmp(buf.b_c + 8, "WAVE", 4))
                goto badheader;
            format = SOUNDFILE_WAVE, bigendian = 0;
        }
        else if (!strncmp(buf.b_c, "FORM", 4))
        {
            if (bytesread < 12 || strncmp(buf.b_c + 8, "AIFF", 4))
                goto badheader;
            format = SOUNDFILE_AIFF, bigendian = 1;
        }
        else
            goto badheader;
        swap = (bigendian != soundfile_systemIsBigEndian());
        if (format == SOUNDFILE_NEXT)   /* nextstep header */
        {
            t_nextstep *nsbuf = &buf.b_nextstep;
            if (bytesread < (int)sizeof(t_nextstep))
                goto badheader;
            nchannels = soundfile_swap4BytesIfNecessary(nsbuf->ns_channelCount, swap);
            format = soundfile_swap4BytesIfNecessary(nsbuf->ns_dataFormat, swap);
            headersize = soundfile_swap4BytesIfNecessary(nsbuf->ns_dataLocation, swap);
            if (format == NS_FORMAT_LINEAR_16)
                bytespersamp = 2;
            else if (format == NS_FORMAT_LINEAR_24)
                bytespersamp = 3;
            else if (format == NS_FORMAT_FLOAT)
                bytespersamp = 4;
            else goto badheader;
            bytelimit = PD_INT_MAX;
        }
        else if (format == SOUNDFILE_WAVE)     /* wave header */
        {
               t_wavechunk *wavechunk = &buf.b_wavechunk;
               /*  This is awful.  You have to skip over chunks,
               except that if one happens to be a "fmt" chunk, you want to
               find out the format from that one.  The case where the
               "fmt" chunk comes after the audio isn't handled. */
            headersize = 12;
            if (bytesread < 20)
                goto badheader;
                /* First we guess a number of channels, etc., in case there's
                no "fmt" chunk to follow. */
            nchannels = 1;
            bytespersamp = 2;
                /* copy the first chunk header to beginnning of buffer. */
            memcpy(buf.b_c, buf.b_c + headersize, sizeof(t_wavechunk));
            /* post("chunk %c %c %c %c",
                    ((t_wavechunk *)buf)->wc_ID[0],
                    ((t_wavechunk *)buf)->wc_ID[1],
                    ((t_wavechunk *)buf)->wc_ID[2],
                    ((t_wavechunk *)buf)->wc_ID[3]); */
                /* read chunks in loop until we get to the data chunk */
            while (strncmp(wavechunk->wc_ID, "data", 4))
            {
                long chunksize = soundfile_swap4BytesIfNecessary(wavechunk->wc_size,
                    swap), seekto = headersize + chunksize + 8, seekout;
                if (seekto & 1)     /* pad up to even number of bytes */
                    seekto++;                
                if (!strncmp(wavechunk->wc_ID, "fmt ", 4))
                {
                    long commblockonset = headersize + 8;
                    seekout = lseek(fd, commblockonset, SEEK_SET);
                    if (seekout != commblockonset)
                        goto badheader;
                    if (read(fd, buf.b_c, sizeof(t_fmt)) < (int) sizeof(t_fmt))
                            goto badheader;
                    nchannels = soundfile_swap2BytesIfNecessary(buf.b_fmt.f_numberOfChannels, swap);
                    format = soundfile_swap2BytesIfNecessary(buf.b_fmt.f_bitsPerSamples, swap);
                    if (format == 16)
                        bytespersamp = 2;
                    else if (format == 24)
                        bytespersamp = 3;
                    else if (format == 32)
                        bytespersamp = 4;
                    else goto badheader;
                }
                seekout = lseek(fd, seekto, SEEK_SET);
                if (seekout != seekto)
                    goto badheader;
                if (read(fd, buf.b_c, sizeof(t_wavechunk)) <
                    (int) sizeof(t_wavechunk))
                        goto badheader;
                /* post("new chunk %c %c %c %c at %d",
                    wavechunk->wc_ID[0],
                    wavechunk->wc_ID[1],
                    wavechunk->wc_ID[2],
                    wavechunk->wc_ID[3], seekto); */
                headersize = seekto;
            }
            bytelimit = soundfile_swap4BytesIfNecessary(wavechunk->wc_size, swap);
            headersize += 8;
        }
        else
        {
                /* AIFF.  same as WAVE; actually predates it.  Disgusting. */
            t_aiffchunk *datachunk;
            headersize = 12;
            if (bytesread < 20)
                goto badheader;
                /* First we guess a number of channels, etc., in case there's
                no COMM block to follow. */
            nchannels = 1;
            bytespersamp = 2;
                /* copy the first chunk header to beginnning of buffer. */
            memcpy(buf.b_c, buf.b_c + headersize, sizeof(t_aiffchunk));
                /* read chunks in loop until we get to the data chunk */
            datachunk = &buf.b_datachunk;
            while (strncmp(datachunk->ac_ID, "SSND", 4))
            {
                long chunksize = soundfile_swap4BytesIfNecessary(datachunk->ac_size,
                    swap), seekto = headersize + chunksize + 8, seekout;
                if (seekto & 1)     /* pad up to even number of bytes */
                    seekto++;
                /* post("chunk %c %c %c %c seek %d",
                    datachunk->ac_ID[0],
                    datachunk->ac_ID[1],
                    datachunk->ac_ID[2],
                    datachunk->ac_ID[3], seekto); */
                if (!strncmp(datachunk->ac_ID, "COMM", 4))
                {
                    long commblockonset = headersize + 8;
                    t_comm *commchunk;
                    seekout = lseek(fd, commblockonset, SEEK_SET);
                    if (seekout != commblockonset)
                        goto badheader;
                    if (read(fd, buf.b_c, sizeof(t_comm)) <
                        (int) sizeof(t_comm))
                            goto badheader;
                    commchunk = &buf.b_commchunk;
                    nchannels = soundfile_swap2BytesIfNecessary(commchunk->c_numberOfChannels, swap);
                    format = soundfile_swap2BytesIfNecessary(commchunk->c_bitsPerSamples, swap);
                    if (format == 16)
                        bytespersamp = 2;
                    else if (format == 24)
                        bytespersamp = 3;
                    else goto badheader;
                }
                seekout = lseek(fd, seekto, SEEK_SET);
                if (seekout != seekto)
                    goto badheader;
                if (read(fd, buf.b_c, sizeof(t_aiffchunk)) <
                    (int) sizeof(t_aiffchunk))
                        goto badheader;
                headersize = seekto;
            }
            bytelimit = soundfile_swap4BytesIfNecessary(datachunk->ac_size, swap) - 8;
            headersize += sizeof(t_aiffchunk);
        }
    }
        /* seek past header and any sample frames to skip */
    sysrtn = lseek(fd,
        ((off_t)nchannels) * bytespersamp * skipframes + headersize, 0);
    if (sysrtn != nchannels * bytespersamp * skipframes + headersize)
        return (-1);
     bytelimit -= nchannels * bytespersamp * skipframes;
     if (bytelimit < 0)
        bytelimit = 0;
        /* copy sample format back to caller */
    *p_bigendian = bigendian;
    *p_nchannels = nchannels;
    *p_bytespersamp = bytespersamp;
    *p_bytelimit = bytelimit;
    return (fd);
badheader:
        /* the header wasn't recognized.  We're threadable here so let's not
        print out the error... */
    errno = EIO;
    return (-1);
}

    /* open a soundfile, using file_openConsideringSearchPath().  This is used by readsf~ in
    a not-perfectly-threadsafe way.  LATER replace with a thread-hardened
    version of open_soundfile_via_canvas() */
int open_soundfile(const char *dirname, const char *filename, int headersize,
    int *p_bytespersamp, int *p_bigendian, int *p_nchannels, long *p_bytelimit,
    long skipframes)
{
    char buf[SOUNDFILE_STRING], *bufptr;
    int fd;
    fd = file_openConsideringSearchPath(dirname, filename, "", buf, &bufptr, PD_STRING);
    if (fd < 0)
        return (-1);
    else return (open_soundfile_via_fd(fd, headersize, p_bytespersamp,
        p_bigendian, p_nchannels, p_bytelimit, skipframes));
}

    /* open a soundfile, using open_via_canvas().  This is used by readsf~ in
    a not-perfectly-threadsafe way.  LATER replace with a thread-hardened
    version of open_soundfile_via_canvas() */
int open_soundfile_via_canvas(t_glist *canvas, const char *filename, int headersize,
    int *p_bytespersamp, int *p_bigendian, int *p_nchannels, long *p_bytelimit,
    long skipframes)
{
    char buf[SOUNDFILE_STRING], *bufptr;
    int fd;
    fd = canvas_openFile(canvas, filename, "", buf, &bufptr, PD_STRING);
    if (fd < 0)
        return (-1);
    else return (open_soundfile_via_fd(fd, headersize, p_bytespersamp,
        p_bigendian, p_nchannels, p_bytelimit, skipframes));
}

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

    /* soundfiler_write ...
 
    usage: write [flags] filename table ...
    flags:
        -nframes <frames>
        -skip <frames>
        -bytes <bytes per sample>
        -normalize
        -nextstep
        -wave
        -big
        -little
    */

    /* the routine which actually does the work should LATER also be called
    from garray_write16. */


    /* Parse arguments for writing.  The "obj" argument is only for flagging
    errors.  For streaming to a file the "normalize", "onset" and "nframes"
    arguments shouldn't be set but the calling routine flags this. */

int soundfiler_writeargparse(void *obj, int *p_argc, t_atom **p_argv,
    t_symbol **p_filesym,
    int *p_filetype, int *p_bytespersamp, int *p_swap, int *p_bigendian,
    int *p_normalize, long *p_onset, long *p_nframes, t_float *p_rate)
{
    int argc = *p_argc;
    t_atom *argv = *p_argv;
    int bytespersamp = 2, bigendian = 0,
        endianness = -1, swap, filetype = -1, normalize = 0;
    long onset = 0, nframes = PD_INT_MAX;
    t_symbol *filesym;
    t_float rate = -1;
    
    while (argc > 0 && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        char *flag = argv->a_w.w_symbol->s_name + 1;
        if (!strcmp(flag, "skip"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((onset = argv[1].a_w.w_float) < 0))
                    goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(flag, "nframes"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((nframes = argv[1].a_w.w_float) < 0))
                    goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(flag, "bytes"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((bytespersamp = argv[1].a_w.w_float) < 2) ||
                    bytespersamp > 4)
                        goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(flag, "normalize"))
        {
            normalize = 1;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "wave"))
        {
            filetype = SOUNDFILE_WAVE;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "nextstep"))
        {
            filetype = SOUNDFILE_NEXT;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "aiff"))
        {
            filetype = SOUNDFILE_AIFF;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "big"))
        {
            endianness = 1;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "little"))
        {
            endianness = 0;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "r") || !strcmp(flag, "rate"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((rate = argv[1].a_w.w_float) <= 0))
                    goto usage;
            argc -= 2; argv += 2;
        }
        else goto usage;
    }
    if (!argc || argv->a_type != A_SYMBOL)
        goto usage;
    filesym = argv->a_w.w_symbol;
    
        /* check if format not specified and fill in */
    if (filetype < 0) 
    {
        if (strlen(filesym->s_name) >= 5 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".aif") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".AIF")))
                filetype = SOUNDFILE_AIFF;
        if (strlen(filesym->s_name) >= 6 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 5, ".aiff") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 5, ".AIFF")))
                filetype = SOUNDFILE_AIFF;
        if (strlen(filesym->s_name) >= 5 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".snd") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".SND")))
                filetype = SOUNDFILE_NEXT;
        if (strlen(filesym->s_name) >= 4 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 3, ".au") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 3, ".AU")))
                filetype = SOUNDFILE_NEXT;
        if (filetype < 0)
            filetype = SOUNDFILE_WAVE;
    }
        /* don't handle AIFF floating point samples */
    if (bytespersamp == 4)
    {
        if (filetype == SOUNDFILE_AIFF)
        {
            post_error ("AIFF floating-point file format unavailable");
            goto usage;
        }
    }
        /* for WAVE force little endian; for nextstep use machine native */
    if (filetype == SOUNDFILE_WAVE)
    {
        bigendian = 0;
        if (endianness == 1)
            post_error ("WAVE file forced to little endian");
    }
    else if (filetype == SOUNDFILE_AIFF)
    {
        bigendian = 1;
        if (endianness == 0)
            post_error ("AIFF file forced to big endian");
    }
    else if (endianness == -1)
    {
        bigendian = soundfile_systemIsBigEndian();
    }
    else bigendian = endianness;
    swap = (bigendian != soundfile_systemIsBigEndian());
    
    argc--; argv++;
    
    *p_argc = argc;
    *p_argv = argv;
    *p_filesym = filesym;
    *p_filetype = filetype;
    *p_bytespersamp = bytespersamp;
    *p_swap = swap;
    *p_normalize = normalize;
    *p_onset = onset;
    *p_nframes = nframes;
    *p_bigendian = bigendian;
    *p_rate = rate;
    return (0);
usage:
    return (-1);
}

int create_soundfile(t_glist *canvas, const char *filename,
    int filetype, int nframes, int bytespersamp,
    int bigendian, int nchannels, int swap, t_float samplerate)
{
    char filenamebuf[PD_STRING], buf2[PD_STRING];
    char headerbuf[SOUNDFILE_STRING];
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
        soundfile_swap4bytesInfoIfNecessary(nexthdr->ns_info, swap);
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
        memcpy(&aiffhdr->a_framesHigh, &longtmp, 4);
        aiffhdr->a_bitsPerSample = soundfile_swap2BytesIfNecessary(8 * bytespersamp, swap);
        soundfile_makeAiff80BitFloat(samplerate, aiffhdr->a_sampleRate);
        strncpy(((char *)(&aiffhdr->a_sampleRate))+10, "SSND", 4);
        longtmp = soundfile_swap4BytesIfNecessary(datasize + 8, swap);
        memcpy(((char *)(&aiffhdr->a_sampleRate))+14, &longtmp, 4);
        memset(((char *)(&aiffhdr->a_sampleRate))+18, 0, 8);
        headersize = SOUNDFILE_SIZE_AIFF + SOUNDFILE_SIZE_AIFFCHUNK;
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
        wavehdr->w_fmtTag =
            soundfile_swap2BytesIfNecessary((bytespersamp == 4 ? WAV_FLOAT : WAV_INT), swap);
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

void soundfile_finishwrite(void *obj, char *filename, int fd,
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
                ((char *)(&((t_aiff *)0)->a_framesHigh)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = soundfile_swap4BytesIfNecessary(itemswritten, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd,
                ((char *)(&((t_aiff *)0)->a_chunkSize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = soundfile_swap4BytesIfNecessary(itemswritten*bytesperframe+SOUNDFILE_SIZE_AIFF, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd, (SOUNDFILE_SIZE_AIFF+4), SEEK_SET) == 0)
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
