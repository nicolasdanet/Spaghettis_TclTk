
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listsplit_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listsplit {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletMiddle;
    t_outlet    *x_outletRight;
}   t_listsplit;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void listsplit_list (t_listsplit *x, t_symbol *s, int argc, t_atom *argv)
{
    int n = (int)PD_MAX (0.0, x->x_f);
    
    if (argc >= n) {
        outlet_list (x->x_outletMiddle, argc - n, argv + n);
        outlet_list (x->x_outletLeft,   n, argv);
    } else {
        outlet_list (x->x_outletRight,  argc, argv);
    }
}

static void listsplit_anything (t_listsplit *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)listsplit_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *listsplit_new (t_symbol *s, int argc, t_atom *argv)
{
    t_listsplit *x = (t_listsplit *)pd_new (listsplit_class);
    
    x->x_f = atom_getFloatAtIndex (0, argc, argv);
    
    x->x_outletLeft   = outlet_new (cast_object (x), &s_list);
    x->x_outletMiddle = outlet_new (cast_object (x), &s_list);
    x->x_outletRight  = outlet_new (cast_object (x), &s_list);
    
    inlet_newFloat (cast_object (x), &x->x_f);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listsplit_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__split,
            (t_newmethod)listsplit_new,
            NULL,
            sizeof (t_listsplit),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)listsplit_list);
    class_addAnything (c, (t_method)listsplit_anything);
    
    class_setHelpName (c, &s_list);
    
    listsplit_class = c;
}

void listsplit_destroy (void)
{
    class_free (listsplit_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

