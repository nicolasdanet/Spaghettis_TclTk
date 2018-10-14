
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *mag_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _mag_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_mag_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that the two signals incoming could be theoretically just one. */
/* But as only loads are done, it is assumed safe to use restricted pointers. */

static void mag_tilde_dsp (t_mag_tilde *x, t_signal **sp)
{
    dsp_addMagnitudePerform (sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *mag_tilde_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_mag_tilde *x = (t_mag_tilde *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__signals);
    object_getSignalValues (cast_object (x), b, 2);
    
    return b;
    //
    }
    
    return NULL;
}

void mag_tilde_signals (t_mag_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    object_setSignalValues (cast_object (x), argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *mag_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_mag_tilde *x = (t_mag_tilde *)pd_new (mag_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    inlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void mag_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mag__tilde__,
            (t_newmethod)mag_tilde_new,
            NULL,
            sizeof (t_mag_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    CLASS_SIGNAL (c, t_mag_tilde, x_f);
    
    class_addDSP (c, (t_method)mag_tilde_dsp);
    
    class_addMethod (c, (t_method)mag_tilde_signals, sym__signals, A_GIMME, A_NULL);
    
    class_setDataFunction (c, mag_tilde_functionData);
    
    mag_tilde_class = c;
}

void mag_tilde_destroy (void)
{
    class_free (mag_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

