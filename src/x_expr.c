
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_TINYEXPR

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://github.com/codeplea/tinyexpr > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "tinyexpr.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *expr_class;                     /* Shared. */
static t_class *vexpr_class;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EXPR_VARIABLES  9
#define EXPR_FUNCTIONS  10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_randMT *expr_randMT;                   /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _expr {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f[EXPR_VARIABLES];
    double      x_v[EXPR_VARIABLES];
    te_variable x_variables[EXPR_VARIABLES + EXPR_FUNCTIONS];
    te_expr     *x_expression;
    t_buffer    *x_vector;
    t_outlet    *x_outlet;
    } t_expr;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define EXPR_TE_VARIABLE(i, s)          { \
                                            x->x_variables[i].name      = (s)->s_name;  \
                                            x->x_variables[i].address   = x->x_v + (i); \
                                            x->x_variables[i].type      = TE_VARIABLE;  \
                                            x->x_variables[i].context   = NULL; \
                                        }

#define EXPR_TE_FUNCTION(i, s, f, t) { \
                                            x->x_variables[i].name      = s;    \
                                            x->x_variables[i].address   = f;    \
                                            x->x_variables[i].type      = t;    \
                                            x->x_variables[i].context   = NULL; \
                                        }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

double expr_functionRandom (void)
{
    static int once = 0;            /* Static. */
    static t_rand48 seed = 0;       /* Static. */
    
    if (!once) { seed = time_makeRandomSeed(); once = 1; }
    
    return PD_RAND48_DOUBLE (seed);
}

double expr_functionRandomMT (void)
{
    return randMT_getDouble (expr_randMT);
}

double expr_functionMinimum (double a, double b)
{
    return PD_MIN (a, b);
}

double expr_functionMaximum (double a, double b)
{
    return PD_MAX (a, b);
}

double expr_functionEqual (double a, double b)
{
    return (a == b);
}

double expr_functionUnequal (double a, double b)
{
    return (a != b);
}

double expr_functionLessThan (double a, double b)
{
    return (a < b);
}

double expr_functionLessEqual (double a, double b)
{
    return (a <= b);
}

double expr_functionGreaterThan (double a, double b)
{
    return (a > b);
}

