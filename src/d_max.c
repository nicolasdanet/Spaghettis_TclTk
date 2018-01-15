
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

static t_class *max_tilde_class;                /* Shared. */
static t_class *maxScalar_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _max_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_max_tilde;

typedef struct _maxscalar_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_maxscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void max_tilde_dsp (t_max_tilde *x, t_signal **sp)
{
    dsp_addMaximumPerform (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void maxScalar_tilde_dsp (t_maxscalar_tilde *x, t_signal **sp)
{
    dsp_addMaximumScalarPerform (sp[0]->s_vector, &x->x_scalar, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *max_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc - 1, argv + 1); }
    
    t_maxscalar_tilde *x = (t_maxscalar_tilde *)pd_new (maxScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newFloat (cast_object (x), &x->x_scalar);
            
    return x;
}

static void *max_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_max_tilde *x = (t_max_tilde *)pd_new (max_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

static void *max_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return max_tilde_newWithScalar (s, argc, argv);
    } else {
        return max_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void max_tilde_setup (void)
{
    max_tilde_class = class_new (sym_max__tilde__,
                                    (t_newmethod)max_tilde_new,
                                    NULL,
                                    sizeof (t_max_tilde),
                                    CLASS_DEFAULT,
                                    A_GIMME,
                                    A_NULL);
    
    maxScalar_tilde_class = class_new (sym_max__tilde__,
                                    NULL,
                                    NULL,
                                    sizeof (t_maxscalar_tilde),
                                    CLASS_DEFAULT,
                                    A_NULL);
        
    CLASS_SIGNAL (max_tilde_class, t_max_tilde, x_f);
    CLASS_SIGNAL (maxScalar_tilde_class, t_maxscalar_tilde, x_f);
        
    class_addDSP (max_tilde_class, (t_method)max_tilde_dsp);
    class_addDSP (maxScalar_tilde_class, (t_method)maxScalar_tilde_dsp);
    
    class_setHelpName (max_tilde_class, sym_math__tilde__);
    class_setHelpName (maxScalar_tilde_class, sym_math__tilde__);
}

void max_tilde_destroy (void)
{
    class_free (max_tilde_class);
    class_free (maxScalar_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

