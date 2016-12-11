
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -
/* -------------------------- vsnapshot~ ------------------------------ */
static t_class *vsnapshot_tilde_class;

typedef struct _vsnapshot
{
    t_object x_obj;
    int x_n;
    int x_gotone;
    t_sample *x_vec;
    t_float x_f;
    t_float x_sampspermsec;
    double x_time;
} t_vsnapshot;

static void *vsnapshot_tilde_new(void)
{
    t_vsnapshot *x = (t_vsnapshot *)pd_new(vsnapshot_tilde_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_f = 0;
    x->x_n = 0;
    x->x_vec = 0;
    x->x_gotone = 0;
    return x;
}

static t_int *vsnapshot_tilde_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_vsnapshot *x = (t_vsnapshot *)(w[2]);
    t_sample *out = x->x_vec;
    int n = x->x_n, i;
    for (i = 0; i < n; i++)
        out[i] = in[i];
    x->x_time = scheduler_getLogicalTime();
    x->x_gotone = 1;
    return (w+3);
}

static void vsnapshot_tilde_dsp(t_vsnapshot *x, t_signal **sp)
{
    int n = sp[0]->s_vectorSize;
    if (n != x->x_n)
    {
        if (x->x_vec)
            PD_MEMORY_FREE(x->x_vec);
        x->x_vec = (t_sample *)PD_MEMORY_GET(n * sizeof(t_sample));
        x->x_gotone = 0;
        x->x_n = n;
    }
    x->x_sampspermsec = sp[0]->s_sampleRate / 1000;
    dsp_add(vsnapshot_tilde_perform, 2, sp[0]->s_vector, x);
}

static void vsnapshot_tilde_bang(t_vsnapshot *x)
{
    t_sample val;
    if (x->x_gotone)
    {
        int indx = scheduler_getMillisecondsSince(x->x_time) * x->x_sampspermsec;
        if (indx < 0)
            indx = 0;
        else if (indx >= x->x_n)
            indx = x->x_n - 1;
        val = x->x_vec[indx];
    }
    else val = 0;
    outlet_float(x->x_obj.te_outlet, val);
}

static void vsnapshot_tilde_ff(t_vsnapshot *x)
{
    if (x->x_vec)
        PD_MEMORY_FREE(x->x_vec);
}

void vsnapshot_tilde_setup(void)
{
    vsnapshot_tilde_class = class_new(sym_vsnapshot__tilde__,
        (t_newmethod)vsnapshot_tilde_new, (t_method)vsnapshot_tilde_ff,
        sizeof(t_vsnapshot), 0, 0);
    CLASS_SIGNAL(vsnapshot_tilde_class, t_vsnapshot, x_f);
    class_addMethod(vsnapshot_tilde_class, (t_method)vsnapshot_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addBang(vsnapshot_tilde_class, vsnapshot_tilde_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
