
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

/* ------------------------- clip~ -------------------------- */
static t_class *clip_class;

typedef struct _clip_tilde
{
    t_object x_obj;
    t_float x_f;
    t_float x_lo;
    t_float x_hi;
} t_clip_tilde;

static void *clip_tilde_new(t_float lo, t_float hi)
{
    t_clip_tilde *x = (t_clip_tilde *)pd_new(clip_class);
    x->x_lo = lo;
    x->x_hi = hi;
    outlet_new(&x->x_obj, &s_signal);
    inlet_newFloat(&x->x_obj, &x->x_lo);
    inlet_newFloat(&x->x_obj, &x->x_hi);
    x->x_f = 0;
    return (x);
}

static t_int *clip_perform(t_int *w)
{
    t_clip_tilde *x = (t_clip_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
        t_sample f = *in++;
        if (f < x->x_lo) f = x->x_lo;
        if (f > x->x_hi) f = x->x_hi;
        *out++ = f;
    }
    return (w+5);
}

static void clip_dsp(t_clip_tilde *x, t_signal **sp)
{
    dsp_add(clip_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void clip_tilde_setup(void)
{
    clip_class = class_new(sym_clip__tilde__, (t_newmethod)clip_tilde_new, 0,
        sizeof(t_clip_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(clip_class, t_clip_tilde, x_f);
    class_addMethod(clip_class, (t_method)clip_dsp, sym_dsp, A_CANT, 0);
}
