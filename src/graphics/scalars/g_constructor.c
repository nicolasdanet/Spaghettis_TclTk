
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../control/x_tinyexpr.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *constructor_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _constructor {
    t_object    x_obj;              /* MUST be the first. */
    int         x_once;
    t_symbol    *x_field;
    t_buffer    *x_buffer;
    te_expr     *x_expression;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *constructor_getField (t_constructor *x)
{
    return x->x_field;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float constructor_evaluateAsFloat (t_constructor *x)
{
    if (x && buffer_getSize (x->x_buffer)) {
    //
    double f = 0.0;
    
    if (!x->x_once) {
    //
    char *t = atom_atomsToString (buffer_getSize (x->x_buffer), buffer_getAtoms (x->x_buffer));
    
    if ((x->x_expression = te_compile (t, NULL, 0)) == NULL) {
        error_invalid (sym_constructor, sym_expression);
    }
    
    PD_MEMORY_FREE (t); x->x_once = 1;
    //
    }
    
    if (x->x_expression) { f = te_eval (x->x_expression); }
    
    return PD_FLOAT64_IS_INVALID_OR_ZERO (f) ? 0.0 : f;
    //
    }
    
    return 0.0;
}

t_symbol *constructor_evaluateAsSymbol (t_constructor *x)
{
    if (x && buffer_getSize (x->x_buffer)) {
        return atom_getSymbol (buffer_getAtomAtIndex (x->x_buffer, 0));
    }
    
    return &s_symbol;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *constructor_new (t_symbol *s, int argc, t_atom *argv)
{
    t_constructor *x = (t_constructor *)pd_new (constructor_class);
    
    x->x_field  = &s_;
    x->x_buffer = buffer_new();
    
    if (argc && IS_SYMBOL (argv)) {
    //
    x->x_field = GET_SYMBOL (argv); argc--; argv++;
    
    if (argc) { buffer_append (x->x_buffer, argc, argv); }
    //
    }
    
    return x;
}

static void constructor_free (t_constructor *x)
{
    if (x->x_expression) { te_free (x->x_expression); }
    
    buffer_free (x->x_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void constructor_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_constructor,
            (t_newmethod)constructor_new,
            (t_method)constructor_free,
            sizeof (t_constructor),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
                
    constructor_class = c;
}

void constructor_destroy (void)
{
    class_free (constructor_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
