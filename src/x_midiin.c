
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

static t_class *midiin_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _midiin {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_midiin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void midiin_list (t_midiin *x, t_symbol *s, int argc, t_atom *argv)
{
    int byte = (int)atom_getFloatAtIndex (0, argc, argv);
    int port = (int)atom_getFloatAtIndex (1, argc, argv);

    outlet_float (x->x_outletRight, (t_float)port);
    outlet_float (x->x_outletLeft,  (t_float)byte);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *midiin_new (void)
{
    t_midiin *x = (t_midiin *)pd_new (midiin_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_float);
    
    pd_bind (cast_pd (x), sym__midiin);
    
    return x;
}

static void midiin_free (t_midiin *x)
{
    pd_unbind (cast_pd (x), sym__midiin);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midiin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_midiin,
            (t_newmethod)midiin_new,
            (t_method)midiin_free,
            sizeof (t_midiin),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
            
    class_addList (c, midiin_list);
    
    class_setHelpName (c, sym_midiout);
    
    midiin_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
