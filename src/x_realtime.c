
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
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
// MARK: -

static void realtime_bang (t_realtime *x)
{
    x->x_time = clock_getRealTimeInSeconds();
}

static void realtime_elapsed (t_realtime *x)
{
    double elapsed = clock_getRealTimeInSeconds() - x->x_time;
    
    outlet_float (x->x_outlet, (t_float)(SECONDS_TO_MILLISECONDS (elapsed)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *realtime_new (void)
{
    t_realtime *x = (t_realtime *)pd_new (realtime_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));

    inlet_new2 (x, &s_bang);

    realtime_bang (x);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    class_addMethod (c, (t_method)realtime_elapsed, sym__inlet2, A_NULL);
    
    realtime_class = c;
}

void realtime_destroy (void)
{
    class_free (realtime_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
