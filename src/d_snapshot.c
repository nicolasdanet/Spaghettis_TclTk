
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *snapshot_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _snapshot_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_sample    x_value;
    t_outlet    *x_outlet;
    } t_snapshot_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void snapshot_tilde_bang (t_snapshot_tilde *x)
{
    outlet_float (x->x_outlet, x->x_value);
}

static void snapshot_tilde_set (t_snapshot_tilde *x, t_float f)
{
    x->x_value = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *snapshot_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    
    *out = *in;
    
    return (w + 3);
}

static void snapshot_tilde_dsp (t_snapshot_tilde *x, t_signal **sp)
{
    dsp_add (snapshot_tilde_perform, 2, sp[0]->s_vector + (sp[0]->s_vectorSize - 1), &x->x_value);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *snapshot_tilde_new (void)
{
    t_snapshot_tilde *x = (t_snapshot_tilde *)pd_new (snapshot_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void snapshot_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_snapshot__tilde__,
            (t_newmethod)snapshot_tilde_new,
            NULL,
            sizeof (t_snapshot_tilde),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addCreator ((t_newmethod)snapshot_tilde_new, sym_vsnapshot__tilde__, A_NULL);
    
    CLASS_SIGNAL (c, t_snapshot_tilde, x_f);
    
    class_addDSP (c, (t_method)snapshot_tilde_dsp);
    class_addBang (c, (t_method)snapshot_tilde_bang);
        
    class_addMethod (c, (t_method)snapshot_tilde_set, sym_set, A_DEFFLOAT, A_NULL);

    snapshot_tilde_class = c;
}

void snapshot_tilde_destroy (void)
{
    CLASS_FREE (snapshot_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
