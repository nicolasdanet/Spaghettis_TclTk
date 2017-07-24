
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_array.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arrayget_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arrayget {
    t_arrayrange    x_arrayrange;           /* Must be the first. */
    t_outlet        *x_outlet;
    } t_arrayget;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void arrayget_bang (t_arrayget *x)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__get, sym_field); }
    else {
    //
    int i, start, n;
    t_array *a = arrayrange_getRange (&x->x_arrayrange, &start, &n);
    t_atom *t  = NULL;
    
    PD_ATOMS_ALLOCA (t, n);
    
    for (i = 0; i < n; i++) {
        SET_FLOAT (t + i, array_getFloatAtIndex (a, start + i, arrayrange_getFieldName (&x->x_arrayrange))); 
    }
    outlet_list (x->x_outlet, n, t);
    
    PD_ATOMS_FREEA (t, n);
    //
    }
}

static void arrayget_float (t_arrayget *x, t_float f)
{
    arrayrange_setFirst (&x->x_arrayrange, f); arrayget_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *arrayget_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arrayget *x = (t_arrayget *)arrayrange_new (arrayget_class, argc, argv, 0, 1);
    
    if (ARRAYRANGE_GOOD (x)) { x->x_outlet = outlet_new (cast_object (x), &s_list); }
    else {
        error_invalidArguments (sym_array__space__get, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayget_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__get,
            (t_newmethod)arrayget_new,
            (t_method)arrayclient_free,
            sizeof (t_arrayget),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)arrayget_bang);
    class_addFloat (c, (t_method)arrayget_float);
    
    class_setHelpName (c, sym_array);
    
    arrayget_class = c;
}

void arrayget_destroy (void)
{
    class_free (arrayget_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
