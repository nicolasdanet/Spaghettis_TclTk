
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
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *line_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _line {
    t_object    x_obj;              /* Must be the first. */
    t_systime   x_targetTime;
    t_systime   x_startTime;
    double      x_rampTime;
    t_float     x_targetValue;
    t_float     x_startValue;
    t_float     x_grain;
    int         x_hasRamp;
    t_outlet    *x_outlet;
    t_clock     *x_clock;
    } t_line;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void line_set (t_line *, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_float line_valueAtTime (t_line *x, t_systime t)
{
    double a = t - x->x_startTime;
    double b = x->x_targetTime - x->x_startTime;
    double y = x->x_targetValue - x->x_startValue;
        
    PD_ASSERT (b != 0.0); 
    
    return (t_float)(x->x_startValue + (a / b * y));
}

static void line_task (t_line *x)
{
    double remains = - scheduler_getMillisecondsSince (x->x_targetTime);
    
    if (remains < math_epsilon()) { outlet_float (x->x_outlet, x->x_targetValue); }
    else {
    //
    outlet_float (x->x_outlet, line_valueAtTime (x, scheduler_getLogicalTime()));
    
    clock_delay (x->x_clock, PD_MIN (x->x_grain, remains));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void line_float (t_line *x, t_float f)
{
    t_systime now   = scheduler_getLogicalTime();
    t_systime after = scheduler_getLogicalTimeAfter (x->x_rampTime);
    
    if (x->x_hasRamp && (after > now)) {
    
        x->x_hasRamp = 0;
        
        if (now > x->x_targetTime) { x->x_startValue = x->x_targetValue; }      /* Usual case. */
        else { 
            x->x_startValue = line_valueAtTime (x, now);                        /* Retriggered case. */
        }
        
        x->x_startTime   = now;
        x->x_targetTime  = after;
        x->x_targetValue = f;
        
        line_task (x);
        
        clock_delay (x->x_clock, PD_MIN (x->x_grain, x->x_rampTime));
    
    } else {
    
        line_set (x, f);
        outlet_float (x->x_outlet, f);
    }
}

static void line_floatRamp (t_line *x, t_float f)
{
    x->x_rampTime = f; x->x_hasRamp = 1;
}

static void line_floatGrain (t_line *x, t_float f)
{
    x->x_grain = (f <= 0.0 ? TIME_DEFAULT_GRAIN : f);
}

static void line_stop (t_line *x)
{
    line_set (x, x->x_startValue);
}

static void line_set (t_line *x, t_float f)
{
    clock_unset (x->x_clock); x->x_targetValue = x->x_startValue = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *line_new (t_float f, t_float grain)
{
    t_line *x = (t_line *)pd_new (line_class);
    
    x->x_targetTime     = scheduler_getLogicalTime();
    x->x_startTime      = x->x_targetTime;
    x->x_targetValue    = f;
    x->x_startValue     = f;
    x->x_hasRamp        = 0;
    x->x_outlet         = outlet_new (cast_object (x), &s_float);
    x->x_clock          = clock_new ((void *)x, (t_method)line_task);
    
    line_floatGrain (x, grain);
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet3);
    
    return x;
}

static void line_free (t_line *x)
{
    clock_free (x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void line_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_line,
            (t_newmethod)line_new,
            (t_method)line_free,
            sizeof (t_line),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
        
    class_addFloat (c, line_float);
        
    class_addMethod (c, (t_method)line_floatRamp,   sym_inlet2, A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)line_floatGrain,  sym_inlet3, A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)line_stop,        sym_stop,   A_NULL);
    class_addMethod (c, (t_method)line_set,         sym_set,    A_FLOAT, A_NULL);

    line_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
