
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

typedef struct dbtopow_tilde
{
    t_object x_obj;
    t_float x_f;
} t_dbtopow_tilde;

t_class *dbtopow_tilde_class;

static void *dbtopow_tilde_new(void)
{
    t_dbtopow_tilde *x = (t_dbtopow_tilde *)pd_new(dbtopow_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *dbtopow_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        if (f <= 0) *out = 0;
        else
        {
            if (f > 870)
                f = 870;
            *out = exp((PD_LOGTEN * 0.1) * (f-100.));
        }
    }
    return (w + 4);
}

static void dbtopow_tilde_dsp(t_dbtopow_tilde *x, t_signal **sp)
{
    dsp_add(dbtopow_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void dbtopow_tilde_setup(void)
{
    dbtopow_tilde_class = class_new(sym_dbtopow__tilde__, (t_newmethod)dbtopow_tilde_new, 0,
        sizeof(t_dbtopow_tilde), 0, 0);
    CLASS_SIGNAL(dbtopow_tilde_class, t_dbtopow_tilde, x_f);
    class_addMethod(dbtopow_tilde_class, (t_method)dbtopow_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(dbtopow_tilde_class, sym_mtof__tilde__);
}
