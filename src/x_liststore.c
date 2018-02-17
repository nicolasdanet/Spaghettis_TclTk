
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

static t_class *liststore_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _liststore {
    t_object        x_obj;                  /* Must be the first. */
    t_listinlet     x_listinlet;
    t_outlet        *x_outletLeft;
    t_outlet        *x_outletRight;
    } t_liststore;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void liststore_append (t_liststore *x, t_symbol *s, int argc, t_atom *argv)
{
    listinlet_listAppend (&x->x_listinlet, argc, argv);
}

static void liststore_prepend (t_liststore *x, t_symbol *s, int argc, t_atom *argv)
{
    listinlet_listPrepend (&x->x_listinlet, argc, argv);
}

static void liststore_get (t_liststore *x, t_symbol *s, int argc, t_atom *argv)
{
    int count = listinlet_getSize (&x->x_listinlet);
    int m = (int)atom_getFloatAtIndex (0, argc, argv);
    int n = (int)atom_getFloatAtIndex (1, argc, argv);
    
    if (m >= 0 && n > 0) {
    //
    if (m + n <= count) {
    //
    t_atom *t = NULL;
    
    PD_ATOMS_ALLOCA (t, count);
    
    if (listinlet_hasPointer (&x->x_listinlet)) {
    
        t_listinlet cache;
        listinlet_init (&cache);
        listinlet_clone (&x->x_listinlet, &cache);
        listinlet_copyAtomsUnchecked (&cache, t);
        outlet_list (x->x_outletLeft, n, t + m);
        listinlet_clear (&cache);
        
    } else {
    
        listinlet_copyAtomsUnchecked (&x->x_listinlet, t);
        outlet_list (x->x_outletLeft, n, t + m);
    }
    
    PD_ATOMS_FREEA (t, count);
    
    return;
    //
    }
    //
    }
    
    outlet_bang (x->x_outletRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void liststore_list (t_liststore *x, t_symbol *s, int argc, t_atom *argv)
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
        outlet_list (x->x_outletLeft, count, t);
        listinlet_clear (&cache);
        
    } else {
    
        listinlet_copyAtomsUnchecked (&x->x_listinlet, t + argc);
        outlet_list (x->x_outletLeft, count, t);
    }
    
    PD_ATOMS_FREEA (t, count);
}

static void liststore_anything (t_liststore *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)liststore_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *liststore_new (t_symbol *s, int argc, t_atom *argv)
{
    t_liststore *x = (t_liststore *)pd_new (liststore_class);
    
    listinlet_init (&x->x_listinlet);
    listinlet_listSet (&x->x_listinlet, argc, argv);
    
    x->x_outletLeft  = outlet_newList (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    inlet_new (cast_object (x), cast_pd (&x->x_listinlet), NULL, NULL);
    
    return x;
}

static void liststore_free (t_liststore *x)
{
    listinlet_clear (&x->x_listinlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void liststore_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__store,
            (t_newmethod)liststore_new,
            (t_method)liststore_free,
            sizeof (t_liststore),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)liststore_list);
    class_addAnything (c, (t_method)liststore_anything);
    
    class_addMethod (c, (t_method)liststore_append,     sym_append,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)liststore_prepend,    sym_prepend,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)liststore_get,        sym_get,        A_GIMME, A_NULL);
    
    class_setHelpName (c, &s_list);
    
    liststore_class = c;
}

void liststore_destroy (void)
{
    class_free (liststore_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

