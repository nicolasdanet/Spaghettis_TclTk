
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

#define x_sym x_tc.tc_sym
#define x_struct x_tc.tc_struct
#define x_field x_tc.tc_field
#define x_gp x_tc.tc_gp
#define x_outlet x_tc.tc_obj.te_outlet

/* ----------------  array get -- output as list ------------------- */
static t_class *array_get_class;

#define t_array_get t_array_rangeop

void *array_get_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_get *x = array_rangeop_new(array_get_class, s, &argc, &argv,
        0, 1, 1);
    outlet_new(&x->x_tc.tc_obj, &s_float);
    return (x);
}

static void array_get_bang(t_array_rangeop *x)
{
    char *itemp, *firstitem;
    int stride, nitem, arrayonset, i;
    t_atom *outv;
    if (!array_rangeop_getrange(x, &firstitem, &nitem, &stride, &arrayonset))
        return;
    ATOMS_ALLOCA(outv, nitem);
    for (i = 0, itemp = firstitem; i < nitem; i++, itemp += stride)
        SET_FLOAT(&outv[i],  *(t_float *)itemp);
    outlet_list(x->x_outlet, 0, nitem, outv);
    ATOMS_FREEA(outv, nitem);
}

static void array_get_float(t_array_rangeop *x, t_float f)
{
    x->x_onset = f;
    array_get_bang(x);
}

/* ---------------- global setup function -------------------- */

void arrayget_setup(void)
{
    array_get_class = class_new(sym_array__space__get,
        (t_newmethod)array_get_new, (t_method)array_client_free,
            sizeof(t_array_get), 0, A_GIMME, 0);
    class_addBang(array_get_class, array_get_bang);
    class_addFloat(array_get_class, array_get_float);
    class_setHelpName(array_get_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
