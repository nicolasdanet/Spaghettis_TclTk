
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
#include "d_osc.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Resonant filter with audio-rate center frequency input. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_float *cos_tilde_table;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *vcf_tilde_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vcf_tilde_ctl {
    t_float             c_re;
    t_float             c_im;
    t_float             c_q;
    t_float             c_isr;
    } t_vcf_tilde_ctl;

typedef struct _vcf_tilde {
    t_object            x_obj;
    t_float             x_f;
    t_vcf_tilde_ctl     x_cspace;
    t_vcf_tilde_ctl     *x_ctl;
    } t_vcf_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vcf_tilde_qFactor (t_vcf_tilde *x, t_float f)
{
    x->x_ctl->c_q = (f > 0 ? f : 0.f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *vcf_tilde_perform(t_int *w)
{
    float *in1 = (float *)(w[1]);
    float *in2 = (float *)(w[2]);
    float *out1 = (float *)(w[3]);
    float *out2 = (float *)(w[4]);
    t_vcf_tilde_ctl *c = (t_vcf_tilde_ctl *)(w[5]);
    int n = (t_int)(w[6]);
    int i;
    float re = c->c_re, re2;
    float im = c->c_im;
    float q = c->c_q;
    float qinv = (q > 0? 1.0f/q : 0);
    float ampcorrect = 2.0f - 2.0f / (q + 2.0f);
    float isr = c->c_isr;
    float coefr, coefi;
    float *tab = cos_tilde_table, *addr, f1, f2, frac;
    double dphase;
    int normhipart, tabindex;
    t_rawcast64 tf;
    
    tf.z_d = DSP_UNITBIT;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];

    for (i = 0; i < n; i++)
    {
        float cf, cfindx, r, oneminusr;
        cf = *in2++ * isr;
        if (cf < 0) cf = 0;
        cfindx = cf * (float)(COSINE_TABLE_SIZE/6.28318f);
        r = (qinv > 0 ? 1 - cf * qinv : 0);
        if (r < 0) r = 0;
        oneminusr = 1.0f - r;
        dphase = ((double)(cfindx)) + DSP_UNITBIT;
        tf.z_d = dphase;
        tabindex = tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1);
        addr = tab + tabindex;
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT;
        f1 = addr[0];
        f2 = addr[1];
        coefr = r * (f1 + frac * (f2 - f1));

        addr = tab + ((tabindex - (COSINE_TABLE_SIZE/4)) & (COSINE_TABLE_SIZE-1));
        f1 = addr[0];
        f2 = addr[1];
        coefi = r * (f1 + frac * (f2 - f1));

        f1 = *in1++;
        re2 = re;
        *out1++ = re = ampcorrect * oneminusr * f1 
            + coefr * re2 - coefi * im;
        *out2++ = im = coefi * re2 + coefr * im;
    }
    if (PD_BIG_OR_SMALL(re))
        re = 0;
    if (PD_BIG_OR_SMALL(im))
        im = 0;
    c->c_re = re;
    c->c_im = im;
    return (w+7);
}

static void vcf_tilde_dsp(t_vcf_tilde *x, t_signal **sp)
{
    x->x_ctl->c_isr = 6.28318f/sp[0]->s_sampleRate;
    dsp_add(vcf_tilde_perform, 6,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[3]->s_vector, 
            x->x_ctl, sp[0]->s_vectorSize);

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vcf_tilde_new(t_float q)
{
    t_vcf_tilde *x = (t_vcf_tilde *)pd_new(vcf_tilde_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_re = 0;
    x->x_cspace.c_im = 0;
    x->x_cspace.c_q = q;
    x->x_cspace.c_isr = 0;
    x->x_f = 0;
    return (x);
}


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vcf_tilde_setup (void)
{
    vcf_tilde_class = class_new(sym_vcf__tilde__, (t_newmethod)vcf_tilde_new, 0,
        sizeof(t_vcf_tilde), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(vcf_tilde_class, t_vcf_tilde, x_f);
    class_addMethod(vcf_tilde_class, (t_method)vcf_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(vcf_tilde_class, (t_method)vcf_tilde_qFactor,
        sym_inlet2, A_FLOAT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
