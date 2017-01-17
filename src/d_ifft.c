
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
#include "d_math.h"

static t_class *sigifft_class;

typedef struct _ifft_tilde
{
    t_object x_obj;
    t_float x_f;
} t_ifft_tilde;

static void *sigifft_new(void)
{
    t_ifft_tilde *x = (t_ifft_tilde *)pd_new(sigifft_class);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *sigifft_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    int n = w[3];
    mayer_ifft(n, in1, in2);
    return (w+4);
}

static t_int *sigifft_swap(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    int n = w[3];
    for (;n--; in1++, in2++)
    {   
        t_sample f = *in1;
        *in1 = *in2;
        *in2 = f;
    }
    return (w+4);    
}

static void sigifft_dspx(t_ifft_tilde *x, t_signal **sp, t_int *(*f)(t_int *w))
{
    int n = sp[0]->s_vectorSize;
    t_sample *in1 = sp[0]->s_vector;
    t_sample *in2 = sp[1]->s_vector;
    t_sample *out1 = sp[2]->s_vector;
    t_sample *out2 = sp[3]->s_vector;
    if (out1 == in2 && out2 == in1)
        dsp_add(sigifft_swap, 3, out1, out2, n);
    else if (out1 == in2)
    {
        dsp_addCopyPerform (in2, out2, n);
        dsp_addCopyPerform (in1, out1, n);
    }
    else
    {
        if (out1 != in1) dsp_addCopyPerform (in1, out1, n);
        if (out2 != in2) dsp_addCopyPerform (in2, out2, n);
    }
    dsp_add(f, 3, sp[2]->s_vector, sp[3]->s_vector, n);
}

static void sigifft_dsp(t_ifft_tilde *x, t_signal **sp)
{
    sigifft_dspx(x, sp, sigifft_perform);
}

void ifft_tilde_setup(void)
{
    sigifft_class = class_new(sym_ifft__tilde__, sigifft_new, 0,
        sizeof(t_ifft_tilde), 0, 0);
    CLASS_SIGNAL(sigifft_class, t_ifft_tilde, x_f);
    class_addMethod(sigifft_class, (t_method)sigifft_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigifft_class, sym_fft__tilde__);
}
