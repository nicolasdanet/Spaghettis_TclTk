
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

typedef struct mtof_tilde
{
    t_object x_obj;
    t_float x_f;
} t_mtof_tilde;

t_class *mtof_tilde_class;

static void *mtof_tilde_new(void)
{
    t_mtof_tilde *x = (t_mtof_tilde *)pd_new(mtof_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *mtof_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        if (f <= -1500) *out = 0;
        else
        {
            if (f > 1499) f = 1499;
            *out = 8.17579891564 * exp(.0577622650 * f);
        }
    }
    return (w + 4);
}

static void mtof_tilde_dsp(t_mtof_tilde *x, t_signal **sp)
{
    dsp_add(mtof_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void mtof_tilde_setup(void)
{
    mtof_tilde_class = class_new(sym_mtof__tilde__, (t_newmethod)mtof_tilde_new, 0,
        sizeof(t_mtof_tilde), 0, 0);
    CLASS_SIGNAL(mtof_tilde_class, t_mtof_tilde, x_f);
    class_addMethod(mtof_tilde_class, (t_method)mtof_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(mtof_tilde_class, sym_mtof__tilde__);
}
