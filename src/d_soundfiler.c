
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
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

#define SOUNDFILER_LIMIT_SIZE           (1024 * 1024 * 4)       /* Arbitrary limited to 4 MB. */

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
    if (args->ap_needToResize) {
    //
    int bytesPerFrame    = args->ap_numberOfChannels * args->ap_bytesPerSample;
    int dataSizeInFrames = args->ap_dataSizeInBytes / bytesPerFrame;
    int i;
    
    if (dataSizeInFrames > args->ap_numberOfFrames) {     /* Maximum number of frames required by user. */
        dataSizeInFrames = args->ap_numberOfFrames; 
    }

    for (i = 0; i < channelsRequired; i++) {
        int size;
        garray_resizeWithInteger (a[i], dataSizeInFrames);
        garray_setSaveWithParent (a[i], 0);
        garray_getData (a[i], &size, &w[i]);
        PD_ASSERT (size == dataSizeInFrames);
    }
    
    *m = dataSizeInFrames;
    //
    }

    return PD_ERROR_NONE;
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

static int soundfiler_readPerform (t_glist *glist, int argc, t_atom *argv)
{
    t_audioproperties properties; soundfile_initProperties (&properties);
    
    t_error err = soundfile_readFileParse (sym_soundfiler, &argc, &argv, &properties);
    
    if (!err) {
    //
    t_word   *w[SOUNDFILE_CHANNELS] = { NULL };
    t_garray *a[SOUNDFILE_CHANNELS] = { NULL };

    int arraysSize = SOUNDFILE_UNKNOWN;
    
    /* Fetch the garrays. */
    
    err = soundfiler_readFetch (argc, argv, a, w, &arraysSize, &properties);
    
    if (!err) {
    //
    int f = soundfile_readFileHeader (glist, &properties);     /* WAVE, AIFF or NeXT supported. */
    
    err = (f < 0);
    
    if (!err) { err = (properties.ap_dataSizeInBytes >= SOUNDFILER_LIMIT_SIZE); }
    
    /* Note that at this point the file is positioned at start of the sound. */
    /* Onset is already handled. */
    
    if (!err) {
    
        err = soundfiler_readResizeIfNecessary (f, argc, a, w, &arraysSize, &properties);
        
        if (err) { close (f); }     /* < http://stackoverflow.com/a/13691168 > */
        else {
        
            int i, numberOfFramesRead = soundfiler_readDecode (f, argc, a, w, &arraysSize, &properties);
            
            /* Note that file is closed in function called above. */
            
            for (i = 0; i < argc; i++) {
                if (i >= properties.ap_numberOfChannels) { garray_setDataFromIndex (a[i], 0, (t_float)0.0); }
                else {
                    garray_setDataFromIndex (a[i], numberOfFramesRead, (t_float)0.0);
                }
                garray_redraw (a[i]);
            }
                
            return numberOfFramesRead;
        }
    }
    //
    }
    //
    }
    
    if (err) { error_failsToRead (sym_soundfiler); }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void soundfiler_read (t_soundfiler *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_float (x->x_outlet, (t_float)soundfiler_readPerform (x->x_owner, argc, argv));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error soundfiler_writeFetch (int argc,
    t_atom *argv,
    t_garray **a,
    t_word **w,
    t_sample *m, 
    t_audioproperties *args)
{
    t_error err = (argc < 1 || argc > SOUNDFILE_CHANNELS);
    
    if (!err) {
    //
    t_sample maximum = (t_sample)0.0;
    int i;
        
    for (i = 0; i < argc; i++) {
    //
    t_symbol *t = atom_getSymbolAtIndex (i, argc, argv);

    a[i] = (t_garray *)pd_getThingByClass (t, garray_class);
    
    if (a[i] == NULL) { error_canNotFind (sym_soundfiler, t); err = PD_ERROR; break; }
    else {
        int size;
        t_sample f = garray_getAmplitude (a[i]);
        maximum = PD_MAX (maximum, f);
        garray_getData (a[i], &size, &w[i]);
        args->ap_numberOfFrames = PD_MIN (size - args->ap_onset, args->ap_numberOfFrames);
        err |= (args->ap_numberOfFrames <= 0);
    }
    //
    }
    
    *m = maximum;
    
    args->ap_numberOfChannels = argc;
    //
    }
    
    return err;
}

static t_sample soundfiler_writeGetFactor (t_sample maximumAmplitude, t_audioproperties *args)
{
    t_sample f = (t_sample)1.0;
    
    /* Linear PCM encoding requires a signal in common range. */
    
    if (args->ap_bytesPerSample != 4) { 
        if (maximumAmplitude > 1.0) { args->ap_needToNormalize = 1; }
    }
    
    if (args->ap_needToNormalize) {
        if (maximumAmplitude > 0.0) { f = (t_sample)(32767.0 / (32768.0 * maximumAmplitude)); }
    }
    
    return f;
}

static int soundfiler_writeEncode (int f,
    int channelsRequired,
    t_garray **a,
    t_word **w,
    t_sample *m,
    t_audioproperties *args)
{
    t_sample normalizationFactor = soundfiler_writeGetFactor (*m, args);
    int bytesPerFrame            = args->ap_numberOfChannels * args->ap_bytesPerSample;
    int framesToWrite            = args->ap_numberOfFrames;
    int framesAlreadyWritten     = 0;
    int framesBufferSize         = SOUNDFILER_BUFFER_SIZE / bytesPerFrame;

    PD_ASSERT (args->ap_onset            != SOUNDFILE_UNKNOWN);
    PD_ASSERT (args->ap_numberOfFrames   != SOUNDFILE_UNKNOWN);
    PD_ASSERT (args->ap_numberOfChannels == channelsRequired);
    
    while (framesAlreadyWritten < framesToWrite) {
    //
    char t[SOUNDFILER_BUFFER_SIZE] = { 0 };
    int framesRemaining = framesToWrite - framesAlreadyWritten;
    size_t sizeInFrames = PD_MIN (framesRemaining, framesBufferSize);
    size_t sizeInBytes  = sizeInFrames * bytesPerFrame;
    
    soundfile_encode (args->ap_numberOfChannels,
        (t_float **)w,
        (unsigned char *)t,
        sizeInFrames,
        args->ap_onset,
        args->ap_bytesPerSample,
        args->ap_isBigEndian,
        sizeof (t_word) / sizeof (t_sample),
        normalizationFactor);
   
    {
        ssize_t s = write (f, t, sizeInBytes);
    
        if (s < (ssize_t)sizeInBytes) { 
            if (s > 0) { framesAlreadyWritten += s / bytesPerFrame; }
            PD_ASSERT (s == 0);
            break;
        }
    }
    
    framesAlreadyWritten += sizeInFrames;
    args->ap_onset       += sizeInFrames;
    //
    }
    
    if (soundfile_writeFileClose (f, framesAlreadyWritten, args) == PD_ERROR) {
        PD_BUG;     /* File corrupted; what to do? */
    }
    
    close (f);
    
    return framesAlreadyWritten;
}
    
static int soundfiler_writePerform (t_glist *canvas, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    t_audioproperties properties; soundfile_initProperties (&properties);
    
    err = soundfile_writeFileParse (sym_soundfiler, &argc, &argv, &properties);

    if (!err) {
    //
    t_garray *a[SOUNDFILE_CHANNELS] = { NULL };
    t_word   *w[SOUNDFILE_CHANNELS] = { NULL };

    t_sample maximumAmplitude = (t_sample)0.0;
    
    err = soundfiler_writeFetch (argc, argv, a, w, &maximumAmplitude, &properties);
    
    if (!err) {
    
        int f = soundfile_writeFileHeader (canvas, &properties);
    
        err = (f < 0);
        
        if (!err) { return soundfiler_writeEncode (f, argc, a, w, &maximumAmplitude, &properties); }
        
        /* Opened file is closed in function called above. */
    }
    //
    }
    
    if (err) { error_failsToWrite (sym_soundfiler); }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void soundfiler_write (t_soundfiler *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_float (x->x_outlet, (t_float)soundfiler_writePerform (x->x_owner, argc, argv)); 
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
