
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *polytouchout_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _polytouchout {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_pitch;
    t_float     x_channel;
    } t_polytouchout;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void polytouchout_float (t_polytouchout *x, t_float n)
{
    outmidi_polyPressure (x->x_channel, x->x_pitch, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *polytouchout_new (t_float channel)
{
    t_polytouchout *x = (t_polytouchout *)pd_new (polytouchout_class);
    
    x->x_pitch   = 0;
    x->x_channel = channel;
    
    inlet_newFloat (cast_object (x), &x->x_pitch);
    inlet_newFloat (cast_object (x), &x->x_channel);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void polytouchout_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_polytouchout, 
            (t_newmethod)polytouchout_new,
            NULL,
            sizeof (t_polytouchout),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addFloat (c, (t_method)polytouchout_float);
    
    class_setHelpName (c, sym_midiout);
    
    polytouchout_class = c;
}

void polytouchout_destroy (void)
{
    CLASS_FREE (polytouchout_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
