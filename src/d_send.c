
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
#include "d_global.h"

/* ----------------------------- send~ ----------------------------- */
t_class *sigsend_class;

static void *sigsend_new(t_symbol *s)
{
    t_send_tilde *x = (t_send_tilde *)pd_new(sigsend_class);
    pd_bind(&x->x_obj.te_g.g_pd, s);
    x->x_name = s;
    x->x_vectorSize = DSP_SEND_SIZE;
    x->x_vector = (t_sample *)PD_MEMORY_GET(DSP_SEND_SIZE * sizeof(t_sample));
    //memset(x->x_vec, 0, DSP_SEND_SIZE * sizeof(t_sample));
    x->x_f = 0;
    return (x);
}

static t_int *sigsend_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
    {
        *out = (PD_BIG_OR_SMALL(*in) ? 0 : *in);
        out++;
        in++;
    }
    return (w+4);
}

static void sigsend_dsp(t_send_tilde *x, t_signal **sp)
{
    if (x->x_vectorSize == sp[0]->s_vectorSize)
        dsp_add(sigsend_perform, 3, sp[0]->s_vector, x->x_vector, sp[0]->s_vectorSize);
    else post_error ("sigsend %s: unexpected vector size", x->x_name->s_name);
}

static void sigsend_free(t_send_tilde *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_name);
    PD_MEMORY_FREE(x->x_vector);
}

void sigsend_setup(void)
{
    sigsend_class = class_new(sym_send__tilde__, (t_newmethod)sigsend_new,
        (t_method)sigsend_free, sizeof(t_send_tilde), 0, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)sigsend_new, sym_s__tilde__, A_DEFSYMBOL, 0);
    CLASS_SIGNAL(sigsend_class, t_send_tilde, x_f);
    class_addMethod(sigsend_class, (t_method)sigsend_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
