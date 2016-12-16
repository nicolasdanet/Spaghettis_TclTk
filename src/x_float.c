
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *float_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _floatobject {
    t_object    x_obj;
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_floatobject;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void float_bang (t_floatobject *x)
{
    outlet_float (x->x_outlet, x->x_f);
}

static void float_float (t_floatobject *x, t_float f)
{
    x->x_f = f; float_bang (x);
}

static void float_send (t_floatobject *x, t_symbol *s)
{
    if (pd_isThing (s)) { pd_float (pd_getThing (s), x->x_f); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Called by the t_floatmethod of the object maker class. */

static void *float_newBySlot (t_pd *dummy, t_float f)
{
    t_floatobject *x = (t_floatobject *)pd_new (float_class);
    
    x->x_f = f;
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    inlet_newFloat (cast_object (x), &x->x_f);
    
    pd_newest = cast_pd (x);
    
    return x;
}

static void *float_newByRegular (t_float f)
{
    return float_newBySlot (NULL, f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void float_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (&s_float,
            (t_newmethod)float_newBySlot,
            NULL,
            sizeof (t_floatobject),
            CLASS_DEFAULT,
            A_FLOAT,
            A_NULL);
            
    class_addCreator ((t_newmethod)float_newByRegular, sym_f, A_DEFFLOAT, A_NULL);
    
    class_addBang (c, (t_method)float_bang);
    class_addFloat (c, (t_method)float_float);
    
    class_addMethod (c, (t_method)float_send, sym_send, A_SYMBOL, A_NULL);

    float_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
