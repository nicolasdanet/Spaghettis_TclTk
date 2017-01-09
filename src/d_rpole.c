
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

/* ---------------- rpole~ - real one-pole filter (raw) ----------------- */

typedef struct sigrpole
{
    t_object x_obj;
    t_float x_f;
    t_sample x_last;
} t_sigrpole;

t_class *sigrpole_class;

static void *sigrpole_new(t_float f)
{
    t_sigrpole *x = (t_sigrpole *)pd_new(sigrpole_class);
    pd_float(
        (t_pd *)inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal),
            f);
    outlet_new(&x->x_obj, &s_signal);
    x->x_last = 0;
    return x;
}

static t_int *sigrpole_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    t_sigrpole *x = (t_sigrpole *)(w[4]);
    int n = (t_int)(w[5]);
    int i;
    t_sample last = x->x_last;
    for (i = 0; i < n; i++)
    {
        t_sample next = *in1++;
        t_sample coef = *in2++;
        *out++ = last = coef * last + next;
    }
    if (PD_IS_BIG_OR_SMALL(last))
        last = 0;
    x->x_last = last;
    return (w+6);
}

static void sigrpole_dsp(t_sigrpole *x, t_signal **sp)
{
    dsp_add(sigrpole_perform, 5,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, 
            x, sp[0]->s_vectorSize);
}

static void sigrpole_clear(t_sigrpole *x)
{
    x->x_last = 0;
}

static void sigrpole_set(t_sigrpole *x, t_float f)
{
    x->x_last = f;
}

void sigrpole_setup(void)
{
    sigrpole_class = class_new(sym_rpole__tilde__,
        (t_newmethod)sigrpole_new, 0, sizeof(t_sigrpole), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigrpole_class, t_sigrpole, x_f);
    class_addMethod(sigrpole_class, (t_method)sigrpole_set,
        sym_set, A_DEFFLOAT, 0);
    class_addMethod(sigrpole_class, (t_method)sigrpole_clear,
        sym_clear, 0);
    class_addMethod(sigrpole_class, (t_method)sigrpole_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
