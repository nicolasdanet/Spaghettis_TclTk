
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arraymin_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arraymin {
    t_arrayrange    x_arrayrange;           /* Must be the first. */
    t_outlet        *x_outletLeft;
    t_outlet        *x_outletRight;
    } t_arraymin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void arraymin_bang (t_arraymin *x)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__min, sym_field); }
    else {
    //
    int i, start, n;
    t_array *a = arrayrange_getRange (&x->x_arrayrange, &start, &n);
    
    t_float minIndex = (t_float)-1.0;
    t_float minValue = PD_FLT_MAX;
    
    for (i = 0; i < n; i++) {
        t_float t = array_getFloatAtIndex (a, start + i, arrayrange_getFieldName (&x->x_arrayrange));
        if (t < minValue) { minValue = t; minIndex = start + i; }
    }
    
    outlet_float (x->x_outletRight, minIndex);
    outlet_float (x->x_outletLeft, minValue);
    //
    }
}

static void arraymin_float (t_arraymin *x, t_float f)
{
    arrayrange_setFirst (&x->x_arrayrange, f); arraymin_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *arraymin_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arraymin *x = (t_arraymin *)arrayrange_new (arraymin_class, argc, argv, 0, 1);

    if (x) {
        x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
        x->x_outletRight = outlet_new (cast_object (x), &s_float);

    } else {
        error_invalidArguments (sym_array__space__min, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arraymin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__min,
            (t_newmethod)arraymin_new,
            (t_method)arrayclient_free,
            sizeof (t_arraymin),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)arraymin_bang);
    class_addFloat (c, (t_method)arraymin_float);

    class_setHelpName (c, sym_array);
    
    arraymin_class = c;
}

void arraymin_destroy (void)
{
    CLASS_FREE (arraymin_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
