
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sigrifft_class;

typedef struct rifft
{
    t_object x_obj;
    t_float x_f;
} t_sigrifft;

static void *sigrifft_new(void)
{
    t_sigrifft *x = (t_sigrifft *)pd_new(sigrifft_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *sigrifft_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    int n = w[2];
    mayer_realifft(n, in);
    return (w+3);
}

static t_int *sigrifft_flip(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = w[3];
    while (n--)
        *(--out) = - *in++;
    return (w+4);
}

static void sigrifft_dsp(t_sigrifft *x, t_signal **sp)
{
    int n = sp[0]->s_vectorSize, n2 = (n>>1);
    t_sample *in1 = sp[0]->s_vector;
    t_sample *in2 = sp[1]->s_vector;
    t_sample *out1 = sp[2]->s_vector;
    if (n < 4)
    {
        post_error ("fft: minimum 4 points");
        return;
    }
    if (in2 == out1)
    {
        dsp_add(sigrifft_flip, 3, out1+1, out1 + n, n2-1);
        dsp_addCopyPerform (in1, out1, n2+1);
    }
    else
    {
        if (in1 != out1) dsp_addCopyPerform (in1, out1, n2+1);
        dsp_add(sigrifft_flip, 3, in2+1, out1 + n, n2-1);
    }
    dsp_add(sigrifft_perform, 2, out1, n);
}

void rifft_tilde_setup(void)
{
    sigrifft_class = class_new(sym_rifft__tilde__, sigrifft_new, 0,
        sizeof(t_sigrifft), 0, 0);
    CLASS_SIGNAL(sigrifft_class, t_sigrifft, x_f);
    class_addMethod(sigrifft_class, (t_method)sigrifft_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigrifft_class, sym_fft__tilde__);
}
