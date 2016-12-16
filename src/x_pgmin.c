
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
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *pgmin_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pgmin {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_channel;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_pgmin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pgmin_list (t_pgmin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value   = atom_getFloatAtIndex (0, argc, argv);
    t_float channel = atom_getFloatAtIndex (1, argc, argv);
    
    if (x->x_channel) {
        if (x->x_channel == channel) { outlet_float (x->x_outletLeft, value); }
        
    } else {
        outlet_float (x->x_outletRight, channel);
        outlet_float (x->x_outletLeft, value);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *pgmin_new (t_float f)
{
    t_pgmin *x = (t_pgmin *)pd_new (pgmin_class);
    
    x->x_channel    = f;
    x->x_outletLeft = outlet_new (cast_object (x), &s_float);
    
    if (!x->x_channel) { x->x_outletRight = outlet_new (cast_object (x), &s_float); }
    
    pd_bind (cast_pd (x), sym__pgmin);
    
    return x;
}

static void pgmin_free (t_pgmin *x)
{
    pd_unbind (cast_pd (x), sym__pgmin);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pgmin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pgmin,
            (t_newmethod)pgmin_new,
            (t_method)pgmin_free,
            sizeof (t_pgmin),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFFLOAT,
            A_NULL);
            
    class_addList (c, (t_method)pgmin_list);
    
    class_setHelpName (c, sym_midiout);
    
    pgmin_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
