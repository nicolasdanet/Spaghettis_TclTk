
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_soundfile.h"
#include "d_sfthread.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *readsf_tilde_class;                                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _readsf_tilde {
    t_object            sf_obj;                                     /* Must be the first. */
    t_audioproperties   sf_properties;
    int                 sf_numberOfAudioOutlets;
    int                 sf_bufferSize;
    int                 sf_run;
    t_sfthread          *sf_thread;
    t_glist             *sf_owner;
    t_clock             *sf_clock;
    t_sample            *(sf_vectorsOut[SOUNDFILE_CHANNELS]);
    t_outlet            *(sf_audioOutlets[SOUNDFILE_CHANNELS]);
    t_outlet            *sf_outletTopRight;
    } t_readsf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define READSF_BUFFER_SIZE      65536                               /* Power of two. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void readsf_tilde_close (t_readsf_tilde *x)
{
    if (x->sf_thread) { sfthread_release (x->sf_thread); x->sf_thread = NULL; }
    
    x->sf_run = 0;
    
    soundfile_propertiesInit (&x->sf_properties);
}

static void readsf_tilde_open (t_readsf_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    readsf_tilde_close (x);
    
    {
    //
    t_audioproperties *p = &x->sf_properties;
    
    t_error err = soundfile_readFileParse (sym_readsf__tilde__, &argc, &argv, p);
    
    if (!err) {
    //
    t_fileproperties t;
    
    err = !(glist_fileExist (x->sf_owner, p->ap_fileName->s_name, p->ap_fileExtension->s_name, &t));
    
    if (!err) {
    //
    int f = soundfile_readFileHeader (x->sf_owner, &x->sf_properties);
    err = (f < 0);
    if (!err) {
        x->sf_thread = sfthread_new (SFTHREAD_READER, x->sf_bufferSize, f, &x->sf_properties);
        PD_ASSERT (x->sf_thread);
    }
    //
    }
    //
    }
    
    if (err) { error_canNotOpen (p->ap_fileName); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void readsf_tilde_start (t_readsf_tilde *x)
{
    if (!x->sf_thread) { error_unexpected (sym_readsf__tilde__, sym_start); }
    else {
        x->sf_run = 1;
    }
}

static void readsf_tilde_stop (t_readsf_tilde *x)
{
    readsf_tilde_close (x);
}

static void readsf_tilde_float (t_readsf_tilde *x, t_float f)
{
    if (f != 0.0) { readsf_tilde_start (x); }
    else {
        readsf_tilde_stop (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_int *readsf_tilde_perform (t_int *w)
{
    t_readsf_tilde *x = (t_readsf_tilde *)(w[1]);
    int n = (int)(w[2]);
    
    int numberOfFramesRead = 0;
    
    if (x->sf_run && x->sf_thread) {
    //
    int numberOfChannels = x->sf_properties.ap_numberOfChannels;
    int bytesPerFrame    = numberOfChannels * x->sf_properties.ap_bytesPerSample;
    int32_t required     = bytesPerFrame * n;
    
    /* Buffer on the stack in order to cache samples read. */
    /* Could be a problem in edge cases. */
    
    unsigned char *t = (unsigned char *)alloca (required);
    
    int32_t loaded = ringbuffer_read (sfthread_getBuffer (x->sf_thread), t, required);
    
    if (loaded == 0 && sfthread_isEnd (x->sf_thread)) { x->sf_run = 0; clock_delay (x->sf_clock, 0.0); }
    else {
        
        numberOfFramesRead = (int)(loaded / bytesPerFrame);
        
        soundfile_decode (numberOfChannels,
            x->sf_vectorsOut,
            t,
            numberOfFramesRead,
            0,
            x->sf_properties.ap_bytesPerSample,
            x->sf_properties.ap_isBigEndian,
            1,
            x->sf_numberOfAudioOutlets);
    }
    //
    }
    
    {
    //
    int i, j;
    
    for (i = 0; i < x->sf_numberOfAudioOutlets; i++) {
        PD_RESTRICTED out = x->sf_vectorsOut[i];
        for (j = numberOfFramesRead; j < n; j++) { out[j] = (t_sample)0.0; }
    }
    //
    }
    
    return (w + 3);
}

static void readsf_tilde_dsp (t_readsf_tilde *x, t_signal **sp)
{
    int i;
    
    for (i = 0; i < x->sf_numberOfAudioOutlets; i++) { x->sf_vectorsOut[i] = sp[i]->s_vector; }
    
    dsp_add (readsf_tilde_perform, 2, x, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void readsf_tilde_task (t_readsf_tilde *x)
{
    outlet_bang (x->sf_outletTopRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *readsf_tilde_new (t_float f1, t_float f2)
{
    int i, n = PD_CLAMP ((int)f1, 1, SOUNDFILE_CHANNELS);
    int size = PD_CLAMP ((int)f2, READSF_BUFFER_SIZE * 4 * n, READSF_BUFFER_SIZE * 256 * n);
    
    t_readsf_tilde *x = (t_readsf_tilde *)pd_new (readsf_tilde_class);
    
    soundfile_propertiesInit (&x->sf_properties);
    
    if (!PD_IS_POWER_2 (size)) { size = (int)PD_NEXT_POWER_2 (size); }
    
    x->sf_numberOfAudioOutlets  = n;
    x->sf_bufferSize            = size;
    x->sf_clock                 = clock_new ((void *)x, (t_method)readsf_tilde_task);
    x->sf_owner                 = instance_contextGetCurrent();
    
    for (i = 0; i < n; i++) { x->sf_audioOutlets[i] = outlet_new (cast_object (x), &s_signal); }
    
    x->sf_outletTopRight = outlet_new (cast_object (x), &s_bang);
    
    return x;
}

static void readsf_tilde_free (t_readsf_tilde *x)
{
    readsf_tilde_close (x);
    
    clock_free (x->sf_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void readsf_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_readsf__tilde__,
            (t_newmethod)readsf_tilde_new, 
            (t_method)readsf_tilde_free,
            sizeof (t_readsf_tilde),
            CLASS_DEFAULT, 
            A_DEFFLOAT, 
            A_DEFFLOAT,
            A_NULL);
    
    class_addDSP (c, (t_method)readsf_tilde_dsp);
    class_addFloat (c, (t_method)readsf_tilde_float);
    
    class_addMethod (c, (t_method)readsf_tilde_start,   sym_start,  A_NULL);
    class_addMethod (c, (t_method)readsf_tilde_stop,    sym_stop,   A_NULL);
    class_addMethod (c, (t_method)readsf_tilde_open,    sym_open,   A_GIMME, A_NULL);
    
    readsf_tilde_class = c;
}

void readsf_tilde_destroy (void)
{
    class_free (readsf_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
