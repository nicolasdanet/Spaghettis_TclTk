
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_list.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listprepend_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listprepend {
    t_object        x_obj;              /* Must be the first. */
    t_listinlet     x_listinlet;
    t_outlet        *x_outlet;
    } t_listprepend;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void listprepend_list (t_listprepend *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    int count = listinlet_getSize (&x->x_listinlet) + argc;
    
    PD_ATOMS_ALLOCA (t, count);
    
    atom_copyAtoms (argv, argc, t + listinlet_getSize (&x->x_listinlet), argc);
    
    if (listinlet_hasPointer (&x->x_listinlet)) {
    
        t_listinlet cache;
        listinlet_init (&cache);
        listinlet_clone (&x->x_listinlet, &cache);
        listinlet_copyListUnchecked (&cache, t);
        outlet_list (x->x_outlet, count, t);
        listinlet_clear (&cache);
        
    } else {
    
        listinlet_copyListUnchecked (&x->x_listinlet, t);
        outlet_list (x->x_outlet, count, t);
    }
    
    PD_ATOMS_FREEA (t, count);
}

static void listprepend_anything (t_listprepend *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)listprepend_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *listprepend_new (t_symbol *s, int argc, t_atom *argv)
{
    t_listprepend *x = (t_listprepend *)pd_new (listprepend_class);
    
    listinlet_init (&x->x_listinlet);
    listinlet_setList (&x->x_listinlet, argc, argv);
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    inlet_new (cast_object (x), cast_pd (&x->x_listinlet), NULL, NULL);
    
    return x;
}

static void listprepend_free (t_listprepend *x)
{
    listinlet_clear (&x->x_listinlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listprepend_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__prepend,
            (t_newmethod)listprepend_new,
            (t_method)listprepend_free,
            sizeof (t_listprepend),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)listprepend_list);
    class_addAnything (c, (t_method)listprepend_anything);
    
    class_setHelpName (c, &s_list);
    
    listprepend_class = c;
}

void listprepend_destroy (void)
{
    class_free (listprepend_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

