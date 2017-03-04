
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *atan2_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _atan2 {
    t_object    x_obj;
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_atan2;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *atan2_new (void)
{
    t_atan2 *x = (t_atan2 *)pd_new (atan2_class);
    
    x->x_f = (t_float)0.0;
    x->x_outlet = outlet_new (cast_object (x), &s_float);
        
    inlet_newFloat (cast_object (x), &x->x_f);

    return x;
}

static void atan2_float (t_atan2 *x, t_float f)
{
    outlet_float (x->x_outlet, (f == 0.0 && x->x_f == 0.0 ? (t_float)0.0 : atan2f (f, x->x_f)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void atan2_setup (void)
{
    t_class *c = NULL;

    c = class_new (sym_atan2,
            (t_newmethod)atan2_new,
            NULL,
            sizeof (t_atan2),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addFloat (c, (t_method)atan2_float);
    
    class_setHelpName (c, sym_sqrt);
    
    atan2_class = c;
}

void atan2_destroy (void)
{
    CLASS_FREE (atan2_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
