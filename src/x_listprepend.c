
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

t_class *list_prepend_class;

typedef struct _list_prepend
{
    t_object x_obj;
    t_listinlet x_alist;
} t_list_prepend;

void *listprepend_new(t_symbol *s, int argc, t_atom *argv)
{
    t_list_prepend *x = (t_list_prepend *)pd_new(list_prepend_class);
    listinlet_init(&x->x_alist);
    listinlet_setList(&x->x_alist, argc, argv);
    outlet_new(&x->x_obj, &s_list);
    inlet_new(&x->x_obj, &x->x_alist.li_pd, 0, 0);
    return (x);
}

static void list_prepend_list(t_list_prepend *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_alist.li_size + argc;
    ATOMS_ALLOCA(outv, outc);
    atom_copyAtomsUnchecked(argc, argv, outv + x->x_alist.li_size);
    if (x->x_alist.li_hasPointer)
    {
        t_listinlet y;
        listinlet_clone(&x->x_alist, &y);
        listinlet_copyListUnchecked(&y, outv);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
        listinlet_clear(&y);
    }
    else
    {
        listinlet_copyListUnchecked(&x->x_alist, outv);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}



static void list_prepend_anything(t_list_prepend *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_alist.li_size + argc + 1;
    ATOMS_ALLOCA(outv, outc);
    SET_SYMBOL(outv + x->x_alist.li_size, s);
    atom_copyAtomsUnchecked(argc, argv, outv + x->x_alist.li_size + 1);
    if (x->x_alist.li_hasPointer)
    {
        t_listinlet y;
        listinlet_clone(&x->x_alist, &y);
        listinlet_copyListUnchecked(&y, outv);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
        listinlet_clear(&y);
    }
    else
    {
        listinlet_copyListUnchecked(&x->x_alist, outv);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}

static void list_prepend_free(t_list_prepend *x)
{
    listinlet_clear(&x->x_alist);
}

void list_prepend_setup(void)
{
    list_prepend_class = class_new(sym_list__space__prepend,
        (t_newmethod)listprepend_new, (t_method)list_prepend_free,
        sizeof(t_list_prepend), CLASS_DEFAULT, A_GIMME, 0);
    class_addList(list_prepend_class, list_prepend_list);
    class_addAnything(list_prepend_class, list_prepend_anything);
    class_setHelpName(list_prepend_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

