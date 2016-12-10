
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
#include "s_system.h"
#include "d_dsp.h"

/* -------------------------- sig~ ------------------------------ */
static t_class *sig_tilde_class;

typedef struct _sig_tilde
{
    t_object x_obj;
    t_float x_f;
} t_sig;

static void sig_tilde_float(t_sig *x, t_float f)
{
    x->x_f = f;
}

static void sig_tilde_dsp(t_sig *x, t_signal **sp)
{
    dsp_addScalarPerform (&x->x_f, sp[0]->s_vector, sp[0]->s_vectorSize);
    // dsp_add(sig_tilde_perform, 3, &x->x_f, sp[0]->s_vector, sp[0]->s_vectorSize);
}

static void *sig_tilde_new(t_float f)
{
    t_sig *x = (t_sig *)pd_new(sig_tilde_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_signal);
    return x;
}

void sig_tilde_setup(void)
{
    sig_tilde_class = class_new(sym_sig__tilde__, (t_newmethod)sig_tilde_new, 0,
        sizeof(t_sig), 0, A_DEFFLOAT, 0);
    class_addFloat(sig_tilde_class, (t_method)sig_tilde_float);
    class_addMethod(sig_tilde_class, (t_method)sig_tilde_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
