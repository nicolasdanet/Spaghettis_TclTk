
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
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Resonant filter with audio-rate center frequency input. */

/* One-pole complex filter with coefficients built in. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_float *cos_tilde_table;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *vcf_tilde_class;                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vcf_tilde_ctl {
    t_float             c_real;
    t_float             c_imaginary;
    t_float             c_q;
    t_float             c_conversion;
    } t_vcf_tilde_ctl;

typedef struct _vcf_tilde {
    t_object            x_obj;              /* Must be the first. */
    t_float             x_f;
    t_vcf_tilde_ctl     x_space;
    t_outlet            *x_outletLeft;
    t_outlet            *x_outletRight;
    } t_vcf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vcf_tilde_qFactor (t_vcf_tilde *x, t_float f)
{
    x->x_space.c_q = PD_MAX (0.0, f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */
/* Notice that the two signals in could be theoretically just one. */
/* But as only loads are performed, it is assumed safe to use restricted pointers. */

static t_int *vcf_tilde_perform (t_int *w)
{
    t_vcf_tilde_ctl *c = (t_vcf_tilde_ctl *)(w[1]);
    PD_RESTRICTED in1  = (t_sample *)(w[2]);
    PD_RESTRICTED in2  = (t_sample *)(w[3]);
    PD_RESTRICTED out1 = (t_sample *)(w[4]);
    PD_RESTRICTED out2 = (t_sample *)(w[5]);
    int n = (t_int)(w[6]);
    
    t_float re = c->c_real;
    t_float im = c->c_imaginary;
    t_float q  = c->c_q;
    t_float k  = c->c_conversion;
    
    double qInverse = (q > 0.0 ? 1.0 / q : 0.0);
    double correction = 2.0 - (2.0 / (q + 2.0));

    while (n--) {
    //
    double centerFrequency;
    double r;
    double oneMinusR;
    double coefficientReal;
    double coefficientImaginary;
    
    centerFrequency = (*in2++) * k; 
    centerFrequency = PD_MAX (0.0, centerFrequency);
    
    r = (qInverse > 0.0 ? 1.0 - centerFrequency * qInverse : 0.0);
    r = PD_MAX (0.0, r);
    oneMinusR = 1.0 - r;
    
    coefficientReal      = r * dsp_getCosineAt (centerFrequency * (COSINE_TABLE_SIZE / PD_2PI));
    coefficientImaginary = r * dsp_getSineAt (centerFrequency * (COSINE_TABLE_SIZE / PD_2PI));
    
    {
        double s = *in1++;
        double tReal = re;
        double tImaginary = im;
        
        re = correction * oneMinusR * s + coefficientReal * tReal - coefficientImaginary * tImaginary;
        im = coefficientImaginary * tReal + coefficientReal * tImaginary;
        
        *out1++ = re;
        *out2++ = im;
    }
    //
    }
    
    if (PD_BIG_OR_SMALL (re)) { re = 0.0; }
    if (PD_BIG_OR_SMALL (im)) { im = 0.0; }
    
    c->c_real = re;
    c->c_imaginary = im;
    
    return (w + 7);
}

static void vcf_tilde_dsp (t_vcf_tilde *x, t_signal **sp)
{
    x->x_space.c_conversion = PD_2PI / sp[0]->s_sampleRate;
   
    PD_ASSERT (sp[0]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[0]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[2]->s_vector);
    PD_ASSERT (sp[1]->s_vector != sp[3]->s_vector);
    PD_ASSERT (sp[2]->s_vector != sp[3]->s_vector);
    
    dsp_add (vcf_tilde_perform, 6, &x->x_space,
        sp[0]->s_vector,
        sp[1]->s_vector,
        sp[2]->s_vector,
        sp[3]->s_vector, 
        sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vcf_tilde_new (t_float f)
{
    t_vcf_tilde *x = (t_vcf_tilde *)pd_new (vcf_tilde_class);

    x->x_space.c_real       = 0.0;
    x->x_space.c_imaginary  = 0.0;
    x->x_space.c_q          = f;
    x->x_space.c_conversion = 0.0;

    x->x_outletLeft  = outlet_new (cast_object (x), &s_signal);
    x->x_outletRight = outlet_new (cast_object (x), &s_signal);

    inlet_newSignal (cast_object (x));
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);

    return x;
}


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vcf_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_vcf__tilde__,
            (t_newmethod)vcf_tilde_new,
            NULL,
            sizeof (t_vcf_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_vcf_tilde, x_f);
    
    class_addDSP (c, vcf_tilde_dsp);
    
    class_addMethod (c, (t_method)vcf_tilde_qFactor, sym_inlet2, A_FLOAT, A_NULL);
        
    vcf_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