double expr_functionGreaterEqual (double a, double b)
{
    return (a >= b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void expr_initializeVariables (t_expr *x)
{
    int i;
    
    for (i = 0; i < EXPR_VARIABLES; i++) {
        char t[PD_STRING] = { 0 };
        string_sprintf (t, PD_STRING, "vf%d", i + 1);
        EXPR_TE_VARIABLE (i, gensym (t));
    }
}

static void expr_initializeFunctions (t_expr *x, int i)
{
    PD_ASSERT (i < EXPR_VARIABLES);
    
    /* Add extended functions. */
    
    EXPR_TE_FUNCTION (i,        "rand",     (const void *)expr_functionRandom,        TE_FUNCTION0);
    EXPR_TE_FUNCTION (i + 1,    "randmt",   (const void *)expr_functionRandomMT,      TE_FUNCTION0);
    EXPR_TE_FUNCTION (i + 2,    "min",      (const void *)expr_functionMinimum,       TE_FUNCTION2);
    EXPR_TE_FUNCTION (i + 3,    "max",      (const void *)expr_functionMaximum,       TE_FUNCTION2);
    EXPR_TE_FUNCTION (i + 4,    "eq",       (const void *)expr_functionEqual,         TE_FUNCTION2);
    EXPR_TE_FUNCTION (i + 5,    "ne",       (const void *)expr_functionUnequal,       TE_FUNCTION2);
    EXPR_TE_FUNCTION (i + 6,    "lt",       (const void *)expr_functionLessThan,      TE_FUNCTION2);
    EXPR_TE_FUNCTION (i + 7,    "le",       (const void *)expr_functionLessEqual,     TE_FUNCTION2);
    EXPR_TE_FUNCTION (i + 8,    "gt",       (const void *)expr_functionGreaterThan,   TE_FUNCTION2);
    EXPR_TE_FUNCTION (i + 9,    "ge",       (const void *)expr_functionGreaterEqual,  TE_FUNCTION2);
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
// MARK: -

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
// MARK: -

static void vexpr_bang (t_expr *x)
{
    t_atom *t = NULL;
    int i, size = buffer_getSize (x->x_vector);
    
    PD_ATOMS_ALLOCA (t, size);
    
    x->x_f[0] = (t_float)0.0;
    
    for (i = 0; i < EXPR_VARIABLES; i++) { x->x_v[i] = (double)x->x_f[i]; }
    
    for (i = 0; i < size; i++) {
    //
    x->x_f[0] = atom_getFloat (buffer_getAtomAtIndex (x->x_vector, i));
    x->x_v[0] = (double)x->x_f[0];
    
    {
        t_float f = (t_float)te_eval (x->x_expression); SET_FLOAT (t + i, f);
    }
    //
    }
    
    outlet_list (x->x_outlet, size, t);
    
    PD_ATOMS_FREEA (t, size);
}

static void vexpr_float (t_expr *x, t_float f)
{
    t_atom a; SET_FLOAT (&a, f);
    
    buffer_clear (x->x_vector); buffer_appendAtom (x->x_vector, &a);
    
    vexpr_bang (x);
}

static void vexpr_list (t_expr *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_clear (x->x_vector); buffer_append (x->x_vector, argc, argv);
    
    vexpr_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *expr_new (t_symbol *s, int argc, t_atom *argv)
{
    t_expr *x = (t_expr *)pd_new ((s == sym_expr) ? expr_class : vexpr_class);
    
    int err, size = 0;
    
    {
        char *z = (char *)"0";
        char *t = argc ? atom_atomsToString (argc, argv) : z;
        
        if (argc) { string_removeCharacter (t, '\\'); }
        
        size = expr_getNumberOfVariables (t);
        expr_initializeVariables (x);
        string_replaceCharacter (t, '$', 'v');
        expr_initializeFunctions  (x, size);
        x->x_expression = te_compile (t, x->x_variables, size + EXPR_FUNCTIONS, &err);
        
        if (argc) { PD_MEMORY_FREE (t); }
    }
    
    if (x->x_expression) {
        
        int i;
        x->x_outlet = outlet_new (cast_object (x), (s == sym_expr) ? &s_float : &s_list);
        x->x_vector = buffer_new();
        for (i = 1; i < size; i++) { inlet_newFloat (cast_object (x), &x->x_f[i]); }
    
    } else {
    
        error_invalid ((s == sym_expr) ? sym_expr : sym_vexpr, sym_expression);
        pd_free (cast_pd (x));
        x = NULL;
    }
    
    return x;
}

static void expr_free (t_expr *x)
{
    if (x->x_vector)     { buffer_free (x->x_vector); }
    if (x->x_expression) { te_free (x->x_expression); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void expr_setup (void)
{
    expr_class = class_new (sym_expr,
            (t_newmethod)expr_new,
            (t_method)expr_free,
            sizeof (t_expr),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    vexpr_class = class_new (sym_vexpr,
            (t_newmethod)expr_new,
            (t_method)expr_free,
            sizeof (t_expr),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (expr_class, (t_method)expr_bang);
    class_addFloat (expr_class, (t_method)expr_float);
    
    class_addBang (vexpr_class, (t_method)vexpr_bang);
    class_addFloat (vexpr_class, (t_method)vexpr_float);
    class_addList (vexpr_class, (t_method)vexpr_list);
    
    class_setHelpName (vexpr_class, sym_expr);
}

void expr_destroy (void)
{
    class_free (expr_class);
    class_free (vexpr_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void expr_initialize (void)
{
    expr_randMT = randMT_new();
}

void expr_release (void)
{
    randMT_free (expr_randMT);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

void expr_setup (void)
{
}

void expr_destroy (void)
{
}

void expr_initialize (void)
{
}

void expr_release (void)
{
}

#endif // PD_WITH_TINYEXPR

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
