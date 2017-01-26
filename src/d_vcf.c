
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
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Complex one-pole resonant filter (with audio-rate center frequency input). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/Complex_Resonator.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *vcf_tilde_class;                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vcf_tilde_control {
    t_sample            c_real;
    t_sample            c_imaginary;
    t_sample            c_q;
    t_sample            c_conversion;
    } t_vcf_tilde_control;

typedef struct _vcf_tilde {
    t_object            x_obj;              /* Must be the first. */
    t_float             x_f;
    t_vcf_tilde_control x_space;
    t_outlet            *x_outletLeft;
    t_outlet            *x_outletRight;
    } t_vcf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vcf_tilde_qFactor (t_vcf_tilde *x, t_float f)
{
    x->x_space.c_q = (t_sample)PD_MAX (0.0, f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */
/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are done, it is assumed safe to use restricted pointers. */

static t_int *vcf_tilde_perform (t_int *w)
{
    t_vcf_tilde_control *c = (t_vcf_tilde_control *)(w[1]);
    PD_RESTRICTED in1  = (t_sample *)(w[2]);
    PD_RESTRICTED in2  = (t_sample *)(w[3]);
    PD_RESTRICTED out1 = (t_sample *)(w[4]);
    PD_RESTRICTED out2 = (t_sample *)(w[5]);
    int n = (int)(w[6]);
    
    t_sample re = c->c_real;
    t_sample im = c->c_imaginary;
    t_sample q  = c->c_q;
    t_sample k  = c->c_conversion;
    
    double qInverse = (q > 0.0 ? 1.0 / q : 0.0);
    double correction = 2.0 - (2.0 / (q + 2.0));

    while (n--) {
    //
    double centerFrequency;
    double r, g;
    double pReal;
    double pImaginary;
    
    centerFrequency = (*in2++) * k; 
    centerFrequency = PD_MAX (0.0, centerFrequency);
    
    r = (qInverse > 0.0 ? 1.0 - centerFrequency * qInverse : 0.0);
    r = PD_MAX (0.0, r);
    g = correction * (1.0 - r);
    
    pReal      = r * dsp_getCosineAt (centerFrequency * (COSINE_TABLE_SIZE / PD_TWO_PI));
    pImaginary = r * dsp_getSineAt   (centerFrequency * (COSINE_TABLE_SIZE / PD_TWO_PI));
    
    {
        double s = (*in1++);
        double tReal = re;
        double tImaginary = im;
        
        re = (t_sample)((g * s) + (pReal * tReal - pImaginary * tImaginary));
        im = (t_sample)((pImaginary * tReal + pReal * tImaginary));
        
        *out1++ = re;
        *out2++ = im;
    }
    //
    }
    
    if (PD_IS_BIG_OR_SMALL (re)) { re = (t_sample)0.0; }
    if (PD_IS_BIG_OR_SMALL (im)) { im = (t_sample)0.0; }
    
    c->c_real = re;
    c->c_imaginary = im;
    
    return (w + 7);
}

static void vcf_tilde_dsp (t_vcf_tilde *x, t_signal **sp)
{
    x->x_space.c_conversion = (t_sample)(PD_TWO_PI / sp[0]->s_sampleRate);
   
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

    x->x_space.c_real       = (t_sample)0.0;
    x->x_space.c_imaginary  = (t_sample)0.0;
    x->x_space.c_q          = (t_sample)f;
    x->x_space.c_conversion = (t_sample)0.0;

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
    
    class_addDSP (c, (t_method)vcf_tilde_dsp);
    
    class_addMethod (c, (t_method)vcf_tilde_qFactor, sym_inlet2, A_FLOAT, A_NULL);
        
    vcf_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
