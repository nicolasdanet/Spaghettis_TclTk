
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

typedef struct ftom_tilde
{
    t_object x_obj;
    t_float x_f;
} t_ftom_tilde;

t_class *ftom_tilde_class;

static void *ftom_tilde_new(void)
{
    t_ftom_tilde *x = (t_ftom_tilde *)pd_new(ftom_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *ftom_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        *out = (f > 0 ? 17.3123405046 * log(.12231220585 * f) : -1500);
    }
    return (w + 4);
}

static void ftom_tilde_dsp(t_ftom_tilde *x, t_signal **sp)
{
    dsp_add(ftom_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void ftom_tilde_setup(void)
{
    ftom_tilde_class = class_new(sym_ftom__tilde__, (t_newmethod)ftom_tilde_new, 0,
        sizeof(t_ftom_tilde), 0, 0);
    CLASS_SIGNAL(ftom_tilde_class, t_ftom_tilde, x_f);
    class_addMethod(ftom_tilde_class, (t_method)ftom_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(ftom_tilde_class, sym_mtof__tilde__);
}
