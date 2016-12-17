
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



/* Microsoft Visual Studio does not define these... arg */
#ifdef _MSC_VER
#define off_t long
#define O_CREAT   _O_CREAT
#define O_TRUNC   _O_TRUNC
#define O_WRONLY  _O_WRONLY
#endif

/***************** soundfile header structures ************************/

typedef union _samplelong {
  t_sample f;
  long     l;
} t_sampleuint;

#define FORMAT_WAVE 0
#define FORMAT_AIFF 1
#define FORMAT_NEXT 2

/* the NeXTStep sound header structure; can be big or little endian  */

typedef struct _nextstep
{
    char ns_fileid[4];      /* magic number '.snd' if file is big-endian */
    uint32_t ns_onset;      /* byte offset of first sample */
    uint32_t ns_length;     /* length of sound in bytes */
    uint32_t ns_format;     /* format; see below */
    uint32_t ns_sr;         /* sample rate */
    uint32_t ns_nchans;     /* number of channels */
    char ns_info[4];        /* comment */
} t_nextstep;

#define NS_FORMAT_LINEAR_16     3
#define NS_FORMAT_LINEAR_24     4
#define NS_FORMAT_FLOAT         6
#define SCALE (1./(1024. * 1024. * 1024. * 2.))

/* the WAVE header.  All Wave files are little endian.  We assume
    the "fmt" chunk comes first which is usually the case but perhaps not
    always; same for AIFF and the "COMM" chunk.   */

typedef unsigned word;
typedef unsigned long dword;

typedef struct _wave
{
    char  w_fileid[4];              /* chunk id 'RIFF'            */
    uint32_t w_chunksize;           /* chunk size                 */
    char  w_waveid[4];              /* wave chunk id 'WAVE'       */
    char  w_fmtid[4];               /* format chunk id 'fmt '     */
    uint32_t w_fmtchunksize;        /* format chunk size          */
    uint16_t w_fmttag;              /* format tag (WAV_INT etc)   */
    uint16_t w_nchannels;           /* number of channels         */
    uint32_t w_samplespersec;       /* sample rate in hz          */
    uint32_t w_navgbytespersec;     /* average bytes per second   */
    uint16_t w_nblockalign;         /* number of bytes per frame  */
    uint16_t w_nbitspersample;      /* number of bits in a sample */
    char  w_datachunkid[4];         /* data chunk id 'data'       */
    uint32_t w_datachunksize;       /* length of data chunk       */
} t_wave;

typedef struct _fmt         /* format chunk */
{
    uint16_t f_fmttag;              /* format tag, 1 for PCM      */
    uint16_t f_nchannels;           /* number of channels         */
    uint32_t f_samplespersec;       /* sample rate in hz          */
    uint32_t f_navgbytespersec;     /* average bytes per second   */
    uint16_t f_nblockalign;         /* number of bytes per frame  */
    uint16_t f_nbitspersample;      /* number of bits in a sample */
} t_fmt;

typedef struct _wavechunk           /* ... and the last two items */
{
    char  wc_id[4];                 /* data chunk id, e.g., 'data' or 'fmt ' */
    uint32_t wc_size;               /* length of data chunk       */
} t_wavechunk;

#define WAV_INT 1
#define WAV_FLOAT 3

/* the AIFF header.  I'm assuming AIFC is compatible but don't really know
    that. */

typedef struct _datachunk
{
    char  dc_id[4];                 /* data chunk id 'SSND'       */
    uint32_t dc_size;               /* length of data chunk       */
    uint32_t dc_offset;             /* additional offset in bytes */
    uint32_t dc_block;              /* block size                 */
} t_datachunk;

typedef struct _comm
{
    uint16_t c_nchannels;           /* number of channels         */
    uint16_t c_nframeshi;           /* # of sample frames (hi)    */
    uint16_t c_nframeslo;           /* # of sample frames (lo)    */
    uint16_t c_bitspersamp;         /* bits per sample            */
    unsigned char c_samprate[10];   /* sample rate, 80-bit float! */
} t_comm;

    /* this version is more convenient for writing them out: */
