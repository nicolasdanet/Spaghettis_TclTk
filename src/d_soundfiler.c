
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

#define SOUNDFILER_BUFFER_SIZE          1024

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error soundfiler_readFetch (int argc,
    t_atom *argv,
    t_garray **a,
    t_word **w,
    int *m, 
    t_audioproperties *args)
{
    t_error err = PD_ERROR_NONE;
    
    int i, k;
    int size = SOUNDFILE_UNKNOWN;
    
    for (i = 0; i < argc; i++) {
    //
    t_symbol *t = atom_getSymbolAtIndex (i, argc, argv);
    
    a[i] = (t_garray *)pd_getThingByClass (t, garray_class);
    
    if (a[i] == NULL) { error_canNotFind (sym_soundfiler, t); err = PD_ERROR; break; }
    else {
        garray_getData (a[i], &k, &w[i]);
        if (size != SOUNDFILE_UNKNOWN && size != k) {       /* Resized if unequally allocated at first. */
            args->ap_needToResize = 1;
        }
        size = k;
    }
    //
    }
    
    *m = args->ap_needToResize ? SOUNDFILE_UNKNOWN : size;
    
    return err;
}

static t_error soundfiler_readResizeIfNecessary (int f,
    int channelsRequired,
    t_garray **a,
    t_word **w,
    int *m,
    t_audioproperties *args)
{
    t_error err = PD_ERROR_NONE;
    
    /* Note that at this point the file is positioned at start of the sound. */
    
    if (args->ap_needToResize) {
    //
    off_t current     = lseek (f, 0, SEEK_CUR);
    off_t end         = lseek (f, 0, SEEK_END);
    int bytesPerFrame = args->ap_numberOfChannels * args->ap_bytesPerSample;
    int dataSize      = end - current;
    int frames        = dataSize / bytesPerFrame;
    
    if (current < 0 || end < 0 || dataSize < 0) { PD_BUG; err = PD_ERROR; }
    else {
    //
    int i;
    
    /* If the data size header field of the audio file is wrong correct it. */
    /* It is done silently for convenience. */
    
    if (dataSize != args->ap_dataSizeInBytes) { args->ap_dataSizeInBytes = dataSize; PD_BUG; }

    lseek (f, current, SEEK_SET);
    
    if (frames > args->ap_numberOfFrames) {                 /* Maximum number of frames required by user. */
        frames = args->ap_numberOfFrames; 
    }

    for (i = 0; i < channelsRequired; i++) {
        int size;
        garray_resizeWithInteger (a[i], frames);
        garray_setSaveWithParent (a[i], 0);
        garray_getData (a[i], &size, &w[i]);
        PD_ASSERT (size == frames);
    }
    
    *m = frames;
    //
    }
    //
    }

    return err;
}

static int soundfiler_readDecode (int f,
    int channelsRequired,
    t_garray **a,
    t_word **w,
    int *m,
    t_audioproperties *args)
{
    int bytesPerFrame      = args->ap_numberOfChannels * args->ap_bytesPerSample;
    int frames             = args->ap_dataSizeInBytes / bytesPerFrame;
    int framesToRead       = PD_MIN (frames, *m);
    int framesAlreadyRead  = 0;
    int framesBufferSize   = SOUNDFILER_BUFFER_SIZE / bytesPerFrame;
    
    FILE *fp = fdopen (f, "rb");
    
    PD_ASSERT (fp);
    PD_ASSERT (*m != SOUNDFILE_UNKNOWN);
    
    #if PD_WITH_DEBUG
    
    {
    
    int i;
    PD_ASSERT (fp);
    PD_ASSERT (*m != SOUNDFILE_UNKNOWN);
    for (i = 0; i < channelsRequired; i++) {
        PD_ASSERT (framesToRead <= garray_getSize (a[i]));
    }
    
    }
    
    #endif
    
    while (framesAlreadyRead < framesToRead) {
    //
    char t[SOUNDFILER_BUFFER_SIZE] = { 0 };
    int framesRemaining = framesToRead - framesAlreadyRead;
    size_t size = PD_MIN (framesRemaining, framesBufferSize);
    
    if (fread (t, (size_t)bytesPerFrame, (size_t)size, fp) != size) { PD_BUG; break; }
    else {
        soundfile_decode (args->ap_numberOfChannels,
            (t_sample **)w, 
            (unsigned char *)t,
            size,
            framesAlreadyRead,
            args->ap_bytesPerSample,
            args->ap_isBigEndian,
            sizeof (t_word) / sizeof (t_sample),
            channelsRequired);
    }
            
    framesAlreadyRead += size;
    //
    }
    
    fclose (fp);
    
    return framesAlreadyRead;
}

static void soundfiler_read (t_soundfiler *x, t_symbol *s, int argc, t_atom *argv)
{
    t_audioproperties properties; soundfile_initProperties (&properties);
    
    t_error err = soundfile_readFileParse (sym_soundfiler, &argc, &argv, &properties);
    
    if (!err) {
    //
    t_word   *w[SOUNDFILE_MAXIMUM_CHANNELS] = { NULL };
    t_garray *a[SOUNDFILE_MAXIMUM_CHANNELS] = { NULL };

    int arraysSize = SOUNDFILE_UNKNOWN;
    
    /* Fetch the garrays. */
    
    err = soundfiler_readFetch (argc, argv, a, w, &arraysSize, &properties);
    
    if (!err) {
    //
    int f = soundfile_readFileHeader (x->x_owner, &properties);     /* WAVE, AIFF or NeXT supported. */
    
    err = (f < 0);
    
    if (!err) {
    
        err = soundfiler_readResizeIfNecessary (f, argc, a, w, &arraysSize, &properties);
        
        if (err) { close (f); }     /* < http://stackoverflow.com/a/13691168 > */
        else {
        
            int i, numberOfFramesRead = soundfiler_readDecode (f, argc, a, w, &arraysSize, &properties);
                
            for (i = 0; i < argc; i++) {
                if (i >= properties.ap_numberOfChannels) { garray_setDataFromIndex (a[i], 0, 0.0); }
                else {
                    garray_setDataFromIndex (a[i], numberOfFramesRead, 0.0);
                }
                garray_redraw (a[i]);
            }
                
            outlet_float (x->x_outlet, (t_float)numberOfFramesRead);
        }
    }
    //
    }
    //
    }
    
    if (err) { error_failsToRead (sym_soundfiler); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

long soundfiler_performWrite (void *dummy, t_glist *canvas, int argc, t_atom *argv)
{
    int headersize, bytespersamp, bigendian,
        endianness, swap, filetype, normalize, i, j, nchannels;
    long onset, nframes, itemsleft, itemswritten = 0;
    t_garray *garrays[SOUNDFILE_MAXIMUM_CHANNELS];
    t_word *vecs[SOUNDFILE_MAXIMUM_CHANNELS];
    char sampbuf[SOUNDFILER_BUFFER_SIZE];
    int bufframes, nitems;
    int fd = -1;
    t_sample normfactor, biggest = 0;
    t_float samplerate;
    t_symbol *filesym;
    t_symbol *fileExtension;
    
    t_audioproperties prop; soundfile_initProperties (&prop);
    
    
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
        soundfile_writeFileClose (fd, itemswritten, &prop);
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
