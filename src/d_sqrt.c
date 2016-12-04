
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

extern float rsqrt_exptab[];
extern float rsqrt_mantissatab[];

/* sigsqrt -  square root good to 8 mantissa bits  */

typedef struct sigsqrt
{
    t_object x_obj;
    t_float x_f;
} t_sigsqrt;

static t_class *sigsqrt_class;

static void *sigsqrt_new(void)
{
    t_sigsqrt *x = (t_sigsqrt *)pd_new(sigsqrt_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

t_int *sigsqrt_perform(t_int *w)    /* not static; also used in d_fft.c */
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {   
        t_sample f = *in++;
        union {
          float f;
          long l;
        } u;
        u.f = f;
        if (f < 0) *out++ = 0;
        else
        {
            t_sample g = rsqrt_exptab[(u.l >> 23) & 0xff] *
                rsqrt_mantissatab[(u.l >> 13) & 0x3ff];
            *out++ = f * (1.5 * g - 0.5 * g * g * g * f);
        }
    }
    return (w + 4);
}

static void sigsqrt_dsp(t_sigsqrt *x, t_signal **sp)
{
    dsp_add(sigsqrt_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void sigsqrt_tilde_setup(void)
{
    sigsqrt_class = class_new(sym_sqrt__tilde__, (t_newmethod)sigsqrt_new, 0,
        sizeof(t_sigsqrt), 0, 0);
    class_addCreator(sigsqrt_new, sym_q8_sqrt__tilde__, 0);   /* LEGACY !!! */
    CLASS_SIGNAL(sigsqrt_class, t_sigsqrt, x_f);
    class_addMethod(sigsqrt_class, (t_method)sigsqrt_dsp,
        sym_dsp, A_CANT, 0);
}