typedef struct _aiff
{
    char  a_fileid[4];              /* chunk id 'FORM'            */
    uint32_t a_chunksize;           /* chunk size                 */
    char  a_aiffid[4];              /* aiff chunk id 'AIFF'       */
    char  a_fmtid[4];               /* format chunk id 'COMM'     */
    uint32_t a_fmtchunksize;        /* format chunk size, 18      */
    uint16_t a_nchannels;           /* number of channels         */
    uint16_t a_nframeshi;           /* # of sample frames (hi)    */
    uint16_t a_nframeslo;           /* # of sample frames (lo)    */
    uint16_t a_bitspersamp;         /* bits per sample            */
    unsigned char a_samprate[10];   /* sample rate, 80-bit float! */
} t_aiff;

#define AIFFHDRSIZE 38      /* probably not what sizeof() gives */


#define AIFFPLUS (AIFFHDRSIZE + 16)  /* header size including SSND chunk hdr */

#define WHDR1 sizeof(t_nextstep)
#define WHDR2 (sizeof(t_wave) > WHDR1 ? sizeof (t_wave) : WHDR1)
#define WRITEHDRSIZE (AIFFPLUS > WHDR2 ? AIFFPLUS : WHDR2)

#define READHDRSIZE (16 > WHDR2 + 2 ? 16 : WHDR2 + 2)

#define OBUFSIZE PD_STRING  /* assume PD_STRING is bigger than headers */

/* this routine returns 1 if the high order byte comes at the lower
address on our architecture (big-endianness.).  It's 1 for Motorola,
0 for Intel: */

int garray_ambigendian(void)
{
    unsigned short s = 1;
    unsigned char c = *(char *)(&s);
    return (c==0);
}

/* byte swappers */

