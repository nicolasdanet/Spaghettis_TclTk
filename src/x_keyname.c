
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *keyname_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _keyname {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_keyname;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void keyname_list (t_keyname *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_symbol (x->x_outletRight, atom_getSymbolAtIndex (1, argc, argv));
    outlet_float (x->x_outletLeft, atom_getFloatAtIndex (0, argc, argv));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *keyname_new (void)
{
    t_keyname *x = (t_keyname *)pd_new (keyname_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_symbol);
    
    pd_bind (cast_pd (x), sym__keyname);
    
    return x;
}

static void keyname_free (t_keyname *x)
{
    pd_unbind (cast_pd (x), sym__keyname);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void keyname_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_keyname,
            (t_newmethod)keyname_new,
            (t_method)keyname_free,
            sizeof (t_keyname),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
            
    class_addList (c, (t_method)keyname_list);

    class_setHelpName (c, sym_key);
    
    keyname_class = c;
}

void keyname_destroy (void)
{
    CLASS_FREE (keyname_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
