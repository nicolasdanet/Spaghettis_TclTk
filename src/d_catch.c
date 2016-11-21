
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

/* ----------------------------- catch~ ----------------------------- */
t_class *sigcatch_class;

static void *sigcatch_new(t_symbol *s)
{
    t_catch_tilde *x = (t_catch_tilde *)pd_new(sigcatch_class);
    pd_bind(&x->x_obj.te_g.g_pd, s);
    x->x_name = s;
    x->x_vectorSize = DSP_SEND_SIZE;
    x->x_vector = (t_sample *)PD_MEMORY_GET(DSP_SEND_SIZE * sizeof(t_sample));
    //memset(x->x_vec, 0, DSP_SEND_SIZE * sizeof(t_sample));
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *sigcatch_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--) *out++ = *in, *in++ = 0; 
    return (w+4);
}

/* tb: vectorized catch function */
static t_int *sigcatch_perf8(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    for (; n; n -= 8, in += 8, out += 8)
    {
       out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[3] = in[3]; 
       out[4] = in[4]; out[5] = in[5]; out[6] = in[6]; out[7] = in[7]; 
    
       in[0] = 0; in[1] = 0; in[2] = 0; in[3] = 0; 
       in[4] = 0; in[5] = 0; in[6] = 0; in[7] = 0; 
    }
    return (w+4);
}

static void sigcatch_dsp(t_catch_tilde *x, t_signal **sp)
{
    if (x->x_vectorSize == sp[0]->s_vectorSize)
    {
        if(sp[0]->s_vectorSize&7)
        dsp_add(sigcatch_perform, 3, x->x_vector, sp[0]->s_vector, sp[0]->s_vectorSize);
        else
        dsp_add(sigcatch_perf8, 3, x->x_vector, sp[0]->s_vector, sp[0]->s_vectorSize);
    }
    else post_error ("sigcatch %s: unexpected vector size", x->x_name->s_name);
}

static void sigcatch_free(t_catch_tilde *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_name);
    PD_MEMORY_FREE(x->x_vector);
}

void sigcatch_setup(void)
{
    sigcatch_class = class_new(sym_catch__tilde__, (t_newmethod)sigcatch_new,
        (t_method)sigcatch_free, sizeof(t_catch_tilde), CLASS_NOINLET, A_DEFSYMBOL, 0);
    class_addMethod(sigcatch_class, (t_method)sigcatch_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigcatch_class, sym_throw__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
