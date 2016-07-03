
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _guiconnect
{
    t_object    x_obj;
    t_pd        *x_owner;
    t_symbol    *x_bound;
    t_clock     *x_clock;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *guiconnect_class;   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void guiconnect_task (t_guiconnect *x)
{
    pd_free (cast_pd (x));
}

void guiconnect_release (t_guiconnect *x, double timeOut)
{
    PD_ASSERT (timeOut >= 0.0);
    
    if (!x->x_bound) { pd_free (cast_pd (x)); }
    else {
        x->x_owner = NULL;
        x->x_clock = clock_new (x, (t_method)guiconnect_task);
        clock_delay (x->x_clock, PD_MAX (0.0, timeOut));
    }    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guiconnect_anything (t_guiconnect *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner) { pd_message (x->x_owner, s, argc, argv); }
}

static void guiconnect_signoff (t_guiconnect *x)
{
    if (!x->x_owner) { pd_free (cast_pd (x)); }
    else {
        pd_unbind (cast_pd (x), x->x_bound); x->x_bound = NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_guiconnect *guiconnect_new (t_pd *owner, t_symbol *bindTo)
{
    t_guiconnect *x = (t_guiconnect *)pd_new (guiconnect_class);
    
    PD_ASSERT (owner);
    PD_ASSERT (bindTo);
    
    x->x_owner = owner;
    x->x_bound = bindTo;
    
    pd_bind (cast_pd (x), x->x_bound);
    
    return x;
}

static void guiconnect_free (t_guiconnect *x)
{
    if (x->x_bound) { pd_unbind (cast_pd (x), x->x_bound); }
    if (x->x_clock) { clock_free (x->x_clock); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void guiconnect_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_guiconnect,
            NULL,
            (t_method)guiconnect_free,
            sizeof (t_guiconnect), 
            CLASS_NOBOX,
            A_NULL);
        
    class_addAnything (c, guiconnect_anything);
    
    class_addMethod (c, (t_method)guiconnect_signoff, sym__signoff, A_NULL);
        
    guiconnect_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
