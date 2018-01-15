
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

static t_class *arraysum_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arraysum {
    t_arrayrange    x_arrayrange;       /* Must be the first. */
    t_outlet        *x_outlet;
    } t_arraysum;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    outlet_float (x->x_outlet, (t_float)sum);
    //
    }
}

static void arraysum_float (t_arraysum *x, t_float f)
{
    arrayrange_setFirst (&x->x_arrayrange, f); arraysum_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *arraysum_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arraysum *x = (t_arraysum *)arrayrange_new (arraysum_class, argc, argv, 0, 1);
    
    if (ARRAYRANGE_GOOD (x)) { x->x_outlet = outlet_new (cast_object (x), &s_float); }
    else {
        error_invalidArguments (sym_array__space__sum, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
            
    class_addBang (c, (t_method)arraysum_bang);
    class_addFloat (c, (t_method)arraysum_float);
    
    class_setHelpName (c, sym_array);
    
    arraysum_class = c;
}

void arraysum_destroy (void)
{
    class_free (arraysum_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
