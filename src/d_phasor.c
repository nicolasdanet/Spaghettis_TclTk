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

/* -------------------------- phasor~ ------------------------------ */
static t_class *phasor_class;

#if 1   /* in the style of R. Hoeldrich (ICMC 1995 Banff) */

typedef struct _phasor
{
    t_object x_obj;
    double x_phase;
    float x_conv;
    float x_f;      /* scalar frequency */
} t_phasor;

static void *phasor_new(t_float f)
{
    t_phasor *x = (t_phasor *)pd_new(phasor_class);
    x->x_f = f;
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    x->x_phase = 0;
    x->x_conv = 0;
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *phasor_perform(t_int *w)
{
    t_phasor *x = (t_phasor *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    double dphase = x->x_phase + (double)DSP_UNITBIT32;
    t_rawcast64 tf;
    int normhipart;
    float conv = x->x_conv;

    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];
    tf.z_d = dphase;

    while (n--)
    {
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        dphase += *in++ * conv;
        *out++ = tf.z_d - DSP_UNITBIT32;
        tf.z_d = dphase;
    }
    tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    x->x_phase = tf.z_d - DSP_UNITBIT32;
    return (w+5);
}

static void phasor_dsp(t_phasor *x, t_signal **sp)
{
    x->x_conv = 1./sp[0]->s_sampleRate;
    dsp_add(phasor_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

static void phasor_ft1(t_phasor *x, t_float f)
{
    x->x_phase = f;
}

void phasor_setup(void)
{
    phasor_class = class_new(sym_phasor__tilde__, (t_newmethod)phasor_new, 0,
        sizeof(t_phasor), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(phasor_class, t_phasor, x_f);
    class_addMethod(phasor_class, (t_method)phasor_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(phasor_class, (t_method)phasor_ft1,
        sym_inlet2, A_FLOAT, 0);
}

#endif  /* Hoeldrich version */
