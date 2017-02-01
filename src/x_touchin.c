
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
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *touchin_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _touchin {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_channel;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_touchin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void touchin_list (t_touchin *x, t_symbol *s, int argc, t_atom *argv)
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

static void *touchin_new (t_float f)
{
    t_touchin *x = (t_touchin *)pd_new (touchin_class);
    
    x->x_channel    = f;
    x->x_outletLeft = outlet_new (cast_object (x), &s_float);
    
    if (!x->x_channel) { x->x_outletRight = outlet_new (cast_object (x), &s_float); }
    
    pd_bind (cast_pd (x), sym__touchin);
    
    return x;
}

static void touchin_free (t_touchin *x)
{
    pd_unbind (cast_pd (x), sym__touchin);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void touchin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_touchin,
            (t_newmethod)touchin_new,
            (t_method)touchin_free,
            sizeof (t_touchin),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFFLOAT,
            A_NULL);
            
    class_addList (c, (t_method)touchin_list);
    
    class_setHelpName (c, sym_midiout);
    
    touchin_class = c;
}

void touchin_destroy (void)
{
    CLASS_FREE (touchin_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
