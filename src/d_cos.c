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

/* ------------------------ cos~ ----------------------------- */

t_float *cos_tilde_table;

static t_class *cos_tilde_class;

typedef struct _cos
{
    t_object x_obj;
    float x_f;
} t_cos;

static void *cos_tilde_new(void)
{
    t_cos *x = (t_cos *)pd_new(cos_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *cos_tilde_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    float *tab = cos_tilde_table, *addr, f1, f2, frac;
    double dphase;
    int normhipart;
    t_rawcast64 tf;
    
    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];

#if 0           /* this is the readable version of the code. */
    while (n--)
    {
        dphase = (double)(*in++ * (float)(COSINE_TABLE_SIZE)) + DSP_UNITBIT32;
        tf.z_d = dphase;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        *out++ = f1 + frac * (f2 - f1);
    }
#endif
#if 1           /* this is the same, unwrapped by hand. */
        dphase = (double)(*in++ * (float)(COSINE_TABLE_SIZE)) + DSP_UNITBIT32;
        tf.z_d = dphase;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    while (--n)
    {
        dphase = (double)(*in++ * (float)(COSINE_TABLE_SIZE)) + DSP_UNITBIT32;
            frac = tf.z_d - DSP_UNITBIT32;
        tf.z_d = dphase;
            f1 = addr[0];
            f2 = addr[1];
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
            *out++ = f1 + frac * (f2 - f1);
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    }
            frac = tf.z_d - DSP_UNITBIT32;
            f1 = addr[0];
            f2 = addr[1];
            *out++ = f1 + frac * (f2 - f1);
#endif
    return (w+4);
}

static void cos_tilde_dsp(t_cos *x, t_signal **sp)
{
    dsp_add(cos_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void cos_tilde_maketable(void)
{
    int i;
    float *fp, phase, phsinc = (2. * PD_PI) / COSINE_TABLE_SIZE;
    t_rawcast64 tf;
    
    if (cos_tilde_table) return;
    cos_tilde_table = (float *)PD_MEMORY_GET(sizeof(float) * (COSINE_TABLE_SIZE+1));
    for (i = COSINE_TABLE_SIZE + 1, fp = cos_tilde_table, phase = 0; i--;
        fp++, phase += phsinc)
            *fp = cos(phase);

        /* here we check at startup whether the byte alignment
            is as we declared it.  If not, the code has to be
            recompiled the other way. */
    tf.z_d = DSP_UNITBIT32 + 0.5;
    if ((unsigned)tf.z_i[PD_RAWCAST64_LSB] != 0x80000000) { PD_BUG; }
}

void cos_tilde_setup(void)
{
    cos_tilde_class = class_new(sym_cos__tilde__, (t_newmethod)cos_tilde_new, 0,
        sizeof(t_cos), 0, A_DEFFLOAT, 0);
    CLASS_SIGNAL(cos_tilde_class, t_cos, x_f);
    class_addMethod(cos_tilde_class, (t_method)cos_tilde_dsp, sym_dsp, A_CANT, 0);
    cos_tilde_maketable();
}
