
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *multiply_tilde_class;               /* Shared. */
static t_class *multiplyScalar_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _multiply_tilde {
    t_object    x_obj;                              /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_multiply_tilde;

typedef struct _multiplyscalar_tilde {
    t_object    x_obj;                              /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    } t_multiplyscalar_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void multiply_tilde_dsp (t_multiply_tilde *x, t_signal **sp)
{
    dsp_addMultiplyPerform (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

static void multiplyScalar_tilde_dsp (t_multiplyscalar_tilde *x, t_signal **sp)
{
    dsp_addMultiplyScalarPerform (sp[0]->s_vector, &x->x_scalar, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *multiply_tilde_newWithScalar (t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { warning_unusedArguments (s, argc + 1, argv - 1); }
    
    t_multiplyscalar_tilde *x = (t_multiplyscalar_tilde *)pd_new (multiplyScalar_tilde_class);
    
    x->x_scalar = atom_getFloatAtIndex (0, argc, argv);
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
        
    inlet_newFloat (cast_object (x), &x->x_scalar);

    return x;
}

static void *multiply_tilde_newWithSignal (t_symbol *s, int argc, t_atom *argv)
{
    t_multiply_tilde *x = (t_multiply_tilde *)pd_new (multiply_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newSignal (cast_object (x));
        
    return x;
}

static void *multiply_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
        return multiply_tilde_newWithScalar (s, argc, argv);
    } else {
        return multiply_tilde_newWithSignal (s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void multiply_tilde_setup (void)
{
    multiply_tilde_class = class_new (sym___asterisk____tilde__,
                                    (t_newmethod)multiply_tilde_new,
                                    NULL,
                                    sizeof (t_multiply_tilde),
                                    CLASS_DEFAULT,
                                    A_GIMME,
                                    A_NULL);
    
    multiplyScalar_tilde_class = class_new (sym___asterisk____tilde__,
                                    NULL,
                                    NULL,
                                    sizeof (t_multiplyscalar_tilde),
                                    CLASS_DEFAULT,
                                    A_NULL);
        
    CLASS_SIGNAL (multiply_tilde_class, t_multiply_tilde, x_f);
    CLASS_SIGNAL (multiplyScalar_tilde_class, t_multiplyscalar_tilde, x_f);
        
    class_addDSP (multiply_tilde_class, (t_method)multiply_tilde_dsp);
    class_addDSP (multiplyScalar_tilde_class, (t_method)multiplyScalar_tilde_dsp);
        
    class_setHelpName (multiply_tilde_class, sym_max__tilde__);
    class_setHelpName (multiplyScalar_tilde_class, sym_max__tilde__);
}

void multiply_tilde_destroy (void)
{
    CLASS_FREE (multiply_tilde_class);
    CLASS_FREE (multiplyScalar_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

