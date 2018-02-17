
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_list.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listappend_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listappend {
    t_object        x_obj;                  /* Must be the first. */
    t_listinlet     x_listinlet;
    t_outlet        *x_outlet;
    } t_listappend;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void listappend_list (t_listappend *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    int count = listinlet_getSize (&x->x_listinlet) + argc;
    
    PD_ATOMS_ALLOCA (t, count);
    
    atom_copyAtoms (argv, argc, t, argc);
    
    if (listinlet_hasPointer (&x->x_listinlet)) {
    
        t_listinlet cache;
        listinlet_init (&cache);
        listinlet_clone (&x->x_listinlet, &cache);
        listinlet_copyAtomsUnchecked (&cache, t + argc);
        outlet_list (x->x_outlet, count, t);
        listinlet_clear (&cache);
        
    } else {
    
        listinlet_copyAtomsUnchecked (&x->x_listinlet, t + argc);
        outlet_list (x->x_outlet, count, t);
    }
    
    PD_ATOMS_FREEA (t, count);
}

static void listappend_anything (t_listappend *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)listappend_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *listappend_new (t_symbol *s, int argc, t_atom *argv)
{
    t_listappend *x = (t_listappend *)pd_new (listappend_class);
    
    listinlet_init (&x->x_listinlet);
    listinlet_listSet (&x->x_listinlet, argc, argv);
    
    x->x_outlet = outlet_newList (cast_object (x));
    
    inlet_new (cast_object (x), cast_pd (&x->x_listinlet), NULL, NULL);
    
    return x;
}

static void listappend_free (t_listappend *x)
{
    listinlet_clear (&x->x_listinlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listappend_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__append,
            (t_newmethod)listappend_new,
            (t_method)listappend_free,
            sizeof (t_listappend),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)listappend_list);
    class_addAnything (c, (t_method)listappend_anything);
    
    class_setHelpName (c, &s_list);
    
    listappend_class = c;
}

void listappend_destroy (void)
{
    class_free (listappend_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

