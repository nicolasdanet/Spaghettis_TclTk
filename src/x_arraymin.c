
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

/* ----  array min -- output largest value and its index ------------ */
static t_class *array_min_class;

typedef struct _array_min
{
    t_arrayrange x_rangeop;
    t_outlet *x_out1;       /* value */
    t_outlet *x_out2;       /* index */
} t_array_min;

void *array_min_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_min *x = array_rangeop_new(array_min_class, s, &argc, &argv,
        0, 1, 1);
    x->x_out1 = outlet_new(&x->x_rangeop.ar_arrayclient.ac_obj, &s_float);
    x->x_out2 = outlet_new(&x->x_rangeop.ar_arrayclient.ac_obj, &s_float);
    return (x);
}

static void array_min_bang(t_array_min *x)
{
    char *itemp, *firstitem;
    int stride, nitem, i, arrayonset, besti;
    t_float bestf;
    if (!array_rangeop_getrange(&x->x_rangeop, &firstitem, &nitem, &stride,
        &arrayonset))
            return;
    for (i = 0, besti = -1, bestf= 1e30, itemp = firstitem;
        i < nitem; i++, itemp += stride)
            if (*(t_float *)itemp < bestf)
                bestf = *(t_float *)itemp, besti = i+arrayonset;
    outlet_float(x->x_out2, besti);
    outlet_float(x->x_out1, bestf);
}

static void array_min_float(t_array_min *x, t_float f)
{
    x->x_rangeop.ar_onset = f;
    array_min_bang(x);
}

/* ---------------- global setup function -------------------- */

void arraymin_setup(void )
{
    array_min_class = class_new(sym_array__space__min,
        (t_newmethod)array_min_new, (t_method)array_client_free,
            sizeof(t_array_min), 0, A_GIMME, 0);
    class_addFloat(array_min_class, array_min_float);
    class_addBang(array_min_class, array_min_bang);
    class_setHelpName(array_min_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
