
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

typedef struct rmstodb_tilde
{
    t_object x_obj;
    t_float x_f;
} t_rmstodb_tilde;

t_class *rmstodb_tilde_class;

static void *rmstodb_tilde_new(void)
{
    t_rmstodb_tilde *x = (t_rmstodb_tilde *)pd_new(rmstodb_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *rmstodb_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        if (f <= 0) *out = 0;
        else
        {
            t_sample g = 100 + 20./PD_LOGTEN * log(f);
            *out = (g < 0 ? 0 : g);
        }
    }
    return (w + 4);
}

static void rmstodb_tilde_dsp(t_rmstodb_tilde *x, t_signal **sp)
{
    dsp_add(rmstodb_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void rmstodb_tilde_setup(void)
{
    rmstodb_tilde_class = class_new(sym_rmstodb__tilde__, 
        (t_newmethod)rmstodb_tilde_new, 0, sizeof(t_rmstodb_tilde), 0, 0);
    CLASS_SIGNAL(rmstodb_tilde_class, t_rmstodb_tilde, x_f);
    class_addMethod(rmstodb_tilde_class, (t_method)rmstodb_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(rmstodb_tilde_class, sym_mtof__tilde__);
}
