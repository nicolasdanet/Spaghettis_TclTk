
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *min_tilde_class;                /* Shared. */
static t_class *minScalar_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _min_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_min_tilde;

typedef struct _minscalar_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_minscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void min_tilde_dsp (t_min_tilde *x, t_signal **sp)
{
    dsp_addMinimumPerformAliased (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void minScalar_tilde_dsp (t_minscalar_tilde *x, t_signal **sp)
{
    dsp_addMinimumScalarPerform (sp[0]->s_vector, &x->x_scalar, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *min_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc - 1, argv + 1); }
    
    t_minscalar_tilde *x = (t_minscalar_tilde *)pd_new (minScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_newSignal (cast_object (x));
        
    inlet_newFloat (cast_object (x), &x->x_scalar);

    return x;
}

static void *min_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_min_tilde *x = (t_min_tilde *)pd_new (min_tilde_class);

    x->x_outlet = outlet_newSignal (cast_object (x));

    inlet_newSignal (cast_object (x));
        
    return x;
}

static void *min_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return min_tilde_newWithScalar (s, argc, argv);
    } else {
        return min_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void min_tilde_setup (void)
{
    min_tilde_class = class_new (sym_min__tilde__,
                                    (t_newmethod)min_tilde_new,
                                    NULL,
                                    sizeof (t_min_tilde),
                                    CLASS_DEFAULT,
                                    A_GIMME,
                                    A_NULL);
                    
    minScalar_tilde_class = class_new (sym_min__tilde__,
                                    NULL,
                                    NULL,
                                    sizeof (t_minscalar_tilde),
                                    CLASS_DEFAULT,
                                    A_NULL);
        
    CLASS_SIGNAL (min_tilde_class, t_min_tilde, x_f);
    CLASS_SIGNAL (minScalar_tilde_class, t_minscalar_tilde, x_f);

    class_addDSP (min_tilde_class, (t_method)min_tilde_dsp);
    class_addDSP (minScalar_tilde_class, (t_method)minScalar_tilde_dsp);
    
    class_setHelpName (min_tilde_class, sym_math__tilde__);
    class_setHelpName (minScalar_tilde_class, sym_math__tilde__);
}

void min_tilde_destroy (void)
{
    class_free (min_tilde_class);
    class_free (minScalar_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

