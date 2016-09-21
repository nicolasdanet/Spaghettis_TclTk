
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
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listtosymbol_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listtosymbol {
    t_object    x_obj;                          /* Must be the first. */
    t_outlet    *x_outlet;
    } t_listtosymbol;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void listtosymbol_list (t_listtosymbol *x, t_symbol *s, int argc, t_atom *argv)
{
    char *t = (char *)PD_MEMORY_GET ((argc + 1) * sizeof (char));
    
    int i;
    for (i = 0; i < argc; i++) { t[i] = (char)atom_getFloatAtIndex (i, argc, argv); }

    PD_ASSERT (t[argc] == 0);
    
    outlet_symbol (x->x_outlet, gensym (t));
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *listtosymbol_new (t_symbol *s, int argc, t_atom *argv)
{
    t_listtosymbol *x = (t_listtosymbol *)pd_new (listtosymbol_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_symbol);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listtosymbol_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__tosymbol,
            (t_newmethod)listtosymbol_new,
            NULL,
            sizeof (t_listtosymbol),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, listtosymbol_list);
    
    class_setHelpName (c, &s_list);
    
    listtosymbol_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

