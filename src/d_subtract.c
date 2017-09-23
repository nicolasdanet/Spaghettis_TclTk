
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

static t_class *subtract_tilde_class;               /* Shared. */
static t_class *subtractScalar_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _subtract_tilde {
    t_object    x_obj;                              /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_subtract_tilde;

typedef struct _subtractscalar_tilde {
    t_object    x_obj;                              /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_subtractscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void subtract_tilde_dsp (t_subtract_tilde *x, t_signal **sp)
{
    dsp_addSubtractPerform (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void subtractScalar_tilde_dsp (t_subtractscalar_tilde *x, t_signal **sp)
{
    dsp_addSubtractScalarPerform (sp[0]->s_vector, &x->x_scalar, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *subtract_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc - 1, argv + 1); }
    
    t_subtractscalar_tilde *x = (t_subtractscalar_tilde *)pd_new (subtractScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newFloat (cast_object (x), &x->x_scalar);
            
    return x;
}

static void *subtract_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_subtract_tilde *x = (t_subtract_tilde *)pd_new (subtract_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));
        
    return x;
}

static void *subtract_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return subtract_tilde_newWithScalar (s, argc, argv); 
    } else {
        return subtract_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void subtract_tilde_setup (void)
{
    subtract_tilde_class = class_new (sym___minus____tilde__,
                                    (t_newmethod)subtract_tilde_new,
                                    NULL,
                                    sizeof (t_subtract_tilde),
                                    CLASS_DEFAULT,
                                    A_GIMME,
                                    A_NULL);
    
    subtractScalar_tilde_class = class_new (sym___minus____tilde__,
                                    NULL,
                                    NULL,
                                    sizeof (t_subtractscalar_tilde),
                                    CLASS_DEFAULT,
                                    A_NULL);
        
    CLASS_SIGNAL (subtract_tilde_class, t_subtract_tilde, x_f);
    CLASS_SIGNAL (subtractScalar_tilde_class, t_subtractscalar_tilde, x_f);
        
    class_addDSP (subtract_tilde_class, (t_method)subtract_tilde_dsp);
    class_addDSP (subtractScalar_tilde_class, (t_method)subtractScalar_tilde_dsp);
        
    class_setHelpName (subtract_tilde_class, sym_arithmetic__tilde__);
    class_setHelpName (subtractScalar_tilde_class, sym_arithmetic__tilde__);
}

void subtract_tilde_destroy (void)
{
    class_free (subtract_tilde_class);
    class_free (subtractScalar_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

