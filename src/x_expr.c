
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
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://github.com/codeplea/tinyexpr > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "tinyexpr.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *expr_class;                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EXPR_VARIABLES      9

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _expr {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f[EXPR_VARIABLES];
    double      x_v[EXPR_VARIABLES];
    te_variable x_variables[EXPR_VARIABLES];
    te_expr     *x_expression;
    t_outlet    *x_outlet;
    } t_expr;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define EXPR_TE_VARIABLE(i, s)  { \
                                        x->x_variables[i].name      = (s)->s_name;  \
                                        x->x_variables[i].address   = x->x_v + (i); \
                                        x->x_variables[i].type      = TE_VARIABLE;  \
                                        x->x_variables[i].context   = NULL; \
                                }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void expr_initializeVariables (t_expr *x)
{
    int i;
    
    for (i = 0; i < EXPR_VARIABLES; i++) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, PD_STRING, "vf%d", i + 1);
        EXPR_TE_VARIABLE (i, gensym (t));
    }
}

static int expr_getNumberOfVariables (char *expression)
{
    int i, n = 0;
    
    for (i = 0; i < EXPR_VARIABLES; i++) {
        char t[PD_STRING] = { 0 }; 
        string_sprintf (t, PD_STRING, "$f%d", i + 1);
        if (string_contains (expression, t)) { n = i + 1; }
    }
    
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void expr_bang (t_expr *x)
{
    int i; 
    
    for (i = 0; i < EXPR_VARIABLES; i++) { x->x_v[i] = (double)x->x_f[i]; }
    
    outlet_float (x->x_outlet, (t_float)(te_eval (x->x_expression)));
}

static void expr_float (t_expr *x, t_float f)
{
    x->x_f[0] = f; expr_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *expr_new (t_symbol *s, int argc, t_atom *argv)
{
    t_expr *x = (t_expr *)pd_new (expr_class);
    
    int err, size = 0;
    
    {
        char *t = argc ? atom_atomsToString (argc, argv) : "0";
        
        size = expr_getNumberOfVariables (t);
        expr_initializeVariables (x);
        string_replaceCharacter (t, '$', 'v');
        x->x_expression = te_compile (t, x->x_variables, size, &err);
        
        if (argc) { PD_MEMORY_FREE (t); }
    }
    
    if (x->x_expression) {
        
        int i;
        x->x_outlet = outlet_new (cast_object (x), &s_float);
        for (i = 1; i < size; i++) { inlet_newFloat (cast_object (x), &x->x_f[i]); }
    
    } else {
    
        error_invalid (sym_expr, sym_expression);
        pd_free (cast_pd (x));
        x = NULL;
    }
    
    return x;
}

static void expr_free (t_expr *x)
{
    if (x->x_expression) { te_free (x->x_expression); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void expr_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_expr,
            (t_newmethod)expr_new,
            (t_method)expr_free,
            sizeof (t_expr),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
        
    class_addBang (c, (t_method)expr_bang);
    class_addFloat (c, (t_method)expr_float);
    
    expr_class = c;
}

void expr_destroy (void)
{
    CLASS_FREE (expr_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
