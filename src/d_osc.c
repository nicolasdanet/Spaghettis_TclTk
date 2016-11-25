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

#define LOGCOSTABSIZE       9
#define COSTABSIZE          (1 << LOGCOSTABSIZE)

/* -------------------------- phasor~ ------------------------------ */
static t_class *phasor_class;

#if 1   /* in the style of R. Hoeldrich (ICMC 1995 Banff) */

typedef struct _phasor
{
    t_object x_obj;
    double x_phase;
    float x_conv;
    float x_f;      /* scalar frequency */
} t_phasor;

static void *phasor_new(t_float f)
{
    t_phasor *x = (t_phasor *)pd_new(phasor_class);
    x->x_f = f;
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    x->x_phase = 0;
    x->x_conv = 0;
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *phasor_perform(t_int *w)
{
    t_phasor *x = (t_phasor *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    double dphase = x->x_phase + (double)DSP_UNITBIT32;
    t_rawcast64 tf;
    int normhipart;
    float conv = x->x_conv;

    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];
    tf.z_d = dphase;

    while (n--)
    {
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        dphase += *in++ * conv;
        *out++ = tf.z_d - DSP_UNITBIT32;
        tf.z_d = dphase;
    }
    tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    x->x_phase = tf.z_d - DSP_UNITBIT32;
    return (w+5);
}

static void phasor_dsp(t_phasor *x, t_signal **sp)
{
    x->x_conv = 1./sp[0]->s_sampleRate;
    dsp_add(phasor_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

static void phasor_ft1(t_phasor *x, t_float f)
{
    x->x_phase = f;
}

void phasor_setup(void)
{
    phasor_class = class_new(sym_phasor__tilde__, (t_newmethod)phasor_new, 0,
        sizeof(t_phasor), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(phasor_class, t_phasor, x_f);
    class_addMethod(phasor_class, (t_method)phasor_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(phasor_class, (t_method)phasor_ft1,
        sym_inlet2, A_FLOAT, 0);
}

#endif  /* Hoeldrich version */

/* ------------------------ cos~ ----------------------------- */

static float *cos_tilde_table;

static t_class *cos_tilde_class;

typedef struct _cos
{
    t_object x_obj;
    float x_f;
} t_cos;

static void *cos_tilde_new(void)
{
    t_cos *x = (t_cos *)pd_new(cos_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *cos_tilde_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    float *tab = cos_tilde_table, *addr, f1, f2, frac;
    double dphase;
    int normhipart;
    t_rawcast64 tf;
    
    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];

#if 0           /* this is the readable version of the code. */
    while (n--)
    {
        dphase = (double)(*in++ * (float)(COSTABSIZE)) + DSP_UNITBIT32;
        tf.z_d = dphase;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSTABSIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        *out++ = f1 + frac * (f2 - f1);
    }
#endif
#if 1           /* this is the same, unwrapped by hand. */
        dphase = (double)(*in++ * (float)(COSTABSIZE)) + DSP_UNITBIT32;
        tf.z_d = dphase;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSTABSIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    while (--n)
    {
        dphase = (double)(*in++ * (float)(COSTABSIZE)) + DSP_UNITBIT32;
            frac = tf.z_d - DSP_UNITBIT32;
        tf.z_d = dphase;
            f1 = addr[0];
            f2 = addr[1];
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSTABSIZE-1));
            *out++ = f1 + frac * (f2 - f1);
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    }
            frac = tf.z_d - DSP_UNITBIT32;
            f1 = addr[0];
            f2 = addr[1];
            *out++ = f1 + frac * (f2 - f1);
#endif
    return (w+4);
}

static void cos_tilde_dsp(t_cos *x, t_signal **sp)
{
    dsp_add(cos_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

static void cos_tilde_maketable(void)
{
    int i;
    float *fp, phase, phsinc = (2. * PD_PI) / COSTABSIZE;
    t_rawcast64 tf;
    
    if (cos_tilde_table) return;
    cos_tilde_table = (float *)PD_MEMORY_GET(sizeof(float) * (COSTABSIZE+1));
    for (i = COSTABSIZE + 1, fp = cos_tilde_table, phase = 0; i--;
        fp++, phase += phsinc)
            *fp = cos(phase);

        /* here we check at startup whether the byte alignment
            is as we declared it.  If not, the code has to be
            recompiled the other way. */
    tf.z_d = DSP_UNITBIT32 + 0.5;
    if ((unsigned)tf.z_i[PD_RAWCAST64_LSB] != 0x80000000) { PD_BUG; }
}

void cos_tilde_setup(void)
{
    cos_tilde_class = class_new(sym_cos__tilde__, (t_newmethod)cos_tilde_new, 0,
        sizeof(t_cos), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(cos_tilde_class, t_cos, x_f);
    class_addMethod(cos_tilde_class, (t_method)cos_tilde_dsp, sym_dsp, A_CANT, 0);
    cos_tilde_maketable();
}

/* ------------------------ osc~ ----------------------------- */

static t_class *osc_class, *scalarosc_class;

typedef struct _osc
{
    t_object x_obj;
    double x_phase;
    float x_conv;
    float x_f;      /* frequency if scalar */
} t_osc;

static void *osc_new(t_float f)
{
    t_osc *x = (t_osc *)pd_new(osc_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    x->x_phase = 0;
    x->x_conv = 0;
    return (x);
}

static t_int *osc_perform(t_int *w)
{
    t_osc *x = (t_osc *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    float *tab = cos_tilde_table, *addr, f1, f2, frac;
    double dphase = x->x_phase + DSP_UNITBIT32;
    int normhipart;
    t_rawcast64 tf;
    float conv = x->x_conv;
    
    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];
#if 0
    while (n--)
    {
        tf.z_d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSTABSIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        *out++ = f1 + frac * (f2 - f1);
    }
#endif
#if 1
        tf.z_d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSTABSIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
    while (--n)
    {
        tf.z_d = dphase;
            f1 = addr[0];
        dphase += *in++ * conv;
            f2 = addr[1];
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSTABSIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
            *out++ = f1 + frac * (f2 - f1);
        frac = tf.z_d - DSP_UNITBIT32;
    }
            f1 = addr[0];
            f2 = addr[1];
            *out++ = f1 + frac * (f2 - f1);
#endif

    tf.z_d = DSP_UNITBIT32 * COSTABSIZE;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];
    tf.z_d = dphase + (DSP_UNITBIT32 * COSTABSIZE - DSP_UNITBIT32);
    tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    x->x_phase = tf.z_d - DSP_UNITBIT32 * COSTABSIZE;
    return (w+5);
}

static void osc_dsp(t_osc *x, t_signal **sp)
{
    x->x_conv = COSTABSIZE/sp[0]->s_sampleRate;
    dsp_add(osc_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

static void osc_ft1(t_osc *x, t_float f)
{
    x->x_phase = COSTABSIZE * f;
}

void osc_setup(void)
{    
    osc_class = class_new(sym_osc__tilde__, (t_newmethod)osc_new, 0,
        sizeof(t_osc), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(osc_class, t_osc, x_f);
    class_addMethod(osc_class, (t_method)osc_dsp, sym_dsp, A_CANT, 0);
    class_addMethod(osc_class, (t_method)osc_ft1, sym_inlet2, A_FLOAT, 0);

    cos_tilde_maketable();
}

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
    
    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];

    for (i = 0; i < n; i++)
    {
        float cf, cfindx, r, oneminusr;
        cf = *in2++ * isr;
        if (cf < 0) cf = 0;
        cfindx = cf * (float)(COSTABSIZE/6.28318f);
        r = (qinv > 0 ? 1 - cf * qinv : 0);
        if (r < 0) r = 0;
        oneminusr = 1.0f - r;
        dphase = ((double)(cfindx)) + DSP_UNITBIT32;
        tf.z_d = dphase;
        tabindex = tf.z_i[PD_RAWCAST64_MSB] & (COSTABSIZE-1);
        addr = tab + tabindex;
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        coefr = r * (f1 + frac * (f2 - f1));

        addr = tab + ((tabindex - (COSTABSIZE/4)) & (COSTABSIZE-1));
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
