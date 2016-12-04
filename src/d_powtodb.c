
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

typedef struct powtodb_tilde
{
    t_object x_obj;
    t_float x_f;
} t_powtodb_tilde;

t_class *powtodb_tilde_class;

static void *powtodb_tilde_new(void)
{
    t_powtodb_tilde *x = (t_powtodb_tilde *)pd_new(powtodb_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *powtodb_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        if (f <= 0) *out = 0;
        else
        {
            t_sample g = 100 + 10./PD_LOGTEN * log(f);
            *out = (g < 0 ? 0 : g);
        }
    }
    return (w + 4);
}

static void powtodb_tilde_dsp(t_powtodb_tilde *x, t_signal **sp)
{
    dsp_add(powtodb_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void powtodb_tilde_setup(void)
{
    powtodb_tilde_class = class_new(sym_powtodb__tilde__, (t_newmethod)powtodb_tilde_new, 0,
        sizeof(t_powtodb_tilde), 0, 0);
    CLASS_SIGNAL(powtodb_tilde_class, t_powtodb_tilde, x_f);
    class_addMethod(powtodb_tilde_class, (t_method)powtodb_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(powtodb_tilde_class, sym_mtof__tilde__);
}
