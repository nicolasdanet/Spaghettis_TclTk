
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

static t_class *samphold_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _samphold_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_sample    x_lastControl;
    t_sample    x_lastOut;
    t_outlet    *x_outlet;
    } t_samphold_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void samphold_tilde_reset (t_samphold_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_FLOAT (argv)) { x->x_lastControl = GET_FLOAT (argv); }
    else {
        x->x_lastControl = PD_FLT_MAX;
    }
}

static void samphold_tilde_set (t_samphold_tilde *x, t_float f)
{
    x->x_lastOut = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Aliasing. */

static t_int *samphold_tilde_perform (t_int *w)
{
    t_samphold_tilde *x = (t_samphold_tilde *)(w[1]);
    t_sample *in1 = (t_sample *)(w[2]);
    t_sample *in2 = (t_sample *)(w[3]);
    t_sample *out = (t_sample *)(w[4]);
    int n = (t_int)(w[5]);
    
    t_sample lastControl = x->x_lastControl;
    t_sample lastOut = x->x_lastOut;
    int i;
        
    for (i = 0; i < n; i++) {
    //
    t_sample f = *in2;
    
    if (f < lastControl) { lastOut = *in1; }
    *out++ = lastOut;
    lastControl = f;
    
    in1++;
    in2++;
    //
    }
    
    x->x_lastControl = lastControl;
    x->x_lastOut = lastOut;
    
    return (w + 6);
}

static void samphold_tilde_dsp (t_samphold_tilde *x, t_signal **sp)
{
    dsp_add (samphold_tilde_perform, 5, x,
        sp[0]->s_vector,
        sp[1]->s_vector,
        sp[2]->s_vector, 
        sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *samphold_tilde_new (void)
{
    t_samphold_tilde *x = (t_samphold_tilde *)pd_new (samphold_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
        
    inlet_newSignal (cast_object (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void samphold_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_samphold__tilde__,
            (t_newmethod)samphold_tilde_new,
            NULL,
            sizeof (t_samphold_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_samphold_tilde, x_f);
    
    class_addDSP (c, (t_method)samphold_tilde_dsp);
        
    class_addMethod (c, (t_method)samphold_tilde_reset, sym_reset,  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)samphold_tilde_set,   sym_set,    A_DEFFLOAT, A_NULL);

    samphold_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
