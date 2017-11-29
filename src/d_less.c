
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *less_tilde_class;               /* Shared. */
static t_class *lessScalar_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _less_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_less_tilde;

typedef struct _lessscalar_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_lessscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void less_tilde_dsp (t_less_tilde *x, t_signal **sp)
{
    dsp_addLessPerform (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void lessScalar_tilde_dsp (t_lessscalar_tilde *x, t_signal **sp)
{
    dsp_addLessScalarPerform (sp[0]->s_vector, &x->x_scalar, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *less_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc - 1, argv + 1); }
    
    t_lessscalar_tilde *x = (t_lessscalar_tilde *)pd_new (lessScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    inlet_newFloat (cast_object (x), &x->x_scalar);
    
    return x;
}

static void *less_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_less_tilde *x = (t_less_tilde *)pd_new (less_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *less_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return less_tilde_newWithScalar (s, argc, argv);
    } else {
        return less_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void less_tilde_setup (void)
{
    less_tilde_class = class_new (sym___less____tilde__,
                                (t_newmethod)less_tilde_new,
                                NULL,
                                sizeof (t_less_tilde),
                                CLASS_DEFAULT,
                                A_GIMME,
                                A_NULL);
    
    lessScalar_tilde_class = class_new (sym___less____tilde__,
                                NULL,
                                NULL,
                                sizeof (t_lessscalar_tilde),
                                CLASS_DEFAULT,
                                A_NULL);
    
    CLASS_SIGNAL (less_tilde_class, t_less_tilde, x_f);
    CLASS_SIGNAL (lessScalar_tilde_class, t_lessscalar_tilde, x_f);
            
    class_addDSP (less_tilde_class, (t_method)less_tilde_dsp);
    class_addDSP (lessScalar_tilde_class, (t_method)lessScalar_tilde_dsp);
    
    class_setHelpName (less_tilde_class, sym_logical__tilde__);
    class_setHelpName (lessScalar_tilde_class, sym_logical__tilde__);
}

void less_tilde_destroy (void)
{
    class_free (less_tilde_class);
    class_free (lessScalar_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

