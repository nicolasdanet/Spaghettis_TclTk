
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

/* ---------------- hip~ - 1-pole 1-zero hipass filter. ----------------- */

typedef struct hipctl
{
    t_sample c_x;
    t_sample c_coef;
} t_hipctl;

typedef struct sighip
{
    t_object x_obj;
    t_float x_sr;
    t_float x_hz;
    t_hipctl x_cspace;
    t_hipctl *x_ctl;
    t_float x_f;
} t_sighip;

t_class *sighip_class;
static void sighip_ft1(t_sighip *x, t_float f);

static void *sighip_new(t_float f)
{
    t_sighip *x = (t_sighip *)pd_new(sighip_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    outlet_new(&x->x_obj, &s_signal);
    x->x_sr = 44100;
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_x = 0;
    sighip_ft1(x, f);
    x->x_f = 0;
    return x;
}

static void sighip_ft1(t_sighip *x, t_float f)
{
    if (f < 0) f = 0;
    x->x_hz = f;
    x->x_ctl->c_coef = 1 - f * (2 * PD_PI) / x->x_sr;
    if (x->x_ctl->c_coef < 0)
        x->x_ctl->c_coef = 0;
    else if (x->x_ctl->c_coef > 1)
        x->x_ctl->c_coef = 1;
}

static t_int *sighip_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_hipctl *c = (t_hipctl *)(w[3]);
    int n = (t_int)(w[4]);
    int i;
    t_sample last = c->c_x;
    t_sample coef = c->c_coef;
    if (coef < 1)
    {
        t_sample normal = 0.5*(1+coef);
        for (i = 0; i < n; i++)
        {
            t_sample new = *in++ + coef * last;
            *out++ = normal * (new - last);
            last = new;
        }
        if (PD_IS_BIG_OR_SMALL(last))
            last = 0; 
        c->c_x = last;
    }
    else
    {
        for (i = 0; i < n; i++)
            *out++ = *in++;
        c->c_x = 0;
    }
    return (w+5);
}

static t_int *sighip_perform_old(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_hipctl *c = (t_hipctl *)(w[3]);
    int n = (t_int)(w[4]);
    int i;
    t_sample last = c->c_x;
    t_sample coef = c->c_coef;
    if (coef < 1)
    {
        for (i = 0; i < n; i++)
        {
            t_sample new = *in++ + coef * last;
            *out++ = new - last;
            last = new;
        }
        if (PD_IS_BIG_OR_SMALL(last))
            last = 0; 
        c->c_x = last;
    }
    else
    {
        for (i = 0; i < n; i++)
            *out++ = *in++;
        c->c_x = 0;
    }
    return (w+5);
}

static void sighip_dsp(t_sighip *x, t_signal **sp)
{
    x->x_sr = sp[0]->s_sampleRate;
    sighip_ft1(x,  x->x_hz);
    dsp_add((1 ?
        sighip_perform : sighip_perform_old),
            4, sp[0]->s_vector, sp[1]->s_vector, x->x_ctl, sp[0]->s_vectorSize);
}

static void sighip_clear(t_sighip *x, t_float q)
{
    x->x_cspace.c_x = 0;
}

