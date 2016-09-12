
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

/* -----  array quantile -- output quantile for input from 0 to 1 ------- */
static t_class *array_quantile_class;

#define t_array_quantile t_arrayrange

void *array_quantile_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_quantile *x = array_rangeop_new(array_quantile_class, s,
        &argc, &argv, 1, 1, 1);
    outlet_new(&x->ar_arrayclient.ac_obj, &s_float);
    return (x);
}

void array_quantile_float(t_arrayrange *x, t_float f)
{
    char *itemp, *firstitem;
    int stride, nitem, arrayonset, i;
    double sum;
    if (!array_rangeop_getrange(x, &firstitem, &nitem, &stride, &arrayonset))
        return;
    for (i = 0, sum = 0, itemp = firstitem; i < nitem; i++, itemp += stride)
        sum += (*(t_float *)itemp > 0? *(t_float *)itemp : 0);
    sum *= f;
    for (i = 0, itemp = firstitem; i < (nitem-1); i++, itemp += stride)
    {
        sum -= (*(t_float *)itemp > 0? *(t_float *)itemp : 0);
        if (sum < 0)
            break;
    }
    outlet_float(x->x_outlet, i);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arrayquantile_setup(void)
{
    array_quantile_class = class_new(sym_array__space__quantile,
        (t_newmethod)array_quantile_new, (t_method)arrayclient_free,
            sizeof(t_array_quantile), 0, A_GIMME, 0);
    class_addFloat(array_quantile_class, array_quantile_float);
    class_setHelpName(array_quantile_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
