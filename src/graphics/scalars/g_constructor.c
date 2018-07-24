
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

t_class *constructor_class;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_TINYEXPR

#include "../../control/tinyexpr.h"
#include "../../control/math/x_expr.h"

#endif // PD_WITH_TINYEXPR

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _constructor {
    t_object    x_obj;             /* MUST be the first. */
    int         x_once;
    t_symbol    *x_field;
    t_buffer    *x_buffer;
#if PD_WITH_TINYEXPR
    te_variable x_variables[EXPR_FUNCTIONS];
    te_expr     *x_expression;
#endif
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

#if PD_WITH_TINYEXPR

t_float constructor_evaluateAsFloat (t_constructor *x)
{
    if (x && buffer_getSize (x->x_buffer)) {
    //
    double f = 0.0;
    
    if (!x->x_once) {
    //
    int err; char *t = buffer_toString (x->x_buffer);
    
    string_removeCharacter (t, '\\');

    if ((x->x_expression = te_compile (t, x->x_variables, EXPR_FUNCTIONS, &err)) == NULL) {
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

#else

t_float constructor_evaluateAsFloat (t_constructor *x)
{
    if (x && buffer_getSize (x->x_buffer)) {
        return atom_getFloat (buffer_getAtomAtIndex (x->x_buffer, 0));
    }
    
    return 0.0;
}

#endif // PD_WITH_TINYEXPR

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
    
    #if PD_WITH_TINYEXPR
    
    EXPR_TE_FUNCTION (0, "rand",    (const void *)expr_functionRandom,          TE_FUNCTION0);
    EXPR_TE_FUNCTION (1, "randmt",  (const void *)expr_functionRandomMT,        TE_FUNCTION0);
    EXPR_TE_FUNCTION (2, "min",     (const void *)expr_functionMinimum,         TE_FUNCTION2);
    EXPR_TE_FUNCTION (3, "max",     (const void *)expr_functionMaximum,         TE_FUNCTION2);
    EXPR_TE_FUNCTION (4, "eq",      (const void *)expr_functionEqual,           TE_FUNCTION2);
    EXPR_TE_FUNCTION (5, "ne",      (const void *)expr_functionUnequal,         TE_FUNCTION2);
    EXPR_TE_FUNCTION (6, "lt",      (const void *)expr_functionLessThan,        TE_FUNCTION2);
    EXPR_TE_FUNCTION (7, "le",      (const void *)expr_functionLessEqual,       TE_FUNCTION2);
    EXPR_TE_FUNCTION (8, "gt",      (const void *)expr_functionGreaterThan,     TE_FUNCTION2);
    EXPR_TE_FUNCTION (9, "ge",      (const void *)expr_functionGreaterEqual,    TE_FUNCTION2);
    
    #endif
    
    return x;
}

static void constructor_free (t_constructor *x)
{
    #if PD_WITH_TINYEXPR
    
    if (x->x_expression) { te_free (x->x_expression); }
    
    #endif
    
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
