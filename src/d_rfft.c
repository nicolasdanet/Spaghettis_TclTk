
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
#include "d_dsp.h"
#include "d_fft.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *rfft_tilde_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _rfft_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_rfft_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *rfft_tilde_performFlipZero (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)w[3];
    
    while (n--) { t_sample f = *in; --out; *out = - f; *in = (t_sample)0.0; in++; }
        
    return (w + 4);
}

/* No aliasing. */

static t_int *rfft_tilde_perform (t_int *w)
{
    PD_RESTRICTED in = (t_sample *)(w[1]);
    int n = (int)w[2];
    
    fft_realFFT (n, in);
    
    return (w + 3);
}

static void rfft_tilde_dsp (t_rfft_tilde *x, t_signal **sp)
{
    int n = sp[0]->s_vectorSize;
    
    PD_ASSERT (PD_IS_POWER_2 (n));
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    
    if (n < FFT_MINIMUM || n > FFT_MAXIMUM) { error_invalid (sym_rfft__tilde__, sym_size); }
    else {
    //
    PD_RESTRICTED in1  = sp[0]->s_vector;
    PD_RESTRICTED out1 = sp[1]->s_vector;
    PD_RESTRICTED out2 = sp[2]->s_vector;
    
    int half = (n >> 1);
    
    fft_setSize (n);
    
    dsp_addCopyPerform (in1, out1, n);
    
    dsp_add (rfft_tilde_perform, 2, out1, n);
    dsp_add (rfft_tilde_performFlipZero, 3, out1 + half + 1, out2 + half, half - 1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *rfft_tilde_new (void)
{
    t_rfft_tilde *x = (t_rfft_tilde *)pd_new (rfft_tilde_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_signal);
    x->x_outletRight = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rfft_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_rfft__tilde__,
            rfft_tilde_new,
            NULL,
            sizeof (t_rfft_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_rfft_tilde, x_f);
    
    class_addDSP (c, (t_method)rfft_tilde_dsp);
    
    class_setHelpName (c, sym_fft__tilde__);
    
    rfft_tilde_class = c;
}

void rfft_tilde_destroy (void)
{
    CLASS_FREE (rfft_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
