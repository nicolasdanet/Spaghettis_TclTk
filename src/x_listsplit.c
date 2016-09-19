
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
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;

t_class *list_split_class;

typedef struct _list_split
{
    t_object x_obj;
    t_float x_f;
    t_outlet *x_out1;
    t_outlet *x_out2;
    t_outlet *x_out3;
} t_list_split;

void *list_split_new(t_float f)
{
    t_list_split *x = (t_list_split *)pd_new(list_split_class);
    x->x_out1 = outlet_new(&x->x_obj, &s_list);
    x->x_out2 = outlet_new(&x->x_obj, &s_list);
    x->x_out3 = outlet_new(&x->x_obj, &s_list);
    inlet_newFloat(&x->x_obj, &x->x_f);
    x->x_f = f;
    return (x);
}

static void list_split_list(t_list_split *x, t_symbol *s,
    int argc, t_atom *argv)
{
    int n = x->x_f;
    if (n < 0)
        n = 0;
    if (argc >= n)
    {
        outlet_list(x->x_out2, &s_list, argc-n, argv+n);
        outlet_list(x->x_out1, &s_list, n, argv);
    }
    else outlet_list(x->x_out3, &s_list, argc, argv);
}

static void list_split_anything(t_list_split *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    ATOMS_ALLOCA(outv, argc+1);
    SET_SYMBOL(outv, s);
    atoms_copy(argc, argv, outv + 1);
    list_split_list(x, &s_list, argc+1, outv);
    ATOMS_FREEA(outv, argc+1);
}

void list_split_setup(void)
{
    list_split_class = class_new(sym_list__space__split,
        (t_newmethod)list_split_new, 0,
        sizeof(t_list_split), 0, A_DEFFLOAT, 0);
    class_addList(list_split_class, list_split_list);
    class_addAnything(list_split_class, list_split_anything);
    class_setHelpName(list_split_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

