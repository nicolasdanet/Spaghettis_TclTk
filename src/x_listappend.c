
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
#include "x_control.h"

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
#pragma mark -

static void listappend_list (t_listappend *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    int count = listinlet_getSize (&x->x_listinlet) + argc;
    
    ATOMS_ALLOCA (t, count);
    
    atom_copyAtomsUnchecked (argc, argv, t);
    
    if (listinlet_hasPointer (&x->x_listinlet)) {
    
        t_listinlet cache;
        listinlet_init (&cache);
        listinlet_clone (&x->x_listinlet, &cache);
        listinlet_copyListUnchecked (&cache, t + argc);
        outlet_list (x->x_outlet, count, t);
        listinlet_clear (&cache);
        
    } else {
    
        listinlet_copyListUnchecked (&x->x_listinlet, t + argc);
        outlet_list (x->x_outlet, count, t);
    }
    
    ATOMS_FREEA (t, count);
}

static void listappend_anything (t_listappend *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), listappend_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *listappend_new (t_symbol *s, int argc, t_atom *argv)
{
    t_listappend *x = (t_listappend *)pd_new (listappend_class);
    
    listinlet_init (&x->x_listinlet);
    listinlet_setList (&x->x_listinlet, argc, argv);
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    inlet_new (cast_object (x), cast_pd (&x->x_listinlet), NULL, NULL);
    
    return x;
}

static void listappend_free (t_listappend *x)
{
    listinlet_clear (&x->x_listinlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

