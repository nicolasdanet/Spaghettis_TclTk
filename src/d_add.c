
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

static t_class *add_tilde_class;            /* Shared. */
static t_class *addScalar_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _add_tilde
{
    t_object x_obj;
    t_float x_f;
} t_add_tilde;

typedef struct _addscalar_tilde
{
    t_object x_obj;
    t_float x_f;
    t_float x_g;            /* inlet value */
} t_addscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int *scalarplus_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in++ + f; 
    return (w+5);
}

t_int *scalarplus_perf8(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in += 8, out += 8)
    {
        t_sample f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
        t_sample f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

        out[0] = f0 + g; out[1] = f1 + g; out[2] = f2 + g; out[3] = f3 + g;
        out[4] = f4 + g; out[5] = f5 + g; out[6] = f6 + g; out[7] = f7 + g;
    }
    return (w+5);
}

static void add_tilde_dsp(t_add_tilde *x, t_signal **sp)
{
    dsp_addPlusPerform(sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void addScalar_tilde_dsp(t_addscalar_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize&7)
        dsp_add(scalarplus_perform, 4, sp[0]->s_vector, &x->x_g,
            sp[1]->s_vector, sp[0]->s_vectorSize);
    else        
        dsp_add(scalarplus_perf8, 4, sp[0]->s_vector, &x->x_g,
            sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *add_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) post("+~: extra arguments ignored");
    if (argc) 
    {
        t_addscalar_tilde *x = (t_addscalar_tilde *)pd_new(addScalar_tilde_class);
        inlet_newFloat(&x->x_obj, &x->x_g);
        x->x_g = atom_getFloatAtIndex(0, argc, argv);
        outlet_new(&x->x_obj, &s_signal);
        x->x_f = 0;
        return (x);
    }
    else
    {
        t_add_tilde *x = (t_add_tilde *)pd_new(add_tilde_class);
        inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
        outlet_new(&x->x_obj, &s_signal);
        x->x_f = 0;
        return (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void add_tilde_setup (void)
{
    add_tilde_class = class_new(sym___plus____tilde__, (t_newmethod)add_tilde_new, 0,
        sizeof(t_add_tilde), 0, A_GIMME, 0);
    class_addMethod(add_tilde_class, (t_method)add_tilde_dsp, sym_dsp, A_CANT, 0);
    CLASS_SIGNAL(add_tilde_class, t_add_tilde, x_f);
    class_setHelpName(add_tilde_class, sym_max__tilde__);
    addScalar_tilde_class = class_new(sym___plus____tilde__, 0, 0,
        sizeof(t_addscalar_tilde), 0, 0);
    CLASS_SIGNAL(addScalar_tilde_class, t_addscalar_tilde, x_f);
    class_addMethod(addScalar_tilde_class, (t_method)addScalar_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(addScalar_tilde_class, sym_max__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

