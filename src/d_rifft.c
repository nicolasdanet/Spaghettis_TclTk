
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

static t_class *rifft_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _rifft_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_rifft_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *rifft_tilde_performFlip (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = w[3];
    
    while (n--) { t_sample f = *in; --out; *out = - f; in++; }
    
    return (w + 4);
}

static t_int *rifft_tilde_perform (t_int *w)
{
    PD_RESTRICTED in = (t_sample *)(w[1]);
    int n = w[2];
    
    fft_realInverseFFT (n, in);
    
    return (w + 3);
}

/* No aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are performed, it is assumed safe to use restricted pointers. */

static void rifft_tilde_dsp (t_rifft_tilde *x, t_signal **sp)
{
    int size = sp[0]->s_vectorSize;
    
    PD_ASSERT (PD_IS_POWER_2 (size));
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    
    if (size < 4) { error_invalid (sym_rifft__tilde__, sym_size); }
    else {
    //
    int half = (size >> 1);
    
    PD_RESTRICTED in1  = sp[0]->s_vector;
    PD_RESTRICTED in2  = sp[1]->s_vector;
    PD_RESTRICTED out1 = sp[2]->s_vector;

    dsp_addCopyPerform (in1, out1, half + 1);
    dsp_add (rifft_tilde_performFlip, 3, in2 + 1, out1 + size, half - 1);
    dsp_add (rifft_tilde_perform, 2, out1, size);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *rifft_tilde_new (void)
{
    t_rifft_tilde *x = (t_rifft_tilde *)pd_new (rifft_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rifft_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_rifft__tilde__,
            rifft_tilde_new,
            NULL,
            sizeof (t_rifft_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_rifft_tilde, x_f);
    
    class_addDSP (c, (t_method)rifft_tilde_dsp);
        
    class_setHelpName (c, sym_fft__tilde__);
    
    rifft_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
