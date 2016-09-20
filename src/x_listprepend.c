
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

static t_class  *listprepend_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listprepend {
    t_object        x_obj;
    t_listinlet     x_listinlet;
    t_outlet        *x_outlet;
    } t_listprepend;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void listprepend_list(t_listprepend *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_listinlet.li_size + argc;
    ATOMS_ALLOCA(outv, outc);
    atom_copyAtomsUnchecked(argc, argv, outv + x->x_listinlet.li_size);
    if (x->x_listinlet.li_hasPointer)
    {
        t_listinlet y;
        listinlet_clone(&x->x_listinlet, &y);
        listinlet_copyListUnchecked(&y, outv);
        outlet_list(x->x_outlet, &s_list, outc, outv);
        listinlet_clear(&y);
    }
    else
    {
        listinlet_copyListUnchecked(&x->x_listinlet, outv);
        outlet_list(x->x_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}



static void listprepend_anything(t_listprepend *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_listinlet.li_size + argc + 1;
    ATOMS_ALLOCA(outv, outc);
    SET_SYMBOL(outv + x->x_listinlet.li_size, s);
    atom_copyAtomsUnchecked(argc, argv, outv + x->x_listinlet.li_size + 1);
    if (x->x_listinlet.li_hasPointer)
    {
        t_listinlet y;
        listinlet_clone(&x->x_listinlet, &y);
        listinlet_copyListUnchecked(&y, outv);
        outlet_list(x->x_outlet, &s_list, outc, outv);
        listinlet_clear(&y);
    }
    else
    {
        listinlet_copyListUnchecked(&x->x_listinlet, outv);
        outlet_list(x->x_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *listprepend_new(t_symbol *s, int argc, t_atom *argv)
{
    t_listprepend *x = (t_listprepend *)pd_new(listprepend_class);
    listinlet_init(&x->x_listinlet);
    listinlet_setList(&x->x_listinlet, argc, argv);
    x->x_outlet = outlet_new(&x->x_obj, &s_list);
    inlet_new(&x->x_obj, &x->x_listinlet.li_pd, 0, 0);
    return (x);
}

static void listprepend_free(t_listprepend *x)
{
    listinlet_clear(&x->x_listinlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listprepend_setup (void)
{
    listprepend_class = class_new(sym_list__space__prepend,
        (t_newmethod)listprepend_new, (t_method)listprepend_free,
        sizeof(t_listprepend), CLASS_DEFAULT, A_GIMME, 0);
    class_addList(listprepend_class, listprepend_list);
    class_addAnything(listprepend_class, listprepend_anything);
    class_setHelpName(listprepend_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

