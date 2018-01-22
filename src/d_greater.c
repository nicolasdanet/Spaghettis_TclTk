
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *greater_tilde_class;            /* Shared. */
static t_class *greaterScalar_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _greater_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_greater_tilde;

typedef struct _greaterscalar_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_greaterscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void greater_tilde_dsp (t_greater_tilde *x, t_signal **sp)
{
    dsp_addGreaterPerformAliased (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void greaterScalar_tilde_dsp (t_greaterscalar_tilde *x, t_signal **sp)
{
    dsp_addGreaterScalarPerform (sp[0]->s_vector, &x->x_scalar, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *greater_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc - 1, argv + 1); }
    
    t_greaterscalar_tilde *x = (t_greaterscalar_tilde *)pd_new (greaterScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    inlet_newFloat (cast_object (x), &x->x_scalar);
    
    return x;
}

static void *greater_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_greater_tilde *x = (t_greater_tilde *)pd_new (greater_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *greater_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return greater_tilde_newWithScalar (s, argc, argv);
    } else {
        return greater_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void greater_tilde_setup (void)
{
    greater_tilde_class = class_new (sym___greater____tilde__,
                                    (t_newmethod)greater_tilde_new,
                                    NULL,
                                    sizeof (t_greater_tilde),
                                    CLASS_DEFAULT,
                                    A_GIMME,
                                    A_NULL);
    
    greaterScalar_tilde_class = class_new (sym___greater____tilde__,
                                    NULL,
                                    NULL,
                                    sizeof (t_greaterscalar_tilde),
                                    CLASS_DEFAULT,
                                    A_NULL);
    
    CLASS_SIGNAL (greater_tilde_class, t_greater_tilde, x_f);
    CLASS_SIGNAL (greaterScalar_tilde_class, t_greaterscalar_tilde, x_f);
            
    class_addDSP (greater_tilde_class, (t_method)greater_tilde_dsp);
    class_addDSP (greaterScalar_tilde_class, (t_method)greaterScalar_tilde_dsp);
    
    class_setHelpName (greater_tilde_class, sym_logical__tilde__);
    class_setHelpName (greaterScalar_tilde_class, sym_logical__tilde__);
}

void greater_tilde_destroy (void)
{
    class_free (greater_tilde_class);
    class_free (greaterScalar_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

