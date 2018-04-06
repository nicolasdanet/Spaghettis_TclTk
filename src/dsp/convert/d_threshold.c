
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *threshold_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _threshold_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    int         x_state;
    t_float     x_high;
    t_float     x_low;
    t_float     x_wait;
    t_float     x_deadTimeHigh;
    t_float     x_deadTimeLow;
    t_float     x_millisecondsPerTick;
    t_clock     *x_clock;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_threshold_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void threshold_tilde_tick (t_threshold_tilde *x)  
{
    if (x->x_state) { outlet_bang (x->x_outletLeft); }
    else {
        outlet_bang (x->x_outletRight);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void threshold_tilde_set (t_threshold_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float high      = atom_getFloatAtIndex (0, argc, argv);
    t_float highDead  = atom_getFloatAtIndex (1, argc, argv);
    t_float low       = atom_getFloatAtIndex (2, argc, argv);
    t_float lowDead   = atom_getFloatAtIndex (3, argc, argv);
    
    x->x_high         = high;
    x->x_deadTimeHigh = highDead;
    x->x_low          = low;
    x->x_deadTimeLow  = lowDead;
}

static void threshold_tilde_state (t_threshold_tilde *x, t_float f)
{
    x->x_state = (f != 0.0);
    x->x_wait  = 0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *threshold_tilde_perform (t_int *w)
{
    t_threshold_tilde *x = (t_threshold_tilde *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    if (x->x_wait > 0.0) { x->x_wait -= x->x_millisecondsPerTick; }
    else if (x->x_state) {
    
        while (n--) {
        //
        if ((*in++) < x->x_low) {
            x->x_state = 0;
            x->x_wait  = x->x_deadTimeLow;
            clock_delay (x->x_clock, 0.0);
            break;
        }
        //
        }
        
    } else {
 
        while (n--) {
        //
        if ((*in++) >= x->x_high) {
            x->x_state = 1;
            x->x_wait  = x->x_deadTimeHigh;
            clock_delay (x->x_clock, 0.0);
            break;
        }
        //
        }
    }
    
    return (w + 4);
}

void threshold_tilde_dsp (t_threshold_tilde *x, t_signal **sp)
{
    x->x_millisecondsPerTick = (t_float)(1000.0 * sp[0]->s_vectorSize / sp[0]->s_sampleRate);
    
    dsp_add (threshold_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *threshold_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_threshold_tilde *x = (t_threshold_tilde *)pd_new (threshold_tilde_class);
    
    x->x_clock       = clock_new ((void *)x, (t_method)threshold_tilde_tick);
    x->x_outletLeft  = outlet_newBang (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    inlet_new2 (x, &s_float);

    threshold_tilde_set (x, s, argc, argv);
    
    return x;
}

static void threshold_tilde_free (t_threshold_tilde *x)
{
    clock_free (x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void threshold_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_threshold__tilde__,
            (t_newmethod)threshold_tilde_new,
            (t_method)threshold_tilde_free,
            sizeof (t_threshold_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    CLASS_SIGNAL (c, t_threshold_tilde, x_f);
    
    class_addDSP (c, (t_method)threshold_tilde_dsp);
        
    class_addMethod (c, (t_method)threshold_tilde_state, sym__inlet2,   A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)threshold_tilde_set,   sym_set,       A_GIMME, A_NULL);
        
    threshold_tilde_class = c;
}

void threshold_tilde_destroy (void)
{
    class_free (threshold_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
