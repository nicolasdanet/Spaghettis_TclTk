
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Complex one-pole filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/One_Pole.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *cpole_tilde_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _cpole_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_sample    x_real;
    t_sample    x_imaginary;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_cpole_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void cpole_tilde_set (t_cpole_tilde *x, t_float real, t_float imaginary)
{
    x->x_real       = real;
    x->x_imaginary  = imaginary;
}

static void cpole_tilde_clear (t_cpole_tilde *x)
{
    cpole_tilde_set (x, 0.0, 0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */
/* Notice that the fourth signals incoming could be theoretically just one. */
/* But as only loads are done, it is assumed safe to use restricted pointers. */

static t_int *cpole_tilde_perform (t_int *w)
{
    t_cpole_tilde *x   = (t_cpole_tilde *)(w[1]);
    PD_RESTRICTED in1  = (t_sample *)(w[2]);
    PD_RESTRICTED in2  = (t_sample *)(w[3]);
    PD_RESTRICTED in3  = (t_sample *)(w[4]);
    PD_RESTRICTED in4  = (t_sample *)(w[5]);
    PD_RESTRICTED out1 = (t_sample *)(w[6]);
    PD_RESTRICTED out2 = (t_sample *)(w[7]);
    int n = (int)(w[8]);

    t_sample lastReal      = x->x_real;
    t_sample lastImaginary = x->x_imaginary;
    
    while (n--) {

        t_sample aReal          = (*in3++);
        t_sample aImaginary     = (*in4++);
        t_sample realPart       = (*in1++) + lastReal * aReal - lastImaginary * aImaginary;
        t_sample imaginaryPart  = (*in2++) + lastReal * aImaginary + lastImaginary * aReal;
        
        *out1++ = lastReal      = realPart;
        *out2++ = lastImaginary = imaginaryPart;
    }
    
    if (PD_FLOAT32_IS_BIG_OR_SMALL ((float)lastReal))      { lastReal      = 0.0; }
    if (PD_FLOAT32_IS_BIG_OR_SMALL ((float)lastImaginary)) { lastImaginary = 0.0; }
    
    x->x_real      = lastReal;
    x->x_imaginary = lastImaginary;
    
    return (w + 9);
}

static void cpole_tilde_dsp (t_cpole_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[4]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[4]->s_vector);
    PD_ASSERT (sp[2]->s_vector != sp[4]->s_vector);
    PD_ASSERT (sp[3]->s_vector != sp[4]->s_vector);
    PD_ASSERT (sp[0]->s_vector != sp[5]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[5]->s_vector);
    PD_ASSERT (sp[2]->s_vector != sp[5]->s_vector);
    PD_ASSERT (sp[3]->s_vector != sp[5]->s_vector);
    PD_ASSERT (sp[4]->s_vector != sp[5]->s_vector);
        
    dsp_add (cpole_tilde_perform, 8, x,
        sp[0]->s_vector,
        sp[1]->s_vector,
        sp[2]->s_vector,
        sp[3]->s_vector, 
        sp[4]->s_vector,
        sp[5]->s_vector,
        sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *cpole_tilde_new (t_float real, t_float imaginary)
{
    t_cpole_tilde *x = (t_cpole_tilde *)pd_new (cpole_tilde_class);
    
    x->x_outletLeft  = outlet_newSignal (cast_object (x));
    x->x_outletRight = outlet_newSignal (cast_object (x));
    
    inlet_newSignal (cast_object (x));
    
    pd_float ((t_pd *)inlet_newSignal (cast_object (x)), real);
    pd_float ((t_pd *)inlet_newSignal (cast_object (x)), imaginary);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void cpole_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_cpole__tilde__,
            (t_newmethod)cpole_tilde_new,
            NULL, 
            sizeof (t_cpole_tilde),
            CLASS_DEFAULT, 
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_cpole_tilde, x_f);
    
    class_addDSP (c, (t_method)cpole_tilde_dsp);
        
    class_addMethod (c, (t_method)cpole_tilde_set,      sym_set,    A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)cpole_tilde_clear,    sym_clear,  A_NULL);
    
    cpole_tilde_class = c;
}

void cpole_tilde_destroy (void)
{
    class_free (cpole_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
