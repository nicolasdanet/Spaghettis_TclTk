
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *bang_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _bang_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outlet;
    t_clock     *x_clock;
    } t_bang_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bang_tilde_task (t_bang_tilde *x)
{
    outlet_bang (x->x_outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *bang_tilde_perform (t_int *w)
{
    t_bang_tilde *x = (t_bang_tilde *)(w[1]);
    
    clock_delay (x->x_clock, 0.0);
    
    return (w + 2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void bang_tilde_dsp (t_bang_tilde *x, t_signal **sp)
{
    dsp_add (bang_tilde_perform, 1, x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *bang_tilde_new (void)
{
    t_bang_tilde *x = (t_bang_tilde *)pd_new (bang_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_bang);
    x->x_clock  = clock_new ((void *)x, (t_method)bang_tilde_task);
    
    return x;
}

static void bang_tilde_free (t_bang_tilde *x)
{
    clock_free (x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void bang_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_bang__tilde__,
            (t_newmethod)bang_tilde_new,
            (t_method)bang_tilde_free,
            sizeof (t_bang_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addDSP (c, (t_method)bang_tilde_dsp);
    
    bang_tilde_class = c;
}

void bang_tilde_destroy (void)
{
    CLASS_FREE (bang_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
