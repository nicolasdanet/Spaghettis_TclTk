
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
#include "s_system.h"
#include "d_dsp.h"

/* -------------------------- snapshot~ ------------------------------ */
static t_class *snapshot_tilde_class;

typedef struct _snapshot
{
    t_object x_obj;
    t_sample x_value;
    t_float x_f;
} t_snapshot;

static void *snapshot_tilde_new(void)
{
    t_snapshot *x = (t_snapshot *)pd_new(snapshot_tilde_class);
    x->x_value = 0;
    outlet_new(&x->x_obj, &s_float);
    x->x_f = 0;
    return x;
}

static t_int *snapshot_tilde_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    *out = *in;
    return (w+3);
}

static void snapshot_tilde_dsp(t_snapshot *x, t_signal **sp)
{
    dsp_add(snapshot_tilde_perform, 2, sp[0]->s_vector + (sp[0]->s_vectorSize-1),
        &x->x_value);
}

static void snapshot_tilde_bang(t_snapshot *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_value);
}

static void snapshot_tilde_set(t_snapshot *x, t_float f)
{
    x->x_value = f;
}

void snapshot_tilde_setup(void)
{
    snapshot_tilde_class = class_new(sym_snapshot__tilde__, snapshot_tilde_new, 0,
        sizeof(t_snapshot), 0, 0);
    CLASS_SIGNAL(snapshot_tilde_class, t_snapshot, x_f);
    class_addMethod(snapshot_tilde_class, (t_method)snapshot_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(snapshot_tilde_class, (t_method)snapshot_tilde_set,
        sym_set, A_DEFFLOAT, 0);
    class_addBang(snapshot_tilde_class, snapshot_tilde_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
