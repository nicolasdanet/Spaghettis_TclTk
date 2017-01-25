
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arraymax_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arraymax {
    t_arrayrange    x_arrayrange;           /* Must be the first. */
    t_outlet        *x_outletLeft;
    t_outlet        *x_outletRight;
    } t_arraymax;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void arraymax_bang (t_arraymax *x)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__max, sym_field); }
    else {
    //
    int i, start, n;
    t_array *a = arrayrange_getRange (&x->x_arrayrange, &start, &n);
    
    t_float maxIndex = (t_float)-1.0;
    t_float maxValue = -PD_FLT_MAX;
    
    for (i = 0; i < n; i++) {
        t_float t = array_getFloatAtIndex (a, start + i, arrayrange_getFieldName (&x->x_arrayrange));
        if (t > maxValue) { maxValue = t; maxIndex = start + i; }
    }
    
    outlet_float (x->x_outletRight, maxIndex);
    outlet_float (x->x_outletLeft, maxValue);
    //
    }
}

static void arraymax_float (t_arraymax *x, t_float f)
{
    arrayrange_setFirst (&x->x_arrayrange, f); arraymax_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *arraymax_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arraymax *x = (t_arraymax *)arrayrange_new (arraymax_class, argc, argv, 0, 1);
    
    if (x) {
        x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
        x->x_outletRight = outlet_new (cast_object (x), &s_float);

    } else {
        error_invalidArguments (sym_array__space__max, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraymax_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__max,
            (t_newmethod)arraymax_new,
            (t_method)arrayclient_free,
            sizeof (t_arraymax),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)arraymax_bang);
    class_addFloat (c, (t_method)arraymax_float);

    class_setHelpName (c, sym_array);
    
    arraymax_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
