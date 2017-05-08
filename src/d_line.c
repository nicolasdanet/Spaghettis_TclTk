
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *line_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _line_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_sample    x_target;
    t_sample    x_current;
    t_sample    x_incrementPerTick;
    t_sample    x_incrementPerSample;
    t_float     x_inverseOfVectorSize;
    t_float     x_millisecondsToTicks;
    t_float     x_timeRamp;
    t_float     x_timeRampCurrent;
    int         x_ticksLeft;
    int         x_retarget;
    t_outlet    *x_outlet;
    } t_line_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void line_tilde_float (t_line_tilde *x, t_float f)
{
    if (x->x_timeRamp <= 0) {
    
        x->x_target             = f;
        x->x_current            = f;
        x->x_ticksLeft          = 0;
        x->x_retarget           = 0;
        
    } else {
    
        x->x_target             = f;
        x->x_retarget           = 1;
        x->x_timeRampCurrent    = x->x_timeRamp;
        x->x_timeRamp           = (t_float)0.0;
    }
}

static void line_tilde_stop (t_line_tilde *x)
{
    x->x_target     = x->x_current;
    x->x_ticksLeft  = 0;
    x->x_retarget   = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *line_tilde_perform (t_int *w)
{
    t_line_tilde *x = (t_line_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample f;
    
    if (x->x_retarget) {
    
        int numberOfTicks = PD_MAX (1, (int)(x->x_timeRampCurrent * x->x_millisecondsToTicks));
        
        x->x_ticksLeft          = numberOfTicks;
        x->x_incrementPerTick   = (x->x_target - x->x_current) / (t_float)(numberOfTicks);
        x->x_incrementPerSample = x->x_incrementPerTick * x->x_inverseOfVectorSize;
        x->x_retarget           = 0;
    }
    
    if (x->x_ticksLeft) {
    
        f = x->x_current;
        while (n--) { *out++ = f; f += x->x_incrementPerSample; }
        x->x_current += x->x_incrementPerTick; x->x_ticksLeft--;
        
    } else {
    
        f = x->x_target;
        while (n--) { *out++ = f; }
        x->x_current = f;
    }
    
    return (w + 4);
}

static void line_tilde_dsp (t_line_tilde *x, t_signal **sp)
{
    x->x_inverseOfVectorSize = (t_float)(1.0 / sp[0]->s_vectorSize);
    x->x_millisecondsToTicks = (t_float)(sp[0]->s_sampleRate / (1000.0 * sp[0]->s_vectorSize));
    
    dsp_add (line_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *line_tilde_new (void)
{
    t_line_tilde *x = (t_line_tilde *)pd_new (line_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newFloat (cast_object (x), &x->x_timeRamp);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void line_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_line__tilde__,
            (t_newmethod)line_tilde_new,
            NULL,
            sizeof (t_line_tilde),
            CLASS_DEFAULT,
            A_NULL);
        
    class_addDSP (c, (t_method)line_tilde_dsp);
    class_addFloat (c, (t_method)line_tilde_float);
    
    class_addMethod (c, (t_method)line_tilde_stop, sym_stop, A_NULL);
        
    line_tilde_class = c;
}

void line_tilde_destroy (void)
{
    CLASS_FREE (line_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
