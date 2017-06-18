
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"

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
// MARK: -

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
    if (pd_hasThing (s)) { int i = (int)x->x_f; pd_float (pd_getThing (s), (t_float)i); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
    
    class_addBang (c, (t_method)int_bang);
    class_addFloat (c, (t_method)int_float);
    
    class_addMethod (c, (t_method)int_send, sym_send, A_SYMBOL, A_NULL);
    
    int_class = c;
}

void int_destroy (void)
{
    class_free (int_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
