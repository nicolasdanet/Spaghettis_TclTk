
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
#include "g_graphics.h"
#include "d_dsp.h"

extern t_class *garray_class;

static t_class *tabosc4_tilde_class;

typedef struct _tabosc4_tilde
{
    t_object x_obj;
    t_float x_fnpoints;
    t_float x_finvnpoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
    double x_phase;
    t_float x_conv;
} t_tabosc4_tilde;

static void *tabosc4_tilde_new(t_symbol *s)
{
    t_tabosc4_tilde *x = (t_tabosc4_tilde *)pd_new(tabosc4_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    x->x_fnpoints = 512.;
    x->x_finvnpoints = (1./512.);
    outlet_new(&x->x_obj, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    x->x_f = 0;
    return x;
}

static t_int *tabosc4_tilde_perform(t_int *w)
{
    t_tabosc4_tilde *x = (t_tabosc4_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    int normhipart;
    t_rawcast64 tf;
    t_float fnpoints = x->x_fnpoints;
    int mask = fnpoints - 1;
    t_float conv = fnpoints * x->x_conv;
    int maxindex;
    t_word *tab = x->x_vec, *addr;
    int i;
    double dphase = fnpoints * x->x_phase + DSP_UNITBIT;

    if (!tab) goto zero;
    tf.z_d = DSP_UNITBIT;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];

#if 1
    while (n--)
    {
        t_sample frac,  a,  b,  c,  d, cminusb;
        tf.z_d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & mask);
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT;
        a = addr[0].w_float;
        b = addr[1].w_float;
        c = addr[2].w_float;
        d = addr[3].w_float;
        cminusb = c-b;
        *out++ = b + frac * (
            cminusb - 0.1666667f * (1.-frac) * (
                (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
            )
        );
    }
#endif

    tf.z_d = DSP_UNITBIT * fnpoints;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];
    tf.z_d = dphase + (DSP_UNITBIT * fnpoints - DSP_UNITBIT);
    tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    x->x_phase = (tf.z_d - DSP_UNITBIT * fnpoints)  * x->x_finvnpoints;
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

static void tabosc4_tilde_set(t_tabosc4_tilde *x, t_symbol *s)
{
    t_garray *a;
    int npoints, pointsinarray;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            post_error ("tabosc4~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getData(a, &pointsinarray, &x->x_vec)) /* Always true now !!! */
    {
        post_error ("%s: bad template for tabosc4~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if ((npoints = pointsinarray - 3) != (1 << math_ilog2 (pointsinarray - 3)))
    {
        post_error ("%s: number of points (%d) not a power of 2 plus three",
            x->x_arrayname->s_name, pointsinarray);
        x->x_vec = 0;
        garray_setAsUsedInDSP(a);
    }
    else
    {
        x->x_fnpoints = npoints;
        x->x_finvnpoints = 1./npoints;
        garray_setAsUsedInDSP(a);
    }
}

static void tabosc4_tilde_ft1(t_tabosc4_tilde *x, t_float f)
{
    x->x_phase = f;
}

static void tabosc4_tilde_dsp(t_tabosc4_tilde *x, t_signal **sp)
{
    x->x_conv = 1. / sp[0]->s_sampleRate;
    tabosc4_tilde_set(x, x->x_arrayname);

    dsp_add(tabosc4_tilde_perform, 4, x,
        sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void tabosc4_tilde_setup(void)
{
    tabosc4_tilde_class = class_new(sym_tabosc4__tilde__,
        (t_newmethod)tabosc4_tilde_new, 0,
        sizeof(t_tabosc4_tilde), 0, A_DEFSYMBOL, 0);
    CLASS_SIGNAL(tabosc4_tilde_class, t_tabosc4_tilde, x_f);
    class_addMethod(tabosc4_tilde_class, (t_method)tabosc4_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(tabosc4_tilde_class, (t_method)tabosc4_tilde_set,
        sym_set, A_SYMBOL, 0);
    class_addMethod(tabosc4_tilde_class, (t_method)tabosc4_tilde_ft1,
        sym_inlet2, A_FLOAT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
