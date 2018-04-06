
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *midiout_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _midiout {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_port;
    } t_midiout;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void midiout_float (t_midiout *x, t_float f)
{
    int port = (int)PD_ABS (x->x_port);
    int byte = (int)f;
    
    byte = PD_CLAMP (byte, 0, 0xff);
    
    midi_broadcast (port, 1, byte, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *midiout_new (t_float port)
{
    t_midiout *x = (t_midiout *)pd_new (midiout_class);
    
    x->x_port = PD_ABS (port);
    
    inlet_newFloat (cast_object (x), &x->x_port);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midiout_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_midiout,
            (t_newmethod)midiout_new,
            NULL,
            sizeof (t_midiout),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
        
    class_addFloat (c, (t_method)midiout_float);
    
    midiout_class = c;
}

void midiout_destroy (void)
{
    class_free (midiout_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
