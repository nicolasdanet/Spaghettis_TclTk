/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* sinusoidal oscillator and table lookup; see also tabosc4~ in d_array.c.
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "math.h"
#include "d_dsp.h"

static t_class *noise_class;

typedef struct _noise
{
    t_object x_obj;
    int x_val;
} t_noise;

static void *noise_new(void)
{
    t_noise *x = (t_noise *)pd_new(noise_class);
    static int init = 307;
    x->x_val = (init *= 1319); 
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *noise_perform(t_int *w)               /* Weird LCG kept for compatbility. */
{
    t_sample *out = (t_sample *)(w[1]);
    int *vp = (int *)(w[2]);
    int n = (int)(w[3]);
    int val = *vp;
    while (n--)
    {
        *out++ = ((float)((val & PD_INT_MAX) - 0x40000000)) *
            (float)(1.0 / 0x40000000);
        val = val * 435898247 + 382842987;
    }
    *vp = val;
    return (w+4);
}

static void noise_dsp(t_noise *x, t_signal **sp)
{
    dsp_add(noise_perform, 3, sp[0]->s_vector, &x->x_val, sp[0]->s_vectorSize);
}

void noise_setup(void)
{
    noise_class = class_new(sym_noise__tilde__, (t_newmethod)noise_new, 0,
        sizeof(t_noise), 0, 0);
    class_addMethod(noise_class, (t_method)noise_dsp, sym_dsp, A_CANT, 0);
}


