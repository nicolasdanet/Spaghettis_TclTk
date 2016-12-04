
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

static t_class *exp_tilde_class;

typedef struct _exp_tilde
{
    t_object x_obj;
    t_float x_f;
} t_exp_tilde;

static void *exp_tilde_new( void)
{
    t_exp_tilde *x = (t_exp_tilde *)pd_new(exp_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

t_int *exp_tilde_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
        *out++ = exp(*in1++);
    return (w+4);
}

static void exp_tilde_dsp(t_exp_tilde *x, t_signal **sp)
{
    dsp_add(exp_tilde_perform, 3,
        sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void exp_tilde_setup(void)
{
    exp_tilde_class = class_new(sym_exp__tilde__, (t_newmethod)exp_tilde_new, 0,
        sizeof(t_exp_tilde), 0, 0);
    CLASS_SIGNAL(exp_tilde_class, t_exp_tilde, x_f);
    class_addMethod(exp_tilde_class, (t_method)exp_tilde_dsp,
        sym_dsp, A_CANT, 0);
}
