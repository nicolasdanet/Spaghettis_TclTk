
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

/* ------ czero_rev~ - complex one-zero filter (raw, reverse form) ----- */

typedef struct sigczero_rev
{
    t_object x_obj;
    t_float x_f;
    t_sample x_lastre;
    t_sample x_lastim;
} t_sigczero_rev;

t_class *sigczero_rev_class;

static void *sigczero_rev_new(t_float re, t_float im)
{
    t_sigczero_rev *x = (t_sigczero_rev *)pd_new(sigczero_rev_class);
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

static t_int *sigczero_rev_perform(t_int *w)
{
    t_sample *inre1 = (t_sample *)(w[1]);
    t_sample *inim1 = (t_sample *)(w[2]);
    t_sample *inre2 = (t_sample *)(w[3]);
    t_sample *inim2 = (t_sample *)(w[4]);
    t_sample *outre = (t_sample *)(w[5]);
    t_sample *outim = (t_sample *)(w[6]);
    t_sigczero_rev *x = (t_sigczero_rev *)(w[7]);
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
            /* transfer function is (A bar) - Z^-1, for the same
            frequency response as 1 - AZ^-1 from czero_tilde. */
        *outre++ = lastre - nextre * coefre - nextim * coefim;
        *outim++ = lastim - nextre * coefim + nextim * coefre;
        lastre = nextre;
        lastim = nextim;
    }
    x->x_lastre = lastre;
    x->x_lastim = lastim;
    return (w+9);
}

static void sigczero_rev_dsp(t_sigczero_rev *x, t_signal **sp)
{
    dsp_add(sigczero_rev_perform, 8,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[3]->s_vector, 
        sp[4]->s_vector, sp[5]->s_vector, x, sp[0]->s_vectorSize);
}

static void sigczero_rev_clear(t_sigczero_rev *x)
{
    x->x_lastre = x->x_lastim = 0;
}

static void sigczero_rev_set(t_sigczero_rev *x, t_float re, t_float im)
{
    x->x_lastre = re;
    x->x_lastim = im;
}

void sigczero_rev_setup(void)
{
    sigczero_rev_class = class_new(sym_czero_rev__tilde__,
        (t_newmethod)sigczero_rev_new, 0, sizeof(t_sigczero_rev), 0, 
            A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigczero_rev_class, t_sigczero_rev, x_f);
    class_addMethod(sigczero_rev_class, (t_method)sigczero_rev_set,
        sym_set, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(sigczero_rev_class, (t_method)sigczero_rev_clear,
        sym_clear, 0);
    class_addMethod(sigczero_rev_class, (t_method)sigczero_rev_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
