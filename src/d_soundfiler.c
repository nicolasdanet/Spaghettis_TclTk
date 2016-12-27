
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

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *soundfiler_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _soundfiler {
    t_object    x_obj;                  /* Must be the first. */
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    } t_soundfiler;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILER_MAXIMUM_SIZE         (1024 * 1024 * 4)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SOUNDFILER_BUFFER_SIZE          1024

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void soundfiler_read (t_soundfiler *x, t_symbol *s, int argc, t_atom *argv)
{
    int headersize = -1, channels = 0, bytespersamp = 0, bigendian = 0,
        resize = 0, i, j;
    long skipframes = 0, finalsize = 0, itemsleft,
        maxsize = SOUNDFILER_MAXIMUM_SIZE, itemsread = 0, bytelimit  = PD_INT_MAX;
    int fd = -1;
    char endianness, *filename;
    t_garray *garrays[SOUNDFILE_MAXIMUM_CHANNELS];
    t_word *vecs[SOUNDFILE_MAXIMUM_CHANNELS];
    char sampbuf[SOUNDFILER_BUFFER_SIZE];
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
                (channels > SOUNDFILE_MAXIMUM_CHANNELS) || 
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
                bigendian = soundfile_systemIsBigEndian();
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
    if (argc < 2 || argc > SOUNDFILE_MAXIMUM_CHANNELS + 1 || argv[0].a_type != A_SYMBOL)
        goto usage;
    filename = argv[0].a_w.w_symbol->s_name;
    t_symbol *fileNameSym = GET_SYMBOL (argv);
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
    t_audioproperties args;
    args.ap_fileName = fileNameSym;
    args.ap_fileExtension = &s_;
    args.ap_headerSize = headersize;
    args.ap_isBigEndian = bigendian;
    args.ap_bytesPerSample = bytespersamp;
    args.ap_numberOfChannels = channels;
    args.ap_dataSizeInBytes = bytelimit;
    args.ap_onset = skipframes;
    
    fd = soundfile_readFileHeader (x->x_owner, &args);
    
    headersize = args.ap_headerSize;
    bigendian = args.ap_isBigEndian;
    bytespersamp = args.ap_bytesPerSample;
    channels = args.ap_numberOfChannels;
    bytelimit = args.ap_dataSizeInBytes;
    
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
    bufframes = SOUNDFILER_BUFFER_SIZE / (channels * bytespersamp);

    for (itemsread = 0; itemsread < finalsize; )
    {
        int thisread = finalsize - itemsread;
        thisread = (thisread > bufframes ? bufframes : thisread);
        nitems = fread(sampbuf, channels * bytespersamp, thisread, fp);
        if (nitems <= 0) break;
        soundfile_decode (channels, (t_float **)vecs, 
            (unsigned char *)sampbuf, nitems, itemsread, bytespersamp, bigendian,
                sizeof (t_word)/sizeof(t_sample), argc);
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

long soundfiler_performWrite (void *dummy, t_glist *canvas, int argc, t_atom *argv)
{
    int headersize, bytespersamp, bigendian,
        endianness, swap, filetype, normalize, i, j, nchannels;
    long onset, nframes, itemsleft,
        maxsize = SOUNDFILER_MAXIMUM_SIZE, itemswritten = 0;
    t_garray *garrays[SOUNDFILE_MAXIMUM_CHANNELS];
    t_word *vecs[SOUNDFILE_MAXIMUM_CHANNELS];
    char sampbuf[SOUNDFILER_BUFFER_SIZE];
    int bufframes, nitems;
    int fd = -1;
    t_sample normfactor, biggest = 0;
    t_float samplerate;
    t_symbol *filesym;
    t_symbol *fileExtension;
    
    t_audioproperties prop;
    
    
    if (soundfile_writeFileParse(sym_soundfiler, &argc, &argv, &prop) == PD_ERROR)
                goto usage;
                
    filesym = prop.ap_fileName;
    fileExtension = prop.ap_fileExtension;
    samplerate = prop.ap_sampleRate;
    filetype = prop.ap_fileType;
    bytespersamp = prop.ap_bytesPerSample;
    bigendian = prop.ap_isBigEndian;
    swap = prop.ap_needToSwap;
    onset = prop.ap_onset;
    nframes = prop.ap_numberOfFrames;
    normalize = prop.ap_needToNormalize;
    
    nchannels = argc;
    if (nchannels < 1 || nchannels > SOUNDFILE_MAXIMUM_CHANNELS)
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

    prop.ap_fileName = filesym;
    prop.ap_fileExtension = fileExtension;
    prop.ap_sampleRate = samplerate;
    prop.ap_fileType = filetype;
    prop.ap_numberOfChannels = nchannels;
    prop.ap_bytesPerSample = bytespersamp;
    prop.ap_isBigEndian = bigendian;
    prop.ap_needToSwap = swap;
    prop.ap_numberOfFrames = nframes;
    prop.ap_needToNormalize;
    
    if ((fd = soundfile_writeFileHeader (canvas, &prop)) < 0)
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

    bufframes = SOUNDFILER_BUFFER_SIZE / (nchannels * bytespersamp);

    for (itemswritten = 0; itemswritten < nframes; )
    {
        int thiswrite = nframes - itemswritten, nitems, nbytes;
        thiswrite = (thiswrite > bufframes ? bufframes : thiswrite);
        soundfile_encode(argc, (t_float **)vecs, (unsigned char *)sampbuf,
            thiswrite, onset, bytespersamp, bigendian,
                 sizeof (t_word)/sizeof(t_sample), normfactor);
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
        soundfile_writeFileClose (fd,
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

static void soundfiler_write (t_soundfiler *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_float (x->x_obj.te_outlet, (t_float)soundfiler_performWrite (x, x->x_owner, argc, argv)); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_soundfiler *soundfiler_new (void)
{
    t_soundfiler *x = (t_soundfiler *)pd_new (soundfiler_class);
    
    x->x_owner  = canvas_getCurrent();
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void soundfiler_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_soundfiler,
            (t_newmethod)soundfiler_new, 
            NULL,
            sizeof (t_soundfiler),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addMethod (c, (t_method)soundfiler_read,  sym_read,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)soundfiler_write, sym_write,  A_GIMME, A_NULL);
    
    soundfiler_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
