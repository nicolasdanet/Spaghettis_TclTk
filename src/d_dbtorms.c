
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

typedef struct dbtorms_tilde
{
    t_object x_obj;
    t_float x_f;
} t_dbtorms_tilde;

t_class *dbtorms_tilde_class;

static void *dbtorms_tilde_new(void)
{
    t_dbtorms_tilde *x = (t_dbtorms_tilde *)pd_new(dbtorms_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *dbtorms_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        if (f <= 0) *out = 0;
        else
        {
            if (f > 485)
                f = 485;
            *out = exp((PD_LOGTEN * 0.05) * (f-100.));
        }
    }
    return (w + 4);
}

static void dbtorms_tilde_dsp(t_dbtorms_tilde *x, t_signal **sp)
{
    dsp_add(dbtorms_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void dbtorms_tilde_setup(void)
{
    dbtorms_tilde_class = class_new(sym_dbtorms__tilde__, (t_newmethod)dbtorms_tilde_new, 0,
        sizeof(t_dbtorms_tilde), 0, 0);
    CLASS_SIGNAL(dbtorms_tilde_class, t_dbtorms_tilde, x_f);
    class_addMethod(dbtorms_tilde_class, (t_method)dbtorms_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(dbtorms_tilde_class, sym_mtof__tilde__);
}
