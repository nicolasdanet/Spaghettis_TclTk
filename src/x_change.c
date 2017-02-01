
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *change_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _change {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_change;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void change_bang (t_change *x)
{
    outlet_float (x->x_outlet, x->x_f);
}

static void change_float (t_change *x, t_float f)
{
    if (f != x->x_f) { x->x_f = f; change_bang (x); }
}

static void change_set (t_change *x, t_float f)
{
    x->x_f = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *change_new (t_float f)
{
    t_change *x = (t_change *)pd_new (change_class);
    
    x->x_f = f;
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void change_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_change,
            (t_newmethod)change_new,
            NULL,
            sizeof (t_change),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addBang (c, (t_method)change_bang);
    class_addFloat (c, (t_method)change_float);
    
    class_addMethod (c, (t_method)change_set, sym_set, A_DEFFLOAT, A_NULL);
        
    change_class = c;
}

void change_destroy (void)
{
    CLASS_FREE (change_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
