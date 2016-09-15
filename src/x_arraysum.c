
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
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_control.h"


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---  array_client - common code for objects that refer to arrays -- */

#define x_sym ar_arrayclient.ac_name
#define x_struct ar_arrayclient.ac_templateIdentifier
#define x_field ar_arrayclient.ac_fieldName
#define x_gp ar_arrayclient.ac_gpointer
#define x_outlet ar_arrayclient.ac_obj.te_outlet

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -


/* ----------------  array sum -- add them up ------------------- */
static t_class *array_sum_class;

#define t_array_sum t_arrayrange

void *arraysum_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_sum *x = arrayrange_new(array_sum_class, argc, argv,
        0, 1);
    if (!x) { return NULL; } /* FREE & WARN ! */
    outlet_new(&x->ar_arrayclient.ac_obj, &s_float);
    return (x);
}

/*
print: 0.653327
print: 21
print: -83

static void array_sum_bang(t_arrayrange *x)
{
    char *itemp, *firstitem;
    int stride, nitem, arrayonset, i;
    double sum;
    if (!array_rangeop_getrange(x, &firstitem, &nitem, &stride, &arrayonset))
        return;
    for (i = 0, sum = 0, itemp = firstitem; i < nitem; i++, itemp += stride)
        sum += *(t_float *)itemp;
    outlet_float(x->x_outlet, sum);
}
*/

static void array_sum_bang (t_arrayrange *x)
{
    if (!arrayrange_isValid (x)) { error_invalid (sym_array__space__sum, sym_field); }
    else {
    //
    int i, n;
    t_array *a = arrayrange_getRange (x, &i, &n);
    double sum = 0.0;
    for (i = 0; i < n; i++) { sum += array_getFloatAtIndex (a, i, arrayrange_getFieldName (x)); }
    outlet_float (x->x_outlet, sum);
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

/* ---------------- global setup function -------------------- */

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