void sighip_setup(void)
{
    sighip_class = class_new(sym_hip__tilde__, (t_newmethod)sighip_new, 0,
        sizeof(t_sighip), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sighip_class, t_sighip, x_f);
    class_addMethod(sighip_class, (t_method)sighip_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(sighip_class, (t_method)sighip_ft1,
        sym_inlet2, A_FLOAT, 0);
    class_addMethod(sighip_class, (t_method)sighip_clear, sym_clear, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---------------- lop~ - 1-pole lopass filter. ----------------- */

typedef struct lopctl
{
    t_sample c_x;
    t_sample c_coef;
} t_lopctl;

typedef struct siglop
{
    t_object x_obj;
    t_float x_sr;
    t_float x_hz;
    t_lopctl x_cspace;
    t_lopctl *x_ctl;
    t_float x_f;
} t_siglop;

t_class *siglop_class;

static void siglop_ft1(t_siglop *x, t_float f);

static void *siglop_new(t_float f)
{
    t_siglop *x = (t_siglop *)pd_new(siglop_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    outlet_new(&x->x_obj, &s_signal);
    x->x_sr = 44100;
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_x = 0;
    siglop_ft1(x, f);
    x->x_f = 0;
    return x;
}

static void siglop_ft1(t_siglop *x, t_float f)
{
    if (f < 0) f = 0;
    x->x_hz = f;
    x->x_ctl->c_coef = f * (2 * PD_PI) / x->x_sr;
    if (x->x_ctl->c_coef > 1)
        x->x_ctl->c_coef = 1;
    else if (x->x_ctl->c_coef < 0)
        x->x_ctl->c_coef = 0;
}

static void siglop_clear(t_siglop *x, t_float q)
{
    x->x_cspace.c_x = 0;
}

static t_int *siglop_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_lopctl *c = (t_lopctl *)(w[3]);
    int n = (t_int)(w[4]);
    int i;
    t_sample last = c->c_x;
    t_sample coef = c->c_coef;
    t_sample feedback = 1 - coef;
    for (i = 0; i < n; i++)
        last = *out++ = coef * *in++ + feedback * last;
    if (PD_IS_BIG_OR_SMALL(last))
        last = 0;
    c->c_x = last;
    return (w+5);
}

static void siglop_dsp(t_siglop *x, t_signal **sp)
{
    x->x_sr = sp[0]->s_sampleRate;
    siglop_ft1(x,  x->x_hz);
    dsp_add(siglop_perform, 4,
        sp[0]->s_vector, sp[1]->s_vector, 
            x->x_ctl, sp[0]->s_vectorSize);

}

void siglop_setup(void)
{
    siglop_class = class_new(sym_lop__tilde__, (t_newmethod)siglop_new, 0,
        sizeof(t_siglop), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(siglop_class, t_siglop, x_f);
    class_addMethod(siglop_class, (t_method)siglop_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(siglop_class, (t_method)siglop_ft1,
        sym_inlet2, A_FLOAT, 0);
    class_addMethod(siglop_class, (t_method)siglop_clear, sym_clear, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---------------- bp~ - 2-pole bandpass filter. ----------------- */

typedef struct bpctl
{
    t_sample c_x1;
    t_sample c_x2;
    t_sample c_coef1;
    t_sample c_coef2;
    t_sample c_gain;
} t_bpctl;

typedef struct sigbp
{
    t_object x_obj;
    t_float x_sr;
    t_float x_freq;
    t_float x_q;
    t_bpctl x_cspace;
    t_bpctl *x_ctl;
    t_float x_f;
} t_sigbp;

t_class *sigbp_class;

static void sigbp_docoef(t_sigbp *x, t_float f, t_float q);

static void *sigbp_new(t_float f, t_float q)
{
    t_sigbp *x = (t_sigbp *)pd_new(sigbp_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet3);
    outlet_new(&x->x_obj, &s_signal);
    x->x_sr = 44100;
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_x1 = 0;
    x->x_cspace.c_x2 = 0;
    sigbp_docoef(x, f, q);
    x->x_f = 0;
    return x;
}

static t_float sigbp_qcos(t_float f)
{
    if (f >= -(0.5f*PD_PI) && f <= 0.5f*PD_PI)
    {
        t_float g = f*f;
        return (((g*g*g * (-1.0f/720.0f) + g*g*(1.0f/24.0f)) - g*0.5) + 1);
    }
    else return (0);
}

static void sigbp_docoef(t_sigbp *x, t_float f, t_float q)
{
    t_float r, oneminusr, omega;
    if (f < 0.001) f = 10;
    if (q < 0) q = 0;
    x->x_freq = f;
    x->x_q = q;
    omega = f * (2.0f * PD_PI) / x->x_sr;
    if (q < 0.001) oneminusr = 1.0f;
    else oneminusr = omega/q;
    if (oneminusr > 1.0f) oneminusr = 1.0f;
    r = 1.0f - oneminusr;
    x->x_ctl->c_coef1 = 2.0f * sigbp_qcos(omega) * r;
    x->x_ctl->c_coef2 = - r * r;
    x->x_ctl->c_gain = 2 * oneminusr * (oneminusr + r * omega);
    /* post("r %f, omega %f, coef1 %f, coef2 %f",
        r, omega, x->x_ctl->c_coef1, x->x_ctl->c_coef2); */
}

static void sigbp_ft1(t_sigbp *x, t_float f)
{
    sigbp_docoef(x, f, x->x_q);
}

static void sigbp_ft2(t_sigbp *x, t_float q)
{
    sigbp_docoef(x, x->x_freq, q);
}

static void sigbp_clear(t_sigbp *x, t_float q)
{
    x->x_ctl->c_x1 = x->x_ctl->c_x2 = 0;
}

static t_int *sigbp_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_bpctl *c = (t_bpctl *)(w[3]);
    int n = (t_int)(w[4]);
    int i;
    t_sample last = c->c_x1;
    t_sample prev = c->c_x2;
    t_sample coef1 = c->c_coef1;
    t_sample coef2 = c->c_coef2;
    t_sample gain = c->c_gain;
    for (i = 0; i < n; i++)
    {
        t_sample output =  *in++ + coef1 * last + coef2 * prev;
        *out++ = gain * output;
        prev = last;
        last = output;
    }
    if (PD_IS_BIG_OR_SMALL(last))
        last = 0;
    if (PD_IS_BIG_OR_SMALL(prev))
        prev = 0;
    c->c_x1 = last;
    c->c_x2 = prev;
    return (w+5);
}

static void sigbp_dsp(t_sigbp *x, t_signal **sp)
{
    x->x_sr = sp[0]->s_sampleRate;
    sigbp_docoef(x, x->x_freq, x->x_q);
    dsp_add(sigbp_perform, 4,
        sp[0]->s_vector, sp[1]->s_vector, 
            x->x_ctl, sp[0]->s_vectorSize);

}

void sigbp_setup(void)
{
    sigbp_class = class_new(sym_bp__tilde__, (t_newmethod)sigbp_new, 0,
        sizeof(t_sigbp), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigbp_class, t_sigbp, x_f);
    class_addMethod(sigbp_class, (t_method)sigbp_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(sigbp_class, (t_method)sigbp_ft1,
        sym_inlet2, A_FLOAT, 0);
    class_addMethod(sigbp_class, (t_method)sigbp_ft2,
        sym_inlet3, A_FLOAT, 0);
    class_addMethod(sigbp_class, (t_method)sigbp_clear, sym_clear, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---------------- biquad~ - raw biquad filter ----------------- */

typedef struct biquadctl
{
    t_sample c_x1;
    t_sample c_x2;
    t_sample c_fb1;
    t_sample c_fb2;
    t_sample c_ff1;
    t_sample c_ff2;
    t_sample c_ff3;
} t_biquadctl;

typedef struct sigbiquad
{
    t_object x_obj;
    t_float x_f;
    t_biquadctl x_cspace;
    t_biquadctl *x_ctl;
} t_sigbiquad;

t_class *sigbiquad_class;

static void sigbiquad_list(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv);

static void *sigbiquad_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sigbiquad *x = (t_sigbiquad *)pd_new(sigbiquad_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_x1 = x->x_cspace.c_x2 = 0;
    sigbiquad_list(x, s, argc, argv);
    x->x_f = 0;
    return x;
}

static t_int *sigbiquad_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_biquadctl *c = (t_biquadctl *)(w[3]);
    int n = (t_int)(w[4]);
    int i;
    t_sample last = c->c_x1;
    t_sample prev = c->c_x2;
    t_sample fb1 = c->c_fb1;
    t_sample fb2 = c->c_fb2;
    t_sample ff1 = c->c_ff1;
    t_sample ff2 = c->c_ff2;
    t_sample ff3 = c->c_ff3;
    for (i = 0; i < n; i++)
    {
        t_sample output =  *in++ + fb1 * last + fb2 * prev;
        if (PD_IS_BIG_OR_SMALL(output))
            output = 0; 
        *out++ = ff1 * output + ff2 * last + ff3 * prev;
        prev = last;
        last = output;
    }
    c->c_x1 = last;
    c->c_x2 = prev;
    return (w+5);
}

static void sigbiquad_list(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float fb1 = atom_getFloatAtIndex(0, argc, argv);
    t_float fb2 = atom_getFloatAtIndex(1, argc, argv);
    t_float ff1 = atom_getFloatAtIndex(2, argc, argv);
    t_float ff2 = atom_getFloatAtIndex(3, argc, argv);
    t_float ff3 = atom_getFloatAtIndex(4, argc, argv);
    t_float discriminant = fb1 * fb1 + 4 * fb2;
    t_biquadctl *c = x->x_ctl;
    if (discriminant < 0) /* imaginary roots -- resonant filter */
    {
            /* they're conjugates so we just check that the product
            is less than one */
        if (fb2 >= -1.0f) goto stable;
    }
    else    /* real roots */
    {
            /* check that the parabola 1 - fb1 x - fb2 x^2 has a
                vertex between -1 and 1, and that it's nonnegative
                at both ends, which implies both roots are in [1-,1]. */
        if (fb1 <= 2.0f && fb1 >= -2.0f &&
            1.0f - fb1 -fb2 >= 0 && 1.0f + fb1 - fb2 >= 0)
                goto stable;
    }
        /* if unstable, just bash to zero */
    fb1 = fb2 = ff1 = ff2 = ff3 = 0;
stable:
    c->c_fb1 = fb1;
    c->c_fb2 = fb2;
    c->c_ff1 = ff1;
    c->c_ff2 = ff2;
    c->c_ff3 = ff3;
}

static void sigbiquad_set(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv)
{
    t_biquadctl *c = x->x_ctl;
    c->c_x1 = atom_getFloatAtIndex(0, argc, argv);
    c->c_x2 = atom_getFloatAtIndex(1, argc, argv);
}

static void sigbiquad_dsp(t_sigbiquad *x, t_signal **sp)
{
    dsp_add(sigbiquad_perform, 4,
        sp[0]->s_vector, sp[1]->s_vector, 
            x->x_ctl, sp[0]->s_vectorSize);

}

void sigbiquad_setup(void)
{
    sigbiquad_class = class_new(sym_biquad__tilde__, (t_newmethod)sigbiquad_new,
        0, sizeof(t_sigbiquad), 0, A_GIMME, 0);
    CLASS_SIGNAL(sigbiquad_class, t_sigbiquad, x_f);
    class_addMethod(sigbiquad_class, (t_method)sigbiquad_dsp,
        sym_dsp, A_CANT, 0);
    class_addList(sigbiquad_class, (t_method)sigbiquad_list);
    class_addMethod(sigbiquad_class, (t_method)sigbiquad_set, sym_set,
        A_GIMME, 0);
    class_addMethod(sigbiquad_class, (t_method)sigbiquad_set, sym_clear,
        A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
