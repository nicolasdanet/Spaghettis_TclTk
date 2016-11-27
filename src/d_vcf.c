/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* sinusoidal oscillator and table lookup; see also tabosc4~ in d_array.c.
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "math.h"
#include "d_dsp.h"
#include "d_osc.h"

extern t_float *cos_tilde_table;

/* ---- vcf~ - resonant filter with audio-rate center frequency input ----- */

typedef struct vcfctl
{
    float c_re;
    float c_im;
    float c_q;
    float c_isr;
} t_vcfctl;

typedef struct sigvcf
{
    t_object x_obj;
    t_vcfctl x_cspace;
    t_vcfctl *x_ctl;
    float x_f;
} t_sigvcf;

t_class *sigvcf_class;

static void *sigvcf_new(t_float q)
{
    t_sigvcf *x = (t_sigvcf *)pd_new(sigvcf_class);
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

static void sigvcf_ft1(t_sigvcf *x, t_float f)
{
    x->x_ctl->c_q = (f > 0 ? f : 0.f);
}

static t_int *sigvcf_perform(t_int *w)
{
    float *in1 = (float *)(w[1]);
    float *in2 = (float *)(w[2]);
    float *out1 = (float *)(w[3]);
    float *out2 = (float *)(w[4]);
    t_vcfctl *c = (t_vcfctl *)(w[5]);
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

static void sigvcf_dsp(t_sigvcf *x, t_signal **sp)
{
    x->x_ctl->c_isr = 6.28318f/sp[0]->s_sampleRate;
    dsp_add(sigvcf_perform, 6,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[3]->s_vector, 
            x->x_ctl, sp[0]->s_vectorSize);

}

void sigvcf_setup(void)
{
    sigvcf_class = class_new(sym_vcf__tilde__, (t_newmethod)sigvcf_new, 0,
        sizeof(t_sigvcf), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigvcf_class, t_sigvcf, x_f);
    class_addMethod(sigvcf_class, (t_method)sigvcf_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(sigvcf_class, (t_method)sigvcf_ft1,
        sym_inlet2, A_FLOAT, 0);
}
