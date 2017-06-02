
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Real one-zero filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/One_Zero.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *rzero_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _rzero_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_sample    x_real;
    t_outlet    *x_outlet;
    } t_rzero_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void rzero_tilde_set (t_rzero_tilde *x, t_float f)
{
    x->x_real = f;
}

static void rzero_tilde_clear (t_rzero_tilde *x)
{
    rzero_tilde_set (x, (t_float)0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are done, it is assumed safe to use restricted pointers. */

static t_int *rzero_tilde_perform (t_int *w)
{
    t_rzero_tilde *x  = (t_rzero_tilde *)(w[1]);
    PD_RESTRICTED in1 = (t_sample *)(w[2]);
    PD_RESTRICTED in2 = (t_sample *)(w[3]);
    PD_RESTRICTED out = (t_sample *)(w[4]);
    int n = (int)(w[5]);
    
    t_sample last = x->x_real;
    
    while (n--) {
        t_sample f = *in1++;
        t_sample b = *in2++;
        *out++ = f - b * last; 
        last   = f;
    }
    
    x->x_real = last;
    
    return (w + 6);
}

static void rzero_tilde_dsp (t_rzero_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    
    dsp_add (rzero_tilde_perform, 5, x,
        sp[0]->s_vector,
        sp[1]->s_vector,
        sp[2]->s_vector, 
        sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *rzero_tilde_new (t_float f)
{
    t_rzero_tilde *x = (t_rzero_tilde *)pd_new (rzero_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    pd_float ((t_pd *)inlet_newSignal (cast_object (x)), f);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void zero_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_rzero__tilde__,
            (t_newmethod)rzero_tilde_new,
            NULL,
            sizeof (t_rzero_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
        
    CLASS_SIGNAL (c, t_rzero_tilde, x_f);
    
    class_addDSP (c, (t_method)rzero_tilde_dsp);
        
    class_addMethod (c, (t_method)rzero_tilde_set,      sym_set,    A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)rzero_tilde_clear,    sym_clear,  A_NULL);
    
    rzero_tilde_class = c;
}

void zero_tilde_destroy (void)
{
    CLASS_FREE (rzero_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
