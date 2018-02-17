
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"
#include "d_fft.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *fft_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _fft_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_FFTState  x_state;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_fft_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *fft_tilde_perform (t_int *w)
{
    t_FFTState *x = (t_FFTState *)(w[1]);
    PD_RESTRICTED in1 = (t_sample *)(w[2]);
    PD_RESTRICTED in2 = (t_sample *)(w[3]);
    int n = (int)w[4];
    
    fft_complexFFT (x, n, in1, in2);
    
    return (w + 5);
}

static void fft_tilde_dsp (t_fft_tilde *x, t_signal **sp)
{
    int n = sp[0]->s_vectorSize;
    
    if (n < FFT_MINIMUM || n > FFT_MAXIMUM) { error_invalid (sym_fft__tilde__, sym_size); }
    else {
    //
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[0]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[2]->s_vector != sp[3]->s_vector);
    
    fft_setSize (n);
    fft_stateInitialize (&x->x_state, n);
    
    dsp_addCopyPerform (sp[0]->s_vector, sp[2]->s_vector, n);
    dsp_addCopyPerform (sp[1]->s_vector, sp[3]->s_vector, n);
    
    dsp_add (fft_tilde_perform, 4, &x->x_state, sp[2]->s_vector, sp[3]->s_vector, n);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *fft_tilde_new (void)
{
    t_fft_tilde *x = (t_fft_tilde *)pd_new (fft_tilde_class);
    
    x->x_outletLeft  = outlet_newSignal (cast_object (x));
    x->x_outletRight = outlet_newSignal (cast_object (x));
    
    inlet_newSignal (cast_object (x));

    return x;
}

static void fft_tilde_free (t_fft_tilde *x)
{
    fft_stateRelease (&x->x_state);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void fft_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_fft__tilde__,
            (t_newmethod)fft_tilde_new,
            (t_method)fft_tilde_free,
            sizeof (t_fft_tilde), 
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_fft_tilde, x_f);
    
    class_addDSP (c, (t_method)fft_tilde_dsp);
    
    fft_tilde_class = c;
}

void fft_tilde_destroy (void)
{
    class_free (fft_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
