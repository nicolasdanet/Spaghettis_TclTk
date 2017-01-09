
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

/* ---------------- samphold~ - sample and hold  ----------------- */

typedef struct sigsamphold
{
    t_object x_obj;
    t_float x_f;
    t_sample x_lastin;
    t_sample x_lastout;
} t_sigsamphold;

t_class *sigsamphold_class;

static void *sigsamphold_new(void)
{
    t_sigsamphold *x = (t_sigsamphold *)pd_new(sigsamphold_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_lastin = 0;
    x->x_lastout = 0;
    x->x_f = 0;
    return x;
}

static t_int *sigsamphold_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    t_sigsamphold *x = (t_sigsamphold *)(w[4]);
    int n = (t_int)(w[5]);
    int i;
    t_sample lastin = x->x_lastin;
    t_sample lastout = x->x_lastout;
    for (i = 0; i < n; i++, in1++)
    {
        t_sample next = *in2++;
        if (next < lastin) lastout = *in1;
        *out++ = lastout;
        lastin = next;
    }
    x->x_lastin = lastin;
    x->x_lastout = lastout;
    return (w+6);
}

static void sigsamphold_dsp(t_sigsamphold *x, t_signal **sp)
{
    dsp_add(sigsamphold_perform, 5,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, 
            x, sp[0]->s_vectorSize);
}

static void sigsamphold_reset(t_sigsamphold *x, t_symbol *s, int argc,
    t_atom *argv)
{
    x->x_lastin = ((argc > 0 && (argv[0].a_type == A_FLOAT)) ?
        argv[0].a_w.w_float : 1e20);
}

static void sigsamphold_set(t_sigsamphold *x, t_float f)
{
    x->x_lastout = f;
}

void sigsamphold_setup(void)
{
    sigsamphold_class = class_new(sym_samphold__tilde__,
        (t_newmethod)sigsamphold_new, 0, sizeof(t_sigsamphold), 0, 0);
    CLASS_SIGNAL(sigsamphold_class, t_sigsamphold, x_f);
    class_addMethod(sigsamphold_class, (t_method)sigsamphold_set,
        sym_set, A_DEFFLOAT, 0);
    class_addMethod(sigsamphold_class, (t_method)sigsamphold_reset,
        sym_reset, A_GIMME, 0);
    class_addMethod(sigsamphold_class, (t_method)sigsamphold_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
