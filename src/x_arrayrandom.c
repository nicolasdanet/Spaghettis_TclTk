
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_array.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arrayrandom_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arrayrandom {
    t_arrayrange    x_arrayrange;           /* Must be the first. */
    t_rand48        x_state;
    t_outlet        *x_outlet;
    } t_arrayrandom;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void arrayrandom_bang (t_arrayrandom *x)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__random, sym_field); }
    else {
        double k = PD_RAND48_DOUBLE (x->x_state);
        outlet_float (x->x_outlet, arrayrange_quantile (&x->x_arrayrange, (t_float)k));
    }
}

static void arrayrandom_float (t_arrayrandom *x, t_float f)
{
    arrayrange_setFirst (&x->x_arrayrange, f); arrayrandom_bang (x);
}

static void arrayrandom_seed (t_arrayrandom *x, t_float f)
{
    x->x_state = (t_rand48)f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *arrayrandom_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arrayrandom *x = (t_arrayrandom *)arrayrange_new (arrayrandom_class, argc, argv, 0, 1);
    
    if (ARRAYRANGE_GOOD (x)) {
        PD_RAND48_INIT (x->x_state);
        x->x_outlet = outlet_new (cast_object (x), &s_float);
        
    } else {
        error_invalidArguments (sym_array__space__random, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayrandom_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__random,
            (t_newmethod)arrayrandom_new,
            (t_method)arrayclient_free,
            sizeof (t_arrayrandom),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)arrayrandom_bang);
    class_addFloat (c, (t_method)arrayrandom_float);
    
    class_addMethod (c, (t_method)arrayrandom_seed, sym_seed, A_FLOAT, A_NULL);

    class_setHelpName (c, sym_array);
    
    arrayrandom_class = c;
}

void arrayrandom_destroy (void)
{
    class_free (arrayrandom_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
