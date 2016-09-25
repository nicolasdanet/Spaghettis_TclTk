
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listtrim_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listtrim {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outlet;
    } t_listtrim;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void listtrim_list (t_listtrim *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || !IS_SYMBOL (argv)) { outlet_list (x->x_outlet, argc, argv); }
    else { 
        outlet_anything (x->x_outlet, GET_SYMBOL (argv), argc - 1, argv + 1);
    }
}

static void listtrim_anything (t_listtrim *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->x_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *listtrim_new (t_symbol *s, int argc, t_atom *argv)
{
    t_listtrim *x = (t_listtrim *)pd_new (listtrim_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listtrim_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__trim,
            (t_newmethod)listtrim_new,
            NULL,
            sizeof (t_listtrim),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, listtrim_list);
    class_addAnything (c, listtrim_anything);
    
    class_setHelpName (c, &s_list);
    
    listtrim_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

