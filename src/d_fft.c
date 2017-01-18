
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
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_fft_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *fft_tilde_perform (t_int *w)
{
    PD_RESTRICTED in1 = (t_sample *)(w[1]);
    PD_RESTRICTED in2 = (t_sample *)(w[2]);
    int n = w[3];
    
    mayer_fft (n, in1, in2);
    
    return (w + 4);
}

static void fft_tilde_dsp (t_fft_tilde *x, t_signal **sp)
{
    int n = sp[0]->s_vectorSize;
    
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[0]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[2]->s_vector != sp[3]->s_vector);
    
    dsp_addCopyPerform (sp[0]->s_vector, sp[2]->s_vector, n);
    dsp_addCopyPerform (sp[1]->s_vector, sp[3]->s_vector, n);
    
    dsp_add (fft_tilde_perform, 3, sp[2]->s_vector, sp[3]->s_vector, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *fft_tilde_new (void)
{
    t_fft_tilde *x = (t_fft_tilde *)pd_new (fft_tilde_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_signal);
    x->x_outletRight = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void fft_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_fft__tilde__,
            fft_tilde_new,
            NULL,
            sizeof (t_fft_tilde), 
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_fft_tilde, x_f);
    
    class_addDSP (c, (t_method)fft_tilde_dsp);
    
    fft_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
