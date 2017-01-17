
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

/* ------------------------ fft~ and ifft~ -------------------------------- */
static t_class *sigfft_class, *sigifft_class;

typedef struct fft
{
    t_object x_obj;
    t_float x_f;
} t_sigfft;

static void *sigfft_new(void)
{
    t_sigfft *x = (t_sigfft *)pd_new(sigfft_class);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    x->x_f = 0;
    return x;
}

static void *sigifft_new(void)
{
    t_sigfft *x = (t_sigfft *)pd_new(sigifft_class);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *sigfft_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    int n = w[3];
    mayer_fft(n, in1, in2);
    return (w+4);
}

static t_int *sigifft_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    int n = w[3];
    mayer_ifft(n, in1, in2);
    return (w+4);
}

static t_int *sigfft_swap(t_int *w)
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

static void sigfft_dspx(t_sigfft *x, t_signal **sp, t_int *(*f)(t_int *w))
{
    int n = sp[0]->s_vectorSize;
    t_sample *in1 = sp[0]->s_vector;
    t_sample *in2 = sp[1]->s_vector;
    t_sample *out1 = sp[2]->s_vector;
    t_sample *out2 = sp[3]->s_vector;
    if (out1 == in2 && out2 == in1)
        dsp_add(sigfft_swap, 3, out1, out2, n);
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

static void sigfft_dsp(t_sigfft *x, t_signal **sp)
{
    sigfft_dspx(x, sp, sigfft_perform);
}

static void sigifft_dsp(t_sigfft *x, t_signal **sp)
{
    sigfft_dspx(x, sp, sigifft_perform);
}

void fft_tilde_setup(void)
{
    sigfft_class = class_new(sym_fft__tilde__, sigfft_new, 0,
        sizeof(t_sigfft), 0, 0);
    CLASS_SIGNAL(sigfft_class, t_sigfft, x_f);
    class_addMethod(sigfft_class, (t_method)sigfft_dsp,
        sym_dsp, A_CANT, 0);

    sigifft_class = class_new(sym_ifft__tilde__, sigifft_new, 0,
        sizeof(t_sigfft), 0, 0);
    CLASS_SIGNAL(sigifft_class, t_sigfft, x_f);
    class_addMethod(sigifft_class, (t_method)sigifft_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigifft_class, sym_fft__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------------------- rfft~ -------------------------------- */

static t_class *sigrfft_class;

typedef struct rfft
{
    t_object x_obj;
    t_float x_f;
} t_sigrfft;

static void *sigrfft_new(void)
{
    t_sigrfft *x = (t_sigrfft *)pd_new(sigrfft_class);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *sigrfft_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    int n = w[2];
    mayer_realfft(n, in);
    return (w+3);
}

static t_int *sigrfft_flip(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = w[3];
    while (n--)
        *(--out) = - *in++;
    return (w+4);
}

static void sigrfft_dsp(t_sigrfft *x, t_signal **sp)
{
    int n = sp[0]->s_vectorSize, n2 = (n>>1);
    t_sample *in1 = sp[0]->s_vector;
    t_sample *out1 = sp[1]->s_vector;
    t_sample *out2 = sp[2]->s_vector;
    if (n < 4)
    {
        post_error ("fft: minimum 4 points");
        return;
    }
    if (in1 != out1)
        dsp_addCopyPerform (in1, out1, n);
    dsp_add(sigrfft_perform, 2, out1, n);
    dsp_add(sigrfft_flip, 3, out1 + (n2+1), out2 + n2, n2-1);
    dsp_addZeroPerform(out1 + (n2+1), ((n2-1)&(~7)));
    dsp_addZeroPerform(out1 + (n2+1) + ((n2-1)&(~7)), ((n2-1)&7));
    dsp_addZeroPerform(out2 + n2, n2);
    dsp_addZeroPerform(out2, 1);
}

void rfft_tilde_setup(void)
{
    sigrfft_class = class_new(sym_rfft__tilde__, sigrfft_new, 0,
        sizeof(t_sigrfft), 0, 0);
    CLASS_SIGNAL(sigrfft_class, t_sigrfft, x_f);
    class_addMethod(sigrfft_class, (t_method)sigrfft_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigrfft_class, sym_fft__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
