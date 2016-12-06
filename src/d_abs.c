
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

static t_class *abs_tilde_class;

typedef struct _abs_tilde
{
    t_object x_obj;
    t_float x_f;
} t_abs_tilde;

static void *abs_tilde_new( void)
{
    t_abs_tilde *x = (t_abs_tilde *)pd_new(abs_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    return x;
}

t_int *abs_tilde_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
    {
        float f = *in1++;
        *out++ = (f >= 0 ? f : -f);
    }
    return (w+4);
}

static void abs_tilde_dsp(t_abs_tilde *x, t_signal **sp)
{
    dsp_add(abs_tilde_perform, 3,
        sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void abs_tilde_setup(void)
{
    abs_tilde_class = class_new(sym_abs__tilde__, (t_newmethod)abs_tilde_new, 0,
        sizeof(t_abs_tilde), 0, 0);
    CLASS_SIGNAL(abs_tilde_class, t_abs_tilde, x_f);
    class_addMethod(abs_tilde_class, (t_method)abs_tilde_dsp,
        sym_dsp, A_CANT, 0);
}

