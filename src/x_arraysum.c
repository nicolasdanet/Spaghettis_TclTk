
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arraysum_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arraysum {
    t_arrayrange  x_arrayrange;         /* Must be the first. */
    t_outlet      *x_outlet;
    } t_arraysum;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void arraysum_bang (t_arraysum *x)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__sum, sym_field); }
    else {
    //
    int i, start, n;
    t_array *a = arrayrange_getRange (&x->x_arrayrange, &start, &n);
    double sum = 0.0;
    for (i = 0; i < n; i++) {
        sum += array_getFloatAtIndex (a, start + i, arrayrange_getFieldName (&x->x_arrayrange)); 
    }
    outlet_float (x->x_outlet, sum);
    //
    }
}

static void arraysum_float (t_arraysum *x, t_float f)
{
    arrayrange_setFirst (&x->x_arrayrange, f); arraysum_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *arraysum_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arraysum *x = arrayrange_new (arraysum_class, argc, argv, 0, 1);
    
    if (x) { x->x_outlet = outlet_new (cast_object (x), &s_float); }
    else {
        error_invalidArguments (sym_array__space__sum, argc, argv);
        pd_free (x); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraysum_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__sum,
            (t_newmethod)arraysum_new,
            (t_method)arrayclient_free,
            sizeof (t_arraysum),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, arraysum_bang);
    class_addFloat (c, arraysum_float);
    
    class_setHelpName (c, sym_array);
    
    arraysum_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
