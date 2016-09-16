
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

static t_class *array_sum_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define t_array_sum t_arrayrange

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void array_sum_bang (t_arrayrange *x)
{
    if (!arrayrange_isValid (x)) { error_invalid (sym_array__space__sum, sym_field); }
    else {
    //
    int i, n;
    t_array *a = arrayrange_getRange (x, &i, &n);
    double sum = 0.0;
    for (i = 0; i < n; i++) { sum += array_getFloatAtIndex (a, i, arrayrange_getFieldName (x)); }
    outlet_float (x->ar_arrayclient.ac_obj.te_outlet, sum);
    //
    }
}

static void array_sum_float(t_arrayrange *x, t_float f)
{
    x->ar_first = f;
    array_sum_bang(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *arraysum_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_sum *x = arrayrange_new(array_sum_class, argc, argv,
        0, 1);
    if (!x) { return NULL; } /* FREE & WARN ! */
    outlet_new(&x->ar_arrayclient.ac_obj, &s_float);
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraysum_setup(void)
{
    array_sum_class = class_new(sym_array__space__sum,
        (t_newmethod)arraysum_new, (t_method)arrayclient_free,
            sizeof(t_array_sum), 0, A_GIMME, 0);
    class_addBang(array_sum_class, array_sum_bang);
    class_addFloat(array_sum_class, array_sum_float);
    class_setHelpName(array_sum_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
