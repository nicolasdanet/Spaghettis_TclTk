
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

static t_class *writesf_tilde_class;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _writesf_tilde {
    t_object            sf_obj;                     /* Must be the first. */
    t_float             sf_f;
    t_audioproperties   sf_properties;
    int                 sf_numberOfChannels;
    int                 sf_bufferSize;
    int                 sf_run;
    t_sfthread          *sf_thread;
    t_glist             *sf_owner;
    t_sample            *(sf_vectorsIn[SOUNDFILE_CHANNELS]);
    } t_writesf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define WRITESF_BUFFER_SIZE     65536               /* Power of two. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void writesf_tilde_close (t_writesf_tilde *x)
{
    if (x->sf_thread) { sfthread_release (x->sf_thread); x->sf_thread = NULL; }
    
    x->sf_run = 0;
    
    soundfile_propertiesInit (&x->sf_properties);
}

static void writesf_tilde_open (t_writesf_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    writesf_tilde_close (x);
    
    {
    //
    t_audioproperties *p = &x->sf_properties;
    
    t_error err = soundfile_writeFileParse (sym_writesf__tilde__, &argc, &argv, p);
    
    p->ap_numberOfChannels = x->sf_numberOfChannels;
    
    if (!err) {
    //
    int f = soundfile_writeFileHeader (x->sf_owner, &x->sf_properties);
    
    err = (f < 0);
    if (!err) {
        x->sf_thread = sfthread_new (SFTHREAD_WRITER, x->sf_bufferSize, f, &x->sf_properties);
        PD_ASSERT (x->sf_thread);
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

static void writesf_tilde_start (t_writesf_tilde *x)
{
    if (!x->sf_thread) { error_unexpected (sym_writesf__tilde__, sym_start); }
    else {
        x->sf_run = 1;
    }
}

static void writesf_tilde_stop (t_writesf_tilde *x)
{
    writesf_tilde_close (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_int *writesf_tilde_perform (t_int *w)
{
    t_writesf_tilde *x = (t_writesf_tilde *)(w[1]);
    int n = (int)(w[2]);
    
    if (x->sf_run && x->sf_thread) {
    //
    int numberOfChannels = x->sf_numberOfChannels;
    int bytesPerFrame    = numberOfChannels * x->sf_properties.ap_bytesPerSample;
    int32_t required     = bytesPerFrame * n;
    
    /* Buffer on the stack in order to cache samples read. */
    /* Could be a problem in edge cases. */
    
    unsigned char *t = (unsigned char *)alloca (required);
    
    soundfile_encode (numberOfChannels,
        x->sf_vectorsIn,
        t,
        n,
        0,
        x->sf_properties.ap_bytesPerSample,
        x->sf_properties.ap_isBigEndian,
        1,
        (t_sample)1.0);
    
    {
        int32_t written = ringbuffer_write (sfthread_getBuffer (x->sf_thread), t, required);
        if (written != required) {
            /* File corrupted; what to do? */
        }
    }
    //
    }
    
    return (w + 3);
}

static void writesf_tilde_dsp (t_writesf_tilde *x, t_signal **sp)
{
    int i;
    
    for (i = 0; i < x->sf_numberOfChannels; i++) { x->sf_vectorsIn[i] = sp[i]->s_vector; }

    dsp_add (writesf_tilde_perform, 2, x, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *writesf_tilde_new (t_float f1, t_float f2)
{
    int i, n = PD_CLAMP ((int)f1, 1, SOUNDFILE_CHANNELS);
    int size = PD_CLAMP ((int)f2, WRITESF_BUFFER_SIZE * 4 * n, WRITESF_BUFFER_SIZE * 256 * n);

    t_writesf_tilde *x = (t_writesf_tilde *)pd_new (writesf_tilde_class);
    
    soundfile_propertiesInit (&x->sf_properties);
    
    if (!PD_IS_POWER_2 (size)) { size = (int)PD_NEXT_POWER_2 (size); }
    
    x->sf_numberOfChannels  = n;
    x->sf_bufferSize        = size;
    x->sf_owner             = instance_contextGetCurrent();
    
    for (i = 1; i < x->sf_numberOfChannels; i++) { inlet_newSignal (cast_object (x)); }

    return x;
}

static void writesf_tilde_free (t_writesf_tilde *x)
{
    writesf_tilde_close (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void writesf_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_writesf__tilde__,
            (t_newmethod)writesf_tilde_new, 
            (t_method)writesf_tilde_free,
            sizeof (t_writesf_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
        
    CLASS_SIGNAL (c, t_writesf_tilde, sf_f);
    
    class_addDSP (c, (t_method)writesf_tilde_dsp);
        
    class_addMethod (c, (t_method)writesf_tilde_start,  sym_start,  A_NULL);
    class_addMethod (c, (t_method)writesf_tilde_stop,   sym_stop,   A_NULL);
    class_addMethod (c, (t_method)writesf_tilde_open,   sym_open,   A_GIMME, A_NULL);

    writesf_tilde_class = c;
}

void writesf_tilde_destroy (void)
{
    class_free (writesf_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
