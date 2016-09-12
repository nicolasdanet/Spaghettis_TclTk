
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

/* ---  array_client - common code for objects that refer to arrays -- */

#define x_sym ar_arrayclient.ac_name
#define x_struct ar_arrayclient.ac_templateIdentifier
#define x_field ar_arrayclient.ac_fieldName
#define x_gp ar_arrayclient.ac_gpointer
#define x_outlet ar_arrayclient.ac_obj.te_outlet

/* --------------  array set -- copy list to array -------------- */
static t_class *array_set_class;

#define t_array_set t_arrayrange

void *array_set_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_set *x = array_rangeop_new(array_set_class, s, &argc, &argv,
        1, 0, 1);
    return (x);
}

static void array_set_list(t_arrayrange *x, t_symbol *s,
    int argc, t_atom *argv)
{
    char *itemp, *firstitem;
    int stride, nitem, arrayonset, i;
    if (!array_rangeop_getrange(x, &firstitem, &nitem, &stride, &arrayonset))
        return;
    if (nitem > argc)
        nitem = argc;
    for (i = 0, itemp = firstitem; i < nitem; i++, itemp += stride)
        *(t_float *)itemp = atom_getFloatAtIndex(i, argc, argv);
    array_client_senditup(&x->ar_arrayclient);
}

/* ---------------- global setup function -------------------- */

void arrayset_setup(void)
{
    array_set_class = class_new(sym_array__space__set,
        (t_newmethod)array_set_new, (t_method)array_client_free,
            sizeof(t_array_set), 0, A_GIMME, 0);
    class_addList(array_set_class, array_set_list);
    class_setHelpName(array_set_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
