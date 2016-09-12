
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -


/* ----------------  array sum -- add them up ------------------- */
static t_class *array_sum_class;

#define t_array_sum t_array_rangeop

void *array_sum_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_sum *x = array_rangeop_new(array_sum_class, s, &argc, &argv,
        0, 1, 1);
    outlet_new(&x->x_tc.tc_obj, &s_float);
    return (x);
}

static void array_sum_bang(t_array_rangeop *x)
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

static void array_sum_float(t_array_rangeop *x, t_float f)
{
    x->x_onset = f;
    array_sum_bang(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---------------- global setup function -------------------- */

void arraysum_setup(void)
{
    array_sum_class = class_new(sym_array__space__sum,
        (t_newmethod)array_sum_new, (t_method)array_client_free,
            sizeof(t_array_sum), 0, A_GIMME, 0);
    class_addBang(array_sum_class, array_sum_bang);
    class_addFloat(array_sum_class, array_sum_float);
    class_setHelpName(array_sum_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
