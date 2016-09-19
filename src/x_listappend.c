
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

t_class *list_append_class;

typedef struct _list_append
{
    t_object x_obj;
    t_list x_alist;
} t_list_append;

void *listappend_new(t_symbol *s, int argc, t_atom *argv)
{
    t_list_append *x = (t_list_append *)pd_new(list_append_class);
    alist_init(&x->x_alist);
    alist_list(&x->x_alist, 0, argc, argv);
    outlet_new(&x->x_obj, &s_list);
    inlet_new(&x->x_obj, &x->x_alist.l_pd, 0, 0);
    return (x);
}

static void list_append_list(t_list_append *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_alist.l_size + argc;
    ATOMS_ALLOCA(outv, outc);
    atom_copyAtomsUnchecked(argc, argv, outv);
    if (x->x_alist.l_numberOfPointers)
    {
        t_list y;
        alist_clone(&x->x_alist, &y);
        alist_toatoms(&y, outv+argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
        alist_clear(&y);
    }
    else
    {
        alist_toatoms(&x->x_alist, outv+argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}

static void list_append_anything(t_list_append *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_alist.l_size + argc + 1;
    ATOMS_ALLOCA(outv, outc);
    SET_SYMBOL(outv, s);
    atom_copyAtomsUnchecked(argc, argv, outv + 1);
    if (x->x_alist.l_numberOfPointers)
    {
        t_list y;
        alist_clone(&x->x_alist, &y);
        alist_toatoms(&y, outv + 1 + argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
        alist_clear(&y);
    }
    else
    {
        alist_toatoms(&x->x_alist, outv + 1 + argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}

static void list_append_free(t_list_append *x)
{
    alist_clear(&x->x_alist);
}

void list_append_setup(void)
{
    list_append_class = class_new(sym_list__space__append,
        (t_newmethod)listappend_new, (t_method)list_append_free,
        sizeof(t_list_append), CLASS_DEFAULT, A_GIMME, 0);
    class_addList(list_append_class, list_append_list);
    class_addAnything(list_append_class, list_append_anything);
    class_setHelpName(list_append_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

