
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    int         x_hasPolling;
    t_outlet    *x_outlet;
    } t_snapshot_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void snapshot_tilde_bang (t_snapshot_tilde *x)
{
    outlet_float (x->x_outlet, x->x_value);
}

static void snapshot_tilde_set (t_snapshot_tilde *x, t_float f)
{
    x->x_value = f;
}

static void snapshot_tilde_polling (t_snapshot_tilde *x)
{
    if (dsp_getState()) { snapshot_tilde_bang (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static void *snapshot_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_snapshot_tilde *x = (t_snapshot_tilde *)pd_new (snapshot_tilde_class);
    
    int n = (atom_getSymbolAtIndex (0, argc, argv) == sym_automatic);
    
    /* For convenience number 1 can be used instead of "automatic" keyword. */
    
    n |= (atom_getFloatAtIndex (0, argc, argv) == 1.0);
    
    if (n) {
        instance_pollingRegister (cast_pd (x));
        x->x_hasPolling = 1;
        argc--; argv++;
    }
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);

    return x;
}

static void snapshot_tilde_free (t_snapshot_tilde *x)
{
    if (x->x_hasPolling) { instance_pollingUnregister (cast_pd (x)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void snapshot_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_snapshot__tilde__,
            (t_newmethod)snapshot_tilde_new,
            (t_method)snapshot_tilde_free,
            sizeof (t_snapshot_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addCreator ((t_newmethod)snapshot_tilde_new, sym_vsnapshot__tilde__, A_GIMME, A_NULL);
    
    CLASS_SIGNAL (c, t_snapshot_tilde, x_f);
    
    class_addDSP (c, (t_method)snapshot_tilde_dsp);
    class_addBang (c, (t_method)snapshot_tilde_bang);
    class_addPolling (c, (t_method)snapshot_tilde_polling);
    
    class_addMethod (c, (t_method)snapshot_tilde_set, sym_set, A_DEFFLOAT, A_NULL);

    snapshot_tilde_class = c;
}

void snapshot_tilde_destroy (void)
{
    class_free (snapshot_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
