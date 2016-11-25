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
#include "d_osc.h"

/* ------------------------ osc~ ----------------------------- */

extern t_float *cos_tilde_table;

static t_class *osc_class, *scalarosc_class;

typedef struct _osc
{
    t_object x_obj;
    double x_phase;
    float x_conv;
    float x_f;      /* frequency if scalar */
} t_osc;

static void *osc_new(t_float f)
{
    t_osc *x = (t_osc *)pd_new(osc_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    x->x_phase = 0;
    x->x_conv = 0;
    return (x);
}

static t_int *osc_perform(t_int *w)
{
    t_osc *x = (t_osc *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    float *tab = cos_tilde_table, *addr, f1, f2, frac;
    double dphase = x->x_phase + DSP_UNITBIT32;
    int normhipart;
    t_rawcast64 tf;
    float conv = x->x_conv;
    
    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];
#if 0
    while (n--)
    {
        tf.z_d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        *out++ = f1 + frac * (f2 - f1);
    }
#endif
#if 1
        tf.z_d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
    while (--n)
    {
        tf.z_d = dphase;
            f1 = addr[0];
        dphase += *in++ * conv;
            f2 = addr[1];
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
            *out++ = f1 + frac * (f2 - f1);
        frac = tf.z_d - DSP_UNITBIT32;
    }
            f1 = addr[0];
            f2 = addr[1];
            *out++ = f1 + frac * (f2 - f1);
#endif

    tf.z_d = DSP_UNITBIT32 * COSINE_TABLE_SIZE;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];
    tf.z_d = dphase + (DSP_UNITBIT32 * COSINE_TABLE_SIZE - DSP_UNITBIT32);
    tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    x->x_phase = tf.z_d - DSP_UNITBIT32 * COSINE_TABLE_SIZE;
    return (w+5);
}

static void osc_dsp(t_osc *x, t_signal **sp)
{
    x->x_conv = COSINE_TABLE_SIZE/sp[0]->s_sampleRate;
    dsp_add(osc_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

static void osc_ft1(t_osc *x, t_float f)
{
    x->x_phase = COSINE_TABLE_SIZE * f;
}

void osc_setup(void)
{    
    osc_class = class_new(sym_osc__tilde__, (t_newmethod)osc_new, 0,
        sizeof(t_osc), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(osc_class, t_osc, x_f);
    class_addMethod(osc_class, (t_method)osc_dsp, sym_dsp, A_CANT, 0);
    class_addMethod(osc_class, (t_method)osc_ft1, sym_inlet2, A_FLOAT, 0);

    cos_tilde_maketable();
}
