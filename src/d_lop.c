
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
