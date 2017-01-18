
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
#include "d_fft.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------



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
