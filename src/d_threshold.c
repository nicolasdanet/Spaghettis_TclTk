
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
    t_float     x_highDeadTime;
    t_float     x_lowDeadTime;
    t_float     x_millisecondsPerTick;
    t_clock     *x_clock;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_threshold_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void threshold_tilde_tick (t_threshold_tilde *x)  
{
    if (x->x_state) { outlet_bang (x->x_outletLeft); }
    else {
        outlet_bang (x->x_outletRight);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void threshold_tilde_set (t_threshold_tilde *x, 
    t_float high, 
    t_float highDead, 
    t_float low, 
    t_float lowDead)
{
    x->x_high         = high;
    x->x_highDeadTime = highDead;
    x->x_low          = PD_MIN (low, high);
    x->x_lowDeadTime  = lowDead;
}

static void threshold_tilde_state (t_threshold_tilde *x, t_float f)
{
    x->x_state = (f != 0.0);
    x->x_wait  = (t_float)0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
            x->x_wait  = x->x_lowDeadTime;
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
            x->x_wait  = x->x_highDeadTime;
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
#pragma mark -

static void *threshold_tilde_new (t_float high, t_float highDead, t_float low, t_float lowDead)
{
    t_threshold_tilde *x = (t_threshold_tilde *)pd_new (threshold_tilde_class);
    
    x->x_clock       = clock_new ((void *)x, (t_method)threshold_tilde_tick);
    x->x_outletLeft  = outlet_new (cast_object (x), &s_bang);
    x->x_outletRight = outlet_new (cast_object (x), &s_bang);
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);

    threshold_tilde_set (x, high, highDead, low, lowDead);
    
    return x;
}

static void threshold_tilde_free (t_threshold_tilde *x)
{
    clock_free (x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void threshold_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_threshold__tilde__,
            (t_newmethod)threshold_tilde_new,
            (t_method)threshold_tilde_free,
            sizeof (t_threshold_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_threshold_tilde, x_f);
    
    class_addDSP (c, (t_method)threshold_tilde_dsp);
        
    class_addMethod (c, (t_method)threshold_tilde_state, sym_inlet2, A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)threshold_tilde_set,
        sym_set,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    threshold_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
