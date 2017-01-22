
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *pow_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pow_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_pow_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are performed, it is assumed safe to use restricted pointers. */

t_int *pow_tilde_perform (t_int *w)
{
    PD_RESTRICTED in1 = (t_sample *)(w[1]);
    PD_RESTRICTED in2 = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f = *in1++;
    t_sample g = *in2++;

    if (f > 0.0) { *out++ = (t_sample)pow (f, g); }
    else {
        *out++ = 0.0;
    }
    //
    }
    
    return (w + 5);
}

static void pow_tilde_dsp (t_pow_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    
    dsp_add (pow_tilde_perform, 4, sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *pow_tilde_new (t_float f)
{
    t_pow_tilde *x = (t_pow_tilde *)pd_new (pow_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignalDefault (cast_object (x), f);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pow_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pow__tilde__,
            (t_newmethod)pow_tilde_new,
            NULL,
            sizeof (t_pow_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_pow_tilde, x_f);
    
    class_addDSP (c, (t_method)pow_tilde_dsp);
    
    pow_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
