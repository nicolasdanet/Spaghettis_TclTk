
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *timer_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _timer {
    t_object    x_obj;              /* Must be the first. */
    t_systime   x_start;
    t_float     x_unit;
    int         x_isSamples;        /* Samples or milliseconds. */
    t_outlet    *x_outlet;
    } t_timer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void timer_bang (t_timer *x)
{
    x->x_start = scheduler_getLogicalTime();
}

static void timer_bangElapsed (t_timer *x)
{
    outlet_float (x->x_outlet, scheduler_getUnitsSince (x->x_start, x->x_unit, x->x_isSamples));
}

static void timer_unit (t_timer *x, t_float f, t_symbol *unitName)
{
    t_error err = clock_parseUnit (f, unitName, &x->x_unit, &x->x_isSamples);
    
    if (err) {
        error_invalid (sym_timer, sym_unit); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *timer_new (t_float unit, t_symbol *unitName)
{
    t_timer *x = (t_timer *)pd_new (timer_class);
    
    x->x_unit      = 1;
    x->x_isSamples = 0;
    
    timer_bang (x);
        
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    inlet_new (cast_object (x), cast_pd (x), &s_bang, sym_inlet2);
    
    if (unit != 0.0) { timer_unit (x, unit, unitName); }
        
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void timer_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_timer,
            (t_newmethod)timer_new,
            NULL,
            sizeof (t_timer),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addBang (c, timer_bang);
    
    class_addMethod (c, (t_method)timer_bangElapsed,    sym_inlet2, A_NULL);
    class_addMethod (c, (t_method)timer_unit,           sym_unit,   A_FLOAT, A_SYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)timer_unit,           sym_tempo,  A_FLOAT, A_SYMBOL, A_NULL);
        
    #endif
    
    timer_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
