
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

/* Complex one-zero filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/One_Zero.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *czero_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _czero_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_sample    x_real;
    t_sample    x_imaginary;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_czero_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void czero_tilde_set (t_czero_tilde *x, t_float real, t_float imaginary)
{
    x->x_real       = real;
    x->x_imaginary  = imaginary;
}

static void czero_tilde_clear (t_czero_tilde *x)
{
    czero_tilde_set (x, 0.0, 0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */
/* Notice that the fourth signals incoming could be theoretically just one. */
/* But as only loads are performed, it is assumed safe to use restricted pointers. */

static t_int *czero_tilde_perform (t_int *w)
{
    t_czero_tilde *x   = (t_czero_tilde *)(w[1]);
    PD_RESTRICTED in1  = (t_sample *)(w[2]);
    PD_RESTRICTED in2  = (t_sample *)(w[3]);
    PD_RESTRICTED in3  = (t_sample *)(w[4]);
    PD_RESTRICTED in4  = (t_sample *)(w[5]);
    PD_RESTRICTED out1 = (t_sample *)(w[6]);
    PD_RESTRICTED out2 = (t_sample *)(w[7]);
    int n = (t_int)(w[8]);
    
    t_sample lastReal      = x->x_real;
    t_sample lastImaginary = x->x_imaginary;
    
    while (n--) {
    
        t_sample real       = *in1++;
        t_sample imaginary  = *in2++;
        t_sample bReal      = *in3++;
        t_sample bImaginary = *in4++;
        
        *out1++ = real - lastReal * bReal + lastImaginary * bImaginary;
        *out2++ = imaginary - lastReal * bImaginary - lastImaginary * bReal;
        
        lastReal      = real;
        lastImaginary = imaginary;
    }
    
    x->x_real      = lastReal;
    x->x_imaginary = lastImaginary;
    
    return (w + 9);
}

static void czero_tilde_dsp (t_czero_tilde *x, t_signal **sp)
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
    
    dsp_add (czero_tilde_perform, 8, x,
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
#pragma mark -

static void *czero_tilde_new (t_float real, t_float imaginary)
{
    t_czero_tilde *x = (t_czero_tilde *)pd_new (czero_tilde_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_signal);
    x->x_outletRight = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));
    
    pd_float ((t_pd *)inlet_newSignal (cast_object (x)), real);
    pd_float ((t_pd *)inlet_newSignal (cast_object (x)), imaginary);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void czero_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_czero__tilde__,
            (t_newmethod)czero_tilde_new,
            NULL,
            sizeof (t_czero_tilde),
            CLASS_DEFAULT, 
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_czero_tilde, x_f);
    
    class_addDSP (c, (t_method)czero_tilde_dsp);
        
    class_addMethod (c, (t_method)czero_tilde_set,      sym_set,    A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)czero_tilde_clear,    sym_clear,  A_NULL);
        
    czero_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
