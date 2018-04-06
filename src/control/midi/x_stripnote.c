
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *stripnote_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _stripnote {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_velocity;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_stripnote;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void stripnote_float (t_stripnote *x, t_float f)
{
    if (x->x_velocity) { 
        outlet_float (x->x_outletRight, x->x_velocity);
        outlet_float (x->x_outletLeft,  f);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *stripnote_new (void)
{
    t_stripnote *x = (t_stripnote *)pd_new (stripnote_class);
        
    x->x_outletLeft  = outlet_newFloat (cast_object (x));
    x->x_outletRight = outlet_newFloat (cast_object (x));
    
    inlet_newFloat (cast_object (x), &x->x_velocity);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void stripnote_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_stripnote,
            (t_newmethod)stripnote_new,
            NULL,
            sizeof (t_stripnote),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addFloat (c, (t_method)stripnote_float);
    
    stripnote_class = c;
}

void stripnote_destroy (void)
{
    class_free (stripnote_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
