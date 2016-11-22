
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *catch_tilde_class;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void catch_tilde_dsp(t_catch_tilde *x, t_signal **sp)
{
    if (x->x_vectorSize == sp[0]->s_vectorSize)
    {
        dsp_addCopyZeroPerform (x->x_vector, sp[0]->s_vector, sp[0]->s_vectorSize);
        /*
        if(sp[0]->s_vectorSize&7)
        dsp_add(catch_tilde_perform, 3, x->x_vector, sp[0]->s_vector, sp[0]->s_vectorSize);
        else
        dsp_add(catch_tilde_perf8, 3, x->x_vector, sp[0]->s_vector, sp[0]->s_vectorSize);
        */
    }
    else post_error ("catch_tilde %s: unexpected vector size", x->x_name->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *catch_tilde_new(t_symbol *s)
{
    t_catch_tilde *x = (t_catch_tilde *)pd_new(catch_tilde_class);
    pd_bind(&x->x_obj.te_g.g_pd, s);
    x->x_name = s;
    x->x_vectorSize = DSP_SEND_SIZE;
    x->x_vector = (t_sample *)PD_MEMORY_GET(DSP_SEND_SIZE * sizeof(t_sample));
    //memset(x->x_vec, 0, DSP_SEND_SIZE * sizeof(t_sample));
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static void catch_tilde_free(t_catch_tilde *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_name);
    PD_MEMORY_FREE(x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void catch_tilde_setup (void)
{
    catch_tilde_class = class_new(sym_catch__tilde__, (t_newmethod)catch_tilde_new,
        (t_method)catch_tilde_free, sizeof(t_catch_tilde), CLASS_NOINLET, A_DEFSYMBOL, 0);
    class_addMethod(catch_tilde_class, (t_method)catch_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(catch_tilde_class, sym_throw__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
