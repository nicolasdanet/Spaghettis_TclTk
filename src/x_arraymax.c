
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


/* ----  array max -- output largest value and its index ------------ */
static t_class *array_max_class;

typedef struct _array_max
{
    t_arrayrange x_rangeop;
    t_outlet *x_out1;       /* value */
    t_outlet *x_out2;       /* index */
} t_array_max;

void *array_max_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_max *x = arrayrange_new(array_max_class, argc, argv,
        0, 1);
    if (!x) { return NULL; } /* FREE & WARN ! */
    x->x_out1 = outlet_new(&x->x_rangeop.ar_arrayclient.ac_obj, &s_float);
    x->x_out2 = outlet_new(&x->x_rangeop.ar_arrayclient.ac_obj, &s_float);
    return (x);
}

static void array_max_bang(t_array_max *x)
{
    char *itemp, *firstitem;
    int stride, nitem, arrayonset, i, besti;
    t_float bestf;
    if (!array_rangeop_getrange(&x->x_rangeop, &firstitem, &nitem, &stride,
        &arrayonset))
            return;
    for (i = 0, besti = -1, bestf= -1e30, itemp = firstitem;
        i < nitem; i++, itemp += stride)
            if (*(t_float *)itemp > bestf)
                bestf = *(t_float *)itemp, besti = i+arrayonset;
    outlet_float(x->x_out2, besti);
    outlet_float(x->x_out1, bestf);
}

static void array_max_float(t_array_max *x, t_float f)
{
    x->x_rangeop.ar_first = f;
    array_max_bang(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraymax_setup(void)
{
    array_max_class = class_new(sym_array__space__max,
        (t_newmethod)array_max_new, (t_method)arrayclient_free,
            sizeof(t_array_max), 0, A_GIMME, 0);
    class_addFloat(array_max_class, array_max_float);
    class_addBang(array_max_class, array_max_bang);
    class_setHelpName(array_max_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
