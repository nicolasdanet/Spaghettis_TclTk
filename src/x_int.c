
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *int_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _intobject {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_intobject;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void int_bang (t_intobject *x)
{
    int i = (int)x->x_f; outlet_float (x->x_outlet, (t_float)i);
}

static void int_float (t_intobject *x, t_float f)
{
    x->x_f = f; int_bang (x);
}

static void int_send (t_intobject *x, t_symbol *s)
{
    if (pd_isThing (s)) { int i = (int)x->x_f; pd_float (s->s_thing, (t_float)i); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *int_new (t_float f)
{
    t_intobject *x = (t_intobject *)pd_new (int_class);
    
    x->x_f = f;
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    inlet_newFloat (cast_object (x), &x->x_f);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void int_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_int,
            (t_newmethod)int_new,
            NULL,
            sizeof (t_intobject),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addCreator ((t_newmethod)int_new, sym_i, A_DEFFLOAT, A_NULL);
    
    class_addBang (c, int_bang);
    class_addFloat (c, int_float);
    
    class_addMethod (c, (t_method)int_send, sym_send, A_SYMBOL, A_NULL);
    
    int_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
