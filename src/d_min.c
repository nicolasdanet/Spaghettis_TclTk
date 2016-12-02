
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* ----------------------------- min ----------------------------- */
static t_class *min_class, *scalarmin_class;

typedef struct _min
{
    t_object x_obj;
    t_float x_f;
} t_min;

typedef struct _scalarmin
{
    t_object x_obj;
    t_float x_g;
    t_float x_f;
} t_scalarmin;

static void *min_new(t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) post("min~: extra arguments ignored");
    if (argc) 
    {
        t_scalarmin *x = (t_scalarmin *)pd_new(scalarmin_class);
        inlet_newFloat(&x->x_obj, &x->x_g);
        x->x_g = atom_getFloatAtIndex(0, argc, argv);
        outlet_new(&x->x_obj, &s_signal);
        x->x_f = 0;
        return (x);
    }
    else
    {
        t_min *x = (t_min *)pd_new(min_class);
        inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
        outlet_new(&x->x_obj, &s_signal);
        x->x_f = 0;
        return (x);
    }
}

t_int *min_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
        t_sample f = *in1++, g = *in2++;
        *out++ = (f < g ? f : g); 
    }
    return (w+5);
}

t_int *min_perf8(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
        t_sample f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
        t_sample f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

        t_sample g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
        t_sample g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];

        out[0] = (f0 < g0 ? f0 : g0); out[1] = (f1 < g1 ? f1 : g1);
        out[2] = (f2 < g2 ? f2 : g2); out[3] = (f3 < g3 ? f3 : g3);
        out[4] = (f4 < g4 ? f4 : g4); out[5] = (f5 < g5 ? f5 : g5);
        out[6] = (f6 < g6 ? f6 : g6); out[7] = (f7 < g7 ? f7 : g7);
    }
    return (w+5);
}

t_int *scalarmin_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
        t_sample g = *in++;
        *out++ = (f < g ? f : g); 
    }
    return (w+5);
}

t_int *scalarmin_perf8(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in += 8, out += 8)
    {
        t_sample f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
        t_sample f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

        out[0] = (f0 < g ? f0 : g); out[1] = (f1 < g ? f1 : g);
        out[2] = (f2 < g ? f2 : g); out[3] = (f3 < g ? f3 : g);
        out[4] = (f4 < g ? f4 : g); out[5] = (f5 < g ? f5 : g);
        out[6] = (f6 < g ? f6 : g); out[7] = (f7 < g ? f7 : g);
    }
    return (w+5);
}

static void min_dsp(t_min *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize&7)
        dsp_add(min_perform, 4,
            sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
    else        
        dsp_add(min_perf8, 4,
            sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void scalarmin_dsp(t_scalarmin *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize&7)
        dsp_add(scalarmin_perform, 4, sp[0]->s_vector, &x->x_g,
            sp[1]->s_vector, sp[0]->s_vectorSize);
    else        
        dsp_add(scalarmin_perf8, 4, sp[0]->s_vector, &x->x_g,
            sp[1]->s_vector, sp[0]->s_vectorSize);
}

void min_tilde_setup(void)
{
    min_class = class_new(sym_min__tilde__, (t_newmethod)min_new, 0,
        sizeof(t_min), 0, A_GIMME, 0);
    CLASS_SIGNAL(min_class, t_min, x_f);
    class_addMethod(min_class, (t_method)min_dsp, sym_dsp, A_CANT, 0);
    class_setHelpName(min_class, sym_max__tilde__);
    scalarmin_class = class_new(sym_min__tilde__, 0, 0,
        sizeof(t_scalarmin), 0, 0);
    CLASS_SIGNAL(scalarmin_class, t_scalarmin, x_f);
    class_addMethod(scalarmin_class, (t_method)scalarmin_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(scalarmin_class, sym_max__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

