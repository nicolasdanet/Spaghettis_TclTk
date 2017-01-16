
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Real one-pole filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/One_Pole.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *rpole_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _rpole_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_sample    x_real;
    t_outlet    *x_outlet;
    } t_rpole_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void rpole_tilde_clear (t_rpole_tilde *x)
{
    x->x_real = 0;
}

static void rpole_tilde_set (t_rpole_tilde *x, t_float f)
{
    x->x_real = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are performed, it is assumed safe to use restricted pointers. */

static t_int *rpole_tilde_perform (t_int *w)
{
    t_rpole_tilde *x  = (t_rpole_tilde *)(w[1]);
    PD_RESTRICTED in1 = (t_sample *)(w[2]);
    PD_RESTRICTED in2 = (t_sample *)(w[3]);
    PD_RESTRICTED out = (t_sample *)(w[4]);
    int n = (t_int)(w[5]);
    
    t_sample last = x->x_real;
    
    while (n--) {
        t_sample a = (*in2++);
        t_sample f = (*in1++) + a * last;
        *out++ = last = f;
    }
    
    if (PD_IS_BIG_OR_SMALL (last)) { last = 0.0; }
        
    x->x_real = last;
    
    return (w + 6);
}

static void rpole_tilde_dsp (t_rpole_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    
    dsp_add (rpole_tilde_perform, 5, x,
        sp[0]->s_vector,
        sp[1]->s_vector,
        sp[2]->s_vector,
        sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *rpole_tilde_new (t_float f)
{
    t_rpole_tilde *x = (t_rpole_tilde *)pd_new (rpole_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    pd_float ((t_pd *)inlet_newSignal (cast_object (x)), f);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rpole_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_rpole__tilde__,
            (t_newmethod)rpole_tilde_new,
            NULL,
            sizeof (t_rpole_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_rpole_tilde, x_f);
    
    class_addDSP (c, (t_method)rpole_tilde_dsp);
        
    class_addMethod (c, (t_method)rpole_tilde_set,      sym_set,    A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)rpole_tilde_clear,    sym_clear,  A_NULL);
        
    rpole_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
