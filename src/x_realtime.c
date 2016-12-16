
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

static t_class *realtime_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _realtime {
    t_object    x_obj;                  /* Must be the first. */
    double      x_time;
    t_outlet    *x_outlet;
    } t_realtime;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void realtime_bang (t_realtime *x)
{
    x->x_time = sys_getRealTimeInSeconds();
}

static void realtime_elapsed (t_realtime *x)
{
    double elapsed = sys_getRealTimeInSeconds() - x->x_time;
    
    outlet_float (x->x_outlet, (t_float)(SECONDS_TO_MILLISECONDS (elapsed)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *realtime_new (void)
{
    t_realtime *x = (t_realtime *)pd_new (realtime_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);

    inlet_new (cast_object (x), cast_pd (x), &s_bang, sym_inlet2);

    realtime_bang (x);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void realtime_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_realtime,
            (t_newmethod)realtime_new,
            NULL,
            sizeof (t_realtime),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addCreator ((t_newmethod)realtime_new, sym_cputime, A_NULL);      /* Currently the same thing. */
    
    class_addBang (c, (t_method)realtime_bang);
    
    class_addMethod (c, (t_method)realtime_elapsed, sym_inlet2, A_NULL);
    
    realtime_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
