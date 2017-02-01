
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *add_tilde_class;            /* Shared. */
static t_class *addScalar_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _add_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_add_tilde;

typedef struct _addscalar_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_addscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void add_tilde_dsp (t_add_tilde *x, t_signal **sp)
{
    dsp_addPlusPerform (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void addScalar_tilde_dsp (t_addscalar_tilde *x, t_signal **sp)
{
    dsp_addPlusScalarPerform (sp[0]->s_vector, &x->x_scalar, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *add_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc + 1, argv - 1); }
    
    t_addscalar_tilde *x = (t_addscalar_tilde *)pd_new (addScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    inlet_newFloat (cast_object (x), &x->x_scalar);
    
    return x;
}

static void *add_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_add_tilde *x = (t_add_tilde *)pd_new (add_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *add_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return add_tilde_newWithScalar (s, argc, argv); 
    } else {
        return add_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void add_tilde_setup (void)
{
    add_tilde_class = class_new (sym___plus____tilde__,
                                (t_newmethod)add_tilde_new,
                                NULL,
                                sizeof (t_add_tilde),
                                CLASS_DEFAULT,
                                A_GIMME,
                                A_NULL);
    
    addScalar_tilde_class = class_new (sym___plus____tilde__,
                                NULL,
                                NULL,
                                sizeof (t_addscalar_tilde),
                                CLASS_DEFAULT,
                                A_NULL);
    
    CLASS_SIGNAL (add_tilde_class, t_add_tilde, x_f);
    CLASS_SIGNAL (addScalar_tilde_class, t_addscalar_tilde, x_f);
            
    class_addDSP (add_tilde_class, (t_method)add_tilde_dsp);
    class_addDSP (addScalar_tilde_class, (t_method)addScalar_tilde_dsp);
    
    class_setHelpName (add_tilde_class, sym_max__tilde__);
    class_setHelpName (addScalar_tilde_class, sym_max__tilde__);
}

void add_tilde_destroy (void)
{
    CLASS_FREE (add_tilde_class);
    CLASS_FREE (addScalar_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

