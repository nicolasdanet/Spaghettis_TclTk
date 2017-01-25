
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
#include "d_math.h"
#include "d_fft.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *framp_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _framp_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_framp_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are performed, it is assumed safe to use restricted pointers. */

static t_int *framp_tilde_perform (t_int *w)
{
    PD_RESTRICTED in1  = (t_sample *)(w[1]);
    PD_RESTRICTED in2  = (t_sample *)(w[2]);
    PD_RESTRICTED out1 = (t_sample *)(w[3]);
    PD_RESTRICTED out2 = (t_sample *)(w[4]);
    int n = (int)w[5];
    
    double realLast         = 0.0;
    double realCurrent      = in1[0];
    double realNext         = in1[1];
    double imaginaryLast    = 0.0;
    double imaginaryCurrent = in2[0];
    double imaginaryNext    = in2[1];
    
    double frequency = 1.0;
    double k = 1.0 / (t_sample)(n * n);
    
    in1 += 2;
    in2 += 2;
    
    *out1++ = (t_sample)0.0;
    *out2++ = (t_sample)0.0;

    n -= 2;
    
    while (n--) {
    //
    realLast            = realCurrent;
    realCurrent         = realNext;
    realNext            = *in1++;
    imaginaryLast       = imaginaryCurrent;
    imaginaryCurrent    = imaginaryNext;
    imaginaryNext       = *in2++;
    
    { 
        double real      = realCurrent - 0.5 * (realLast + realNext);
        double imaginary = imaginaryCurrent - 0.5 * (imaginaryLast + imaginaryNext);
        double power     = real * real + imaginary * imaginary;
        double f;
        
        if (power > 1e-19) {
        
            double deltaReal      = realLast - realNext;
            double deltaImaginary = imaginaryLast - imaginaryNext;
            double detune         = (deltaReal * real + deltaImaginary * imaginary) / (2.0 * power);
            
            if (detune > 2.0 || detune < -2.0) { f = power = 0.0; }
            else {
                f = frequency + detune;
            }
            
        } else {
            f = power = 0.0;
        }
        
        *out1++ = (t_sample)(f);
        *out2++ = (t_sample)sqrt_fast ((t_float)(k * power));
        frequency += 1.0;
    }
    //
    }
    
    return (w + 6);
}

static void framp_tilde_dsp (t_framp_tilde *x, t_signal **sp)
{
    int n = sp[0]->s_vectorSize;
    
    PD_ASSERT (PD_IS_POWER_2 (n));
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[0]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[2]->s_vector != sp[3]->s_vector);
        
    if (n < FFT_MINIMUM || n > FFT_MAXIMUM) { error_invalid (sym_framp__tilde__, sym_size); }
    else {
    //
    int half = (n >> 1);
    
    dsp_add (framp_tilde_perform, 5,
        sp[0]->s_vector,
        sp[1]->s_vector,
        sp[2]->s_vector,
        sp[3]->s_vector,
        half);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *framp_tilde_new (void)
{
    t_framp_tilde *x = (t_framp_tilde *)pd_new (framp_tilde_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_signal);
    x->x_outletRight = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void framp_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_framp__tilde__,
            framp_tilde_new,
            NULL,
            sizeof (t_framp_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_framp_tilde, x_f);
    
    class_addDSP (c, (t_method)framp_tilde_dsp);
        
    framp_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
