
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

/* -------------- czero~ - complex one-zero filter (raw) --------------- */

typedef struct sigczero
{
    t_object x_obj;
    t_float x_f;
    t_sample x_lastre;
    t_sample x_lastim;
} t_sigczero;

t_class *sigczero_class;

static void *sigczero_new(t_float re, t_float im)
{
    t_sigczero *x = (t_sigczero *)pd_new(sigczero_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    pd_float(
        (t_pd *)inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal),
            re);
    pd_float(
        (t_pd *)inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal),
            im);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_lastre = x->x_lastim = 0;
    x->x_f = 0;
    return x;
}

static t_int *sigczero_perform(t_int *w)
{
    t_sample *inre1 = (t_sample *)(w[1]);
    t_sample *inim1 = (t_sample *)(w[2]);
    t_sample *inre2 = (t_sample *)(w[3]);
    t_sample *inim2 = (t_sample *)(w[4]);
    t_sample *outre = (t_sample *)(w[5]);
    t_sample *outim = (t_sample *)(w[6]);
    t_sigczero *x = (t_sigczero *)(w[7]);
    int n = (t_int)(w[8]);
    int i;
    t_sample lastre = x->x_lastre;
    t_sample lastim = x->x_lastim;
    for (i = 0; i < n; i++)
    {
        t_sample nextre = *inre1++;
        t_sample nextim = *inim1++;
        t_sample coefre = *inre2++;
        t_sample coefim = *inim2++;
        *outre++ = nextre - lastre * coefre + lastim * coefim;
        *outim++ = nextim - lastre * coefim - lastim * coefre;
        lastre = nextre;
        lastim = nextim;
    }
    x->x_lastre = lastre;
    x->x_lastim = lastim;
    return (w+9);
}

static void sigczero_dsp(t_sigczero *x, t_signal **sp)
{
    dsp_add(sigczero_perform, 8,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[3]->s_vector, 
        sp[4]->s_vector, sp[5]->s_vector, x, sp[0]->s_vectorSize);
}

static void sigczero_clear(t_sigczero *x)
{
    x->x_lastre = x->x_lastim = 0;
}

static void sigczero_set(t_sigczero *x, t_float re, t_float im)
{
    x->x_lastre = re;
    x->x_lastim = im;
}

void sigczero_setup(void)
{
    sigczero_class = class_new(sym_czero__tilde__,
        (t_newmethod)sigczero_new, 0, sizeof(t_sigczero), 0, 
            A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigczero_class, t_sigczero, x_f);
    class_addMethod(sigczero_class, (t_method)sigczero_set,
        sym_set, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(sigczero_class, (t_method)sigczero_clear,
        sym_clear, 0);
    class_addMethod(sigczero_class, (t_method)sigczero_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
