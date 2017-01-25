
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

static t_class *ctlin_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _ctlin {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_control;
    t_float     x_channel;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletMiddle;
    t_outlet    *x_outletRight;
    } t_ctlin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void ctlin_list (t_ctlin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float control = atom_getFloatAtIndex (0, argc, argv);
    t_float value   = atom_getFloatAtIndex (1, argc, argv);
    t_float channel = atom_getFloatAtIndex (2, argc, argv);
    
    if (x->x_control >= 0.0 && x->x_control != control) { return; }
    if (x->x_channel > 0.0  && x->x_channel != channel) { return; }
    
    if (x->x_outletRight)  { outlet_float (x->x_outletRight, channel);  }
    if (x->x_outletMiddle) { outlet_float (x->x_outletMiddle, control); }
    
    outlet_float (x->x_outletLeft, value);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *ctlin_new (t_symbol *s, int argc, t_atom *argv)
{
    t_ctlin *x = (t_ctlin *)pd_new (ctlin_class);
    
    x->x_control    = argc > 0 ? atom_getFloatAtIndex (0, argc, argv) : (t_float)-1.0;
    x->x_channel    = argc > 1 ? atom_getFloatAtIndex (1, argc, argv) : (t_float)0.0;
    x->x_outletLeft = outlet_new (cast_object (x), &s_float);
    
    if (x->x_control < 0.0)  { x->x_outletMiddle = outlet_new (cast_object (x), &s_float); }
    if (x->x_channel <= 0.0) { x->x_outletRight  = outlet_new (cast_object (x), &s_float); }
    
    pd_bind (cast_pd (x), sym__ctlin);
    
    return x;
}

static void ctlin_free (t_ctlin *x)
{
    pd_unbind (cast_pd (x), sym__ctlin);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ctlin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_ctlin,
            (t_newmethod)ctlin_new, 
            (t_method)ctlin_free,
            sizeof (t_ctlin),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)ctlin_list);
    
    class_setHelpName (c, sym_midiout);
    
    ctlin_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
