
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

/* ---------- rzero_rev~ - real, reverse one-zero filter (raw) ------------ */

typedef struct sigrzero_rev
{
    t_object x_obj;
    t_float x_f;
    t_sample x_last;
} t_sigrzero_rev;

t_class *sigrzero_rev_class;

static void *sigrzero_rev_new(t_float f)
{
    t_sigrzero_rev *x = (t_sigrzero_rev *)pd_new(sigrzero_rev_class);
    pd_float(
        (t_pd *)inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal),
            f);
    outlet_new(&x->x_obj, &s_signal);
    x->x_last = 0;
    return x;
}

static t_int *sigrzero_rev_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    t_sigrzero_rev *x = (t_sigrzero_rev *)(w[4]);
    int n = (t_int)(w[5]);
    int i;
    t_sample last = x->x_last;
    for (i = 0; i < n; i++)
    {
        t_sample next = *in1++;
        t_sample coef = *in2++;
        *out++ = last - coef * next;
        last = next;
    }
    x->x_last = last;
    return (w+6);
}

static void sigrzero_rev_dsp(t_sigrzero_rev *x, t_signal **sp)
{
    dsp_add(sigrzero_rev_perform, 5,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, 
            x, sp[0]->s_vectorSize);
}

static void sigrzero_rev_clear(t_sigrzero_rev *x)
{
    x->x_last = 0;
}

static void sigrzero_rev_set(t_sigrzero_rev *x, t_float f)
{
    x->x_last = f;
}

void sigrzero_rev_setup(void)
{
    sigrzero_rev_class = class_new(sym_rzero_rev__tilde__,
        (t_newmethod)sigrzero_rev_new, 0, sizeof(t_sigrzero_rev),
        0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigrzero_rev_class, t_sigrzero_rev, x_f);
    class_addMethod(sigrzero_rev_class, (t_method)sigrzero_rev_set,
        sym_set, A_DEFFLOAT, 0);
    class_addMethod(sigrzero_rev_class, (t_method)sigrzero_rev_clear,
        sym_clear, 0);
    class_addMethod(sigrzero_rev_class, (t_method)sigrzero_rev_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
