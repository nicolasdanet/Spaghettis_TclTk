
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

static t_class *cputime_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _cputime {
    t_object    x_obj;                  /* Must be the first. */
    double      x_time;
    t_outlet    *x_outlet;
    } t_cputime;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void cputime_bang (t_cputime *x)
{
    x->x_time = sys_getRealTimeInSeconds();
}

static void cputime_elapsed (t_cputime *x)
{
    double elapsed = sys_getRealTimeInSeconds() - x->x_time;
    
    outlet_float (x->x_outlet, (t_float)(SECONDS_TO_MILLISECONDS (elapsed)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *cputime_new (void)
{
    t_cputime *x = (t_cputime *)pd_new (cputime_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);

    inlet_new (cast_object (x), cast_pd (x), &s_bang, sym_inlet2);

    cputime_bang (x);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void cputime_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_cputime,
            (t_newmethod)cputime_new,
            NULL,
            sizeof (t_cputime),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addBang (c, cputime_bang);
    
    class_addMethod (c, (t_method)cputime_elapsed, sym_inlet2, A_NULL);
    
    cputime_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
