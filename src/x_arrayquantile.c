
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arrayquantile_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arrayquantile {
    t_arrayrange    x_arrayrange;           /* Must be the first. */
    t_outlet        *x_outlet;
    } t_arrayquantile;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arrayquantile_float (t_arrayquantile *x, t_float f)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__quantile, sym_field); }
    else {
        outlet_float (x->x_outlet, arrayrange_quantile (&x->x_arrayrange, f));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *arrayquantile_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arrayquantile *x = (t_arrayquantile *)arrayrange_new (arrayquantile_class, argc, argv, 1, 1);
    
    if (x) { x->x_outlet = outlet_new (cast_object (x), &s_float); }
    else {
        error_invalidArguments (sym_array__space__quantile, argc, argv);
        pd_free (x); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arrayquantile_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__quantile,
            (t_newmethod)arrayquantile_new,
            (t_method)arrayclient_free,
            sizeof (t_arrayquantile),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, (t_method)arrayquantile_float);
    
    class_setHelpName (c, sym_array);
    
    arrayquantile_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