static uint32_t swap4(uint32_t n, int doit)
{
    if (doit)
        return (((n & 0xff) << 24) | ((n & 0xff00) << 8) |
            ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
    else return (n);
}

static uint16_t swap2(uint32_t n, int doit)
{
    if (doit)
        return (((n & 0xff) << 8) | ((n & 0xff00) >> 8));
    else return (n);
}

static void swapstring(char *foo, int doit)
{
    if (doit)
    {
        char a = foo[0], b = foo[1], c = foo[2], d = foo[3];
        foo[0] = d; foo[1] = c; foo[2] = b; foo[3] = a;
    }
}

    /* write a sample rate as an 80-bit AIFF-compatible number */
static void makeaiffsamprate(double sr, unsigned char *shit)
{
    int exponent;
    double mantissa = frexp(sr, &exponent);
    unsigned long fixmantissa = ldexp(mantissa, 32);
    shit[0] = (exponent+16382)>>8;
    shit[1] = exponent+16382;
    shit[2] = fixmantissa >> 24;
    shit[3] = fixmantissa >> 16;
    shit[4] = fixmantissa >> 8;
    shit[5] = fixmantissa;
    shit[6] = shit[7] = shit[8] = shit[9] = 0;
}

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
            char b_c[OBUFSIZE];
            t_fmt b_fmt;
            t_nextstep b_nextstep;
            t_wavechunk b_wavechunk;
            t_datachunk b_datachunk;
            t_comm b_commchunk;
        } buf;
        int bytesread = read(fd, buf.b_c, READHDRSIZE);
        int format;
        if (bytesread < 4)
            goto badheader;
        if (!strncmp(buf.b_c, ".snd", 4))
            format = FORMAT_NEXT, bigendian = 1;
        else if (!strncmp(buf.b_c, "dns.", 4))
            format = FORMAT_NEXT, bigendian = 0;
        else if (!strncmp(buf.b_c, "RIFF", 4))
        {
            if (bytesread < 12 || strncmp(buf.b_c + 8, "WAVE", 4))
                goto badheader;
            format = FORMAT_WAVE, bigendian = 0;
        }
        else if (!strncmp(buf.b_c, "FORM", 4))
        {
            if (bytesread < 12 || strncmp(buf.b_c + 8, "AIFF", 4))
                goto badheader;
            format = FORMAT_AIFF, bigendian = 1;
        }
        else
            goto badheader;
        swap = (bigendian != garray_ambigendian());
        if (format == FORMAT_NEXT)   /* nextstep header */
        {
            t_nextstep *nsbuf = &buf.b_nextstep;
            if (bytesread < (int)sizeof(t_nextstep))
                goto badheader;
            nchannels = swap4(nsbuf->ns_nchans, swap);
            format = swap4(nsbuf->ns_format, swap);
            headersize = swap4(nsbuf->ns_onset, swap);
            if (format == NS_FORMAT_LINEAR_16)
                bytespersamp = 2;
            else if (format == NS_FORMAT_LINEAR_24)
                bytespersamp = 3;
            else if (format == NS_FORMAT_FLOAT)
                bytespersamp = 4;
            else goto badheader;
            bytelimit = PD_INT_MAX;
        }
        else if (format == FORMAT_WAVE)     /* wave header */
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
                    ((t_wavechunk *)buf)->wc_id[0],
                    ((t_wavechunk *)buf)->wc_id[1],
                    ((t_wavechunk *)buf)->wc_id[2],
                    ((t_wavechunk *)buf)->wc_id[3]); */
                /* read chunks in loop until we get to the data chunk */
            while (strncmp(wavechunk->wc_id, "data", 4))
            {
                long chunksize = swap4(wavechunk->wc_size,
                    swap), seekto = headersize + chunksize + 8, seekout;
                if (seekto & 1)     /* pad up to even number of bytes */
                    seekto++;                
                if (!strncmp(wavechunk->wc_id, "fmt ", 4))
                {
                    long commblockonset = headersize + 8;
                    seekout = lseek(fd, commblockonset, SEEK_SET);
                    if (seekout != commblockonset)
                        goto badheader;
                    if (read(fd, buf.b_c, sizeof(t_fmt)) < (int) sizeof(t_fmt))
                            goto badheader;
                    nchannels = swap2(buf.b_fmt.f_nchannels, swap);
                    format = swap2(buf.b_fmt.f_nbitspersample, swap);
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
                    wavechunk->wc_id[0],
                    wavechunk->wc_id[1],
                    wavechunk->wc_id[2],
                    wavechunk->wc_id[3], seekto); */
                headersize = seekto;
            }
            bytelimit = swap4(wavechunk->wc_size, swap);
            headersize += 8;
        }
        else
        {
                /* AIFF.  same as WAVE; actually predates it.  Disgusting. */
            t_datachunk *datachunk;
            headersize = 12;
            if (bytesread < 20)
                goto badheader;
                /* First we guess a number of channels, etc., in case there's
                no COMM block to follow. */
            nchannels = 1;
            bytespersamp = 2;
                /* copy the first chunk header to beginnning of buffer. */
            memcpy(buf.b_c, buf.b_c + headersize, sizeof(t_datachunk));
                /* read chunks in loop until we get to the data chunk */
            datachunk = &buf.b_datachunk;
            while (strncmp(datachunk->dc_id, "SSND", 4))
            {
                long chunksize = swap4(datachunk->dc_size,
                    swap), seekto = headersize + chunksize + 8, seekout;
                if (seekto & 1)     /* pad up to even number of bytes */
                    seekto++;
                /* post("chunk %c %c %c %c seek %d",
                    datachunk->dc_id[0],
                    datachunk->dc_id[1],
                    datachunk->dc_id[2],
                    datachunk->dc_id[3], seekto); */
                if (!strncmp(datachunk->dc_id, "COMM", 4))
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
                    nchannels = swap2(commchunk->c_nchannels, swap);
                    format = swap2(commchunk->c_bitspersamp, swap);
                    if (format == 16)
                        bytespersamp = 2;
                    else if (format == 24)
                        bytespersamp = 3;
                    else goto badheader;
                }
                seekout = lseek(fd, seekto, SEEK_SET);
                if (seekout != seekto)
                    goto badheader;
                if (read(fd, buf.b_c, sizeof(t_datachunk)) <
                    (int) sizeof(t_datachunk))
                        goto badheader;
                headersize = seekto;
            }
            bytelimit = swap4(datachunk->dc_size, swap) - 8;
            headersize += sizeof(t_datachunk);
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
    char buf[OBUFSIZE], *bufptr;
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
    char buf[OBUFSIZE], *bufptr;
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
                        *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SCALE * ((sp2[1] << 24) | (sp2[0] << 16));
            }
        }
        else if (bytespersamp == 3)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SCALE * ((sp2[2] << 24) | (sp2[1] << 16)
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

static void soundfile_xferin_float(int sfchannels, int nvecs, t_float **vecs,
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
                        *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SCALE * ((sp2[1] << 24) | (sp2[0] << 16));
            }
        }
        else if (bytespersamp == 3)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + spread * itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                        *fp = SCALE * ((sp2[2] << 24) | (sp2[1] << 16)
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
            filetype = FORMAT_WAVE;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "nextstep"))
        {
            filetype = FORMAT_NEXT;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "aiff"))
        {
            filetype = FORMAT_AIFF;
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
                filetype = FORMAT_AIFF;
        if (strlen(filesym->s_name) >= 6 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 5, ".aiff") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 5, ".AIFF")))
                filetype = FORMAT_AIFF;
        if (strlen(filesym->s_name) >= 5 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".snd") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".SND")))
                filetype = FORMAT_NEXT;
        if (strlen(filesym->s_name) >= 4 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 3, ".au") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 3, ".AU")))
                filetype = FORMAT_NEXT;
        if (filetype < 0)
            filetype = FORMAT_WAVE;
    }
        /* don't handle AIFF floating point samples */
    if (bytespersamp == 4)
    {
        if (filetype == FORMAT_AIFF)
        {
            post_error ("AIFF floating-point file format unavailable");
            goto usage;
        }
    }
        /* for WAVE force little endian; for nextstep use machine native */
    if (filetype == FORMAT_WAVE)
    {
        bigendian = 0;
        if (endianness == 1)
            post_error ("WAVE file forced to little endian");
    }
    else if (filetype == FORMAT_AIFF)
    {
        bigendian = 1;
        if (endianness == 0)
            post_error ("AIFF file forced to big endian");
    }
    else if (endianness == -1)
    {
        bigendian = garray_ambigendian();
    }
    else bigendian = endianness;
    swap = (bigendian != garray_ambigendian());
    
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
    char headerbuf[WRITEHDRSIZE];
    t_wave *wavehdr = (t_wave *)headerbuf;
    t_nextstep *nexthdr = (t_nextstep *)headerbuf;
    t_aiff *aiffhdr = (t_aiff *)headerbuf;
    int fd, headersize = 0;
    
    strncpy(filenamebuf, filename, PD_STRING-10);
    filenamebuf[PD_STRING-10] = 0;

    if (filetype == FORMAT_NEXT)
    {
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".snd"))
            strcat(filenamebuf, ".snd");
        if (bigendian)
            strncpy(nexthdr->ns_fileid, ".snd", 4);
        else strncpy(nexthdr->ns_fileid, "dns.", 4);
        nexthdr->ns_onset = swap4(sizeof(*nexthdr), swap);
        nexthdr->ns_length = 0;
        nexthdr->ns_format = swap4((bytespersamp == 3 ? NS_FORMAT_LINEAR_24 :
           (bytespersamp == 4 ? NS_FORMAT_FLOAT : NS_FORMAT_LINEAR_16)), swap);
        nexthdr->ns_sr = swap4(samplerate, swap);
        nexthdr->ns_nchans = swap4(nchannels, swap);
        strcpy(nexthdr->ns_info, PD_NAME_SHORT " ");
        swapstring(nexthdr->ns_info, swap);
        headersize = sizeof(t_nextstep);
    }
    else if (filetype == FORMAT_AIFF)
    {
        long datasize = nframes * nchannels * bytespersamp;
        long longtmp;
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".aif") &&
            strcmp(filenamebuf + strlen(filenamebuf)-5, ".aiff"))
                strcat(filenamebuf, ".aif");
        strncpy(aiffhdr->a_fileid, "FORM", 4);
        aiffhdr->a_chunksize = swap4(datasize + sizeof(*aiffhdr) + 4, swap);
        strncpy(aiffhdr->a_aiffid, "AIFF", 4);
        strncpy(aiffhdr->a_fmtid, "COMM", 4);
        aiffhdr->a_fmtchunksize = swap4(18, swap);
        aiffhdr->a_nchannels = swap2(nchannels, swap);
        longtmp = swap4(nframes, swap);
        memcpy(&aiffhdr->a_nframeshi, &longtmp, 4);
        aiffhdr->a_bitspersamp = swap2(8 * bytespersamp, swap);
        makeaiffsamprate(samplerate, aiffhdr->a_samprate);
        strncpy(((char *)(&aiffhdr->a_samprate))+10, "SSND", 4);
        longtmp = swap4(datasize + 8, swap);
        memcpy(((char *)(&aiffhdr->a_samprate))+14, &longtmp, 4);
        memset(((char *)(&aiffhdr->a_samprate))+18, 0, 8);
        headersize = AIFFPLUS;
    }
    else    /* WAVE format */
    {
        long datasize = nframes * nchannels * bytespersamp;
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".wav"))
            strcat(filenamebuf, ".wav");
        strncpy(wavehdr->w_fileid, "RIFF", 4);
        wavehdr->w_chunksize = swap4(datasize + sizeof(*wavehdr) - 8, swap);
        strncpy(wavehdr->w_waveid, "WAVE", 4);
        strncpy(wavehdr->w_fmtid, "fmt ", 4);
        wavehdr->w_fmtchunksize = swap4(16, swap);
        wavehdr->w_fmttag =
            swap2((bytespersamp == 4 ? WAV_FLOAT : WAV_INT), swap);
        wavehdr->w_nchannels = swap2(nchannels, swap);
        wavehdr->w_samplespersec = swap4(samplerate, swap);
        wavehdr->w_navgbytespersec =
            swap4((int)(samplerate * nchannels * bytespersamp), swap);
        wavehdr->w_nblockalign = swap2(nchannels * bytespersamp, swap);
        wavehdr->w_nbitspersample = swap2(8 * bytespersamp, swap);
        strncpy(wavehdr->w_datachunkid, "data", 4);
        wavehdr->w_datachunksize = swap4(datasize, swap);
        headersize = sizeof(t_wave);
    }

    canvas_makeFilePath(canvas, filenamebuf, buf2, PD_STRING);
    if ((fd = file_openRaw(buf2, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
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
        if (filetype == FORMAT_WAVE)
        {
            long datasize = itemswritten * bytesperframe, mofo;
            
            if (lseek(fd,
                ((char *)(&((t_wave *)0)->w_chunksize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = swap4(datasize + sizeof(t_wave) - 8, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd,
                ((char *)(&((t_wave *)0)->w_datachunksize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = swap4(datasize, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
        if (filetype == FORMAT_AIFF)
        {
            long mofo;
            if (lseek(fd,
                ((char *)(&((t_aiff *)0)->a_nframeshi)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = swap4(itemswritten, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd,
                ((char *)(&((t_aiff *)0)->a_chunksize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = swap4(itemswritten*bytesperframe+AIFFHDRSIZE, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd, (AIFFHDRSIZE+4), SEEK_SET) == 0)
                goto baddonewrite;
            mofo = swap4(itemswritten*bytesperframe, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
        if (filetype == FORMAT_NEXT)
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
                    t_sampleuint f2;
                    f2.f = *fp * normalfactor;
                    sp2[0] = (f2.l >> 24); sp2[1] = (f2.l >> 16);
                    sp2[2] = (f2.l >> 8); sp2[3] = f2.l;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    t_sampleuint f2;
                    f2.f = *fp * normalfactor;
                    sp2[3] = (f2.l >> 24); sp2[2] = (f2.l >> 16);
                    sp2[1] = (f2.l >> 8); sp2[0] = f2.l;
                }
            }
        }
    }
}
static void soundfile_xferout_float(int nchannels, t_float **vecs,
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
                    t_sampleuint f2;
                    f2.f = *fp * normalfactor;
                    sp2[0] = (f2.l >> 24); sp2[1] = (f2.l >> 16);
                    sp2[2] = (f2.l >> 8); sp2[3] = f2.l;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp += spread)
                {
                    t_sampleuint f2;
                    f2.f = *fp * normalfactor;
                    sp2[3] = (f2.l >> 24); sp2[2] = (f2.l >> 16);
                    sp2[1] = (f2.l >> 8); sp2[0] = f2.l;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ------- soundfiler - reads and writes soundfiles to/from "garrays" ---- */
#define DEFMAXSIZE 4000000      /* default maximum 16 MB per channel */
#define SAMPBUFSIZE 1024


static t_class *soundfiler_class;

typedef struct _soundfiler
{
    t_object x_obj;
    t_glist *x_canvas;
} t_soundfiler;

static t_soundfiler *soundfiler_new(void)
{
    t_soundfiler *x = (t_soundfiler *)pd_new(soundfiler_class);
    x->x_canvas = canvas_getCurrent();
    outlet_new(&x->x_obj, &s_float);
    return x;
}

    /* soundfiler_read ...
    
    usage: read [flags] filename table ...
    flags:
        -skip <frames> ... frames to skip in file
        -onset <frames> ... onset in table to read into (NOT DONE YET)
        -raw <headersize channels bytes endian>
        -resize
        -maxsize <max-size>
    */

static void soundfiler_read(t_soundfiler *x, t_symbol *s,
    int argc, t_atom *argv)
{
    int headersize = -1, channels = 0, bytespersamp = 0, bigendian = 0,
        resize = 0, i, j;
    long skipframes = 0, finalsize = 0, itemsleft,
        maxsize = DEFMAXSIZE, itemsread = 0, bytelimit  = PD_INT_MAX;
    int fd = -1;
    char endianness, *filename;
    t_garray *garrays[MAXSFCHANS];
    t_word *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes, nitems;
    FILE *fp;
    while (argc > 0 && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        char *flag = argv->a_w.w_symbol->s_name + 1;
        if (!strcmp(flag, "skip"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((skipframes = argv[1].a_w.w_float) < 0))
                    goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(flag, "raw"))
        {
            if (argc < 5 ||
                argv[1].a_type != A_FLOAT ||
                ((headersize = argv[1].a_w.w_float) < 0) ||
                argv[2].a_type != A_FLOAT ||
                ((channels = argv[2].a_w.w_float) < 1) ||
                (channels > MAXSFCHANS) || 
                argv[3].a_type != A_FLOAT ||
                ((bytespersamp = argv[3].a_w.w_float) < 2) || 
                    (bytespersamp > 4) ||
                argv[4].a_type != A_SYMBOL ||
                    ((endianness = argv[4].a_w.w_symbol->s_name[0]) != 'b'
                    && endianness != 'l' && endianness != 'n'))
                        goto usage;
            if (endianness == 'b')
                bigendian = 1;
            else if (endianness == 'l')
                bigendian = 0;
            else
                bigendian = garray_ambigendian();
            argc -= 5; argv += 5;
        }
        else if (!strcmp(flag, "resize"))
        {
            resize = 1;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "maxsize"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((maxsize = argv[1].a_w.w_float) < 0))
                    goto usage;
            resize = 1;     /* maxsize implies resize. */
            argc -= 2; argv += 2;
        }
        else goto usage;
    }
    if (argc < 2 || argc > MAXSFCHANS + 1 || argv[0].a_type != A_SYMBOL)
        goto usage;
    filename = argv[0].a_w.w_symbol->s_name;
    argc--; argv++;
    
    for (i = 0; i < argc; i++)
    {
        int vecsize;
        if (argv[i].a_type != A_SYMBOL)
            goto usage;
        if (!(garrays[i] =
            (t_garray *)pd_getThingByClass(argv[i].a_w.w_symbol, garray_class)))
        {
            post_error ("%s: no such table", argv[i].a_w.w_symbol->s_name);
            goto done;
        }
        else if (!garray_getData(garrays[i], &vecsize,  /* Always true now !!! */
                &vecs[i]))
            post_error ("%s: bad template for tabwrite",
                argv[i].a_w.w_symbol->s_name);
        if (finalsize && finalsize != vecsize && !resize)
        {
            post("soundfiler_read: arrays have different lengths; resizing...");
            resize = 1;
        }
        finalsize = vecsize;
    }
    fd = open_soundfile_via_canvas(x->x_canvas, filename,
        headersize, &bytespersamp, &bigendian, &channels, &bytelimit,
            skipframes);
    
    if (fd < 0)
    {
        post_error ("soundfiler_read: %s: %s", filename, (errno == EIO ?
            "unknown or bad header format" : strerror(errno)));
        goto done;
    }

    if (resize)
    {
            /* figure out what to resize to */
        long poswas, eofis, framesinfile;
        
        poswas = lseek(fd, 0, SEEK_CUR);
        eofis = lseek(fd, 0, SEEK_END);
        if (poswas < 0 || eofis < 0 || eofis < poswas)
        {
            post_error ("soundfiler_read: lseek failed");
            goto done;
        }
        lseek(fd, poswas, SEEK_SET);
        framesinfile = (eofis - poswas) / (channels * bytespersamp);
        if (framesinfile > maxsize)
        {
            post_error ("soundfiler_read: truncated to %ld elements", maxsize);
            framesinfile = maxsize;
        }
        if (framesinfile > bytelimit / (channels * bytespersamp))
            framesinfile = bytelimit / (channels * bytespersamp);
        finalsize = framesinfile;
        for (i = 0; i < argc; i++)
        {
            int vecsize;

            garray_resizeWithInteger (garrays[i], (int)finalsize);   /* 64-32 CAST !!! */
            
                /* for sanity's sake let's clear the save-in-patch flag here */
            garray_setSaveWithParent(garrays[i], 0);
            garray_getData(garrays[i], &vecsize, 
                &vecs[i]);
                /* if the resize failed, garray_resize reported the error */
            if (vecsize != framesinfile)
            {
                post_error ("resize failed");
                goto done;
            }
        }
    }
    if (!finalsize) finalsize = PD_INT_MAX;
    if (finalsize > bytelimit / (channels * bytespersamp))
        finalsize = bytelimit / (channels * bytespersamp);
    fp = fdopen(fd, "rb");
    bufframes = SAMPBUFSIZE / (channels * bytespersamp);

    for (itemsread = 0; itemsread < finalsize; )
    {
        int thisread = finalsize - itemsread;
        thisread = (thisread > bufframes ? bufframes : thisread);
        nitems = fread(sampbuf, channels * bytespersamp, thisread, fp);
        if (nitems <= 0) break;
        soundfile_xferin_float(channels, argc, (t_float **)vecs, itemsread,
            (unsigned char *)sampbuf, nitems, bytespersamp, bigendian,
                sizeof (t_word)/sizeof(t_sample));
        itemsread += nitems;
    }
        /* zero out remaining elements of vectors */
        
    for (i = 0; i < argc; i++)
    {
        int nzero, vecsize;
        garray_getData(garrays[i], &vecsize, &vecs[i]);
        for (j = itemsread; j < vecsize; j++)
            vecs[i][j].w_float = 0;
    }
        /* zero out vectors in excess of number of channels */
    for (i = channels; i < argc; i++)
    {
        int vecsize;
        t_word *foo;
        garray_getData(garrays[i], &vecsize, &foo);
        for (j = 0; j < vecsize; j++)
            foo[j].w_float = 0;
    }
        /* do all graphics updates */
    for (i = 0; i < argc; i++)
        garray_redraw(garrays[i]);
    fclose(fp);
    fd = -1;
    goto done;
usage:
    post_error ("usage: read [flags] filename tablename...");
    post("flags: -skip <n> -resize -maxsize <n> ...");
    post("-raw <headerbytes> <channels> <bytespersamp> <endian (b, l, or n)>.");
done:
    if (fd >= 0)
        close (fd);
    outlet_float(x->x_obj.te_outlet, (t_float)itemsread); 
}

    /* this is broken out from soundfiler_write below so garray_write can
    call it too... not done yet though. */

long soundfiler_dowrite(void *obj, t_glist *canvas,
    int argc, t_atom *argv)
{
    int headersize, bytespersamp, bigendian,
        endianness, swap, filetype, normalize, i, j, nchannels;
    long onset, nframes, itemsleft,
        maxsize = DEFMAXSIZE, itemswritten = 0;
    t_garray *garrays[MAXSFCHANS];
    t_word *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes, nitems;
    int fd = -1;
    t_sample normfactor, biggest = 0;
    t_float samplerate;
    t_symbol *filesym;

    if (soundfiler_writeargparse(obj, &argc, &argv, &filesym, &filetype,
        &bytespersamp, &swap, &bigendian, &normalize, &onset, &nframes,
            &samplerate))
                goto usage;
    nchannels = argc;
    if (nchannels < 1 || nchannels > MAXSFCHANS)
        goto usage;
    if (samplerate < 0)
        samplerate = audio_getSampleRate();
    for (i = 0; i < nchannels; i++)
    {
        int vecsize;
        if (argv[i].a_type != A_SYMBOL)
            goto usage;
        if (!(garrays[i] =
            (t_garray *)pd_getThingByClass(argv[i].a_w.w_symbol, garray_class)))
        {
            post_error ("%s: no such table", argv[i].a_w.w_symbol->s_name);
            goto fail;
        }
        else if (!garray_getData(garrays[i], &vecsize, &vecs[i])) /* Always true now !!! */
            post_error ("%s: bad template for tabwrite",
                argv[i].a_w.w_symbol->s_name);
        if (nframes > vecsize - onset)
            nframes = vecsize - onset;
        
        for (j = 0; j < vecsize; j++)
        {
            if (vecs[i][j].w_float > biggest)
                biggest = vecs[i][j].w_float;
            else if (-vecs[i][j].w_float > biggest)
                biggest = -vecs[i][j].w_float;
        }
    }
    if (nframes <= 0)
    {
        post_error ("soundfiler_write: no samples at onset %ld", onset);
        goto fail;
    }

    if ((fd = create_soundfile(canvas, filesym->s_name, filetype,
        nframes, bytespersamp, bigendian, nchannels,
            swap, samplerate)) < 0)
    {
        post("%s: %s\n", filesym->s_name, strerror(errno));
        goto fail;
    }
    if (!normalize)
    {
        if ((bytespersamp != 4) && (biggest > 1))
        {
            post("%s: normalizing max amplitude %g to 1", filesym->s_name, biggest);
            normalize = 1;
        }
        else post("%s: biggest amplitude = %g", filesym->s_name, biggest);
    }
    if (normalize)
        normfactor = (biggest > 0 ? 32767./(32768. * biggest) : 1);
    else normfactor = 1;

    bufframes = SAMPBUFSIZE / (nchannels * bytespersamp);

    for (itemswritten = 0; itemswritten < nframes; )
    {
        int thiswrite = nframes - itemswritten, nitems, nbytes;
        thiswrite = (thiswrite > bufframes ? bufframes : thiswrite);
        soundfile_xferout_float(argc, (t_float **)vecs, (unsigned char *)sampbuf,
            thiswrite, onset, bytespersamp, bigendian, normfactor,
                 sizeof (t_word)/sizeof(t_sample));
        nbytes = write(fd, sampbuf, nchannels * bytespersamp * thiswrite);
        if (nbytes < nchannels * bytespersamp * thiswrite)
        {
            post("%s: %s", filesym->s_name, strerror(errno));
            if (nbytes > 0)
                itemswritten += nbytes / (nchannels * bytespersamp);
            break;
        }
        itemswritten += thiswrite;
        onset += thiswrite * (sizeof (t_word)/sizeof(float));
    }
    if (fd >= 0)
    {
        soundfile_finishwrite(obj, filesym->s_name, fd,
            filetype, nframes, itemswritten, nchannels * bytespersamp, swap);
        close (fd);
    }
    return ((float)itemswritten); 
usage:
    post_error ("usage: write [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -bytes <n> -wave -aiff -nextstep ...");
    post("-big -little -normalize");
    post("(defaults to a 16-bit wave file).");
fail:
    if (fd >= 0)
        close (fd);
    return (0); 
}

static void soundfiler_write(t_soundfiler *x, t_symbol *s,
    int argc, t_atom *argv)
{
    long bozo = soundfiler_dowrite(x, x->x_canvas,
        argc, argv);
    outlet_float(x->x_obj.te_outlet, (t_float)bozo); 
}

void soundfiler_setup(void)
{
    soundfiler_class = class_new(sym_soundfiler, (t_newmethod)soundfiler_new, 
        0, sizeof(t_soundfiler), 0, 0);
    class_addMethod(soundfiler_class, (t_method)soundfiler_read, sym_read, 
        A_GIMME, 0);
    class_addMethod(soundfiler_class, (t_method)soundfiler_write,
        sym_write, A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
