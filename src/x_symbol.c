
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *symbol_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _symbolobject {
    t_object    x_obj;
    t_symbol    *x_s;
    t_outlet    *x_outlet;
    } t_symbolobject;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void symbol_bang (t_symbolobject *x)
{
    outlet_symbol (x->x_outlet, x->x_s);
}

static void symbol_symbol (t_symbolobject *x, t_symbol *s)
{
    x->x_s = s; symbol_bang (x);
}

static void symbol_anything (t_symbolobject *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_s = s; symbol_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Called by the t_symbolmethod of the object maker class. */

static void *symbol_new (t_pd *dummy, t_symbol *s)
{
    t_symbolobject *x = (t_symbolobject *)pd_new (symbol_class);
    
    x->x_s = s;
    x->x_outlet = outlet_new (cast_object (x), &s_symbol);
    
    inlet_newSymbol (cast_object (x), &x->x_s);
    
    instance_setNewestObject (cast_pd (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void symbol_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (&s_symbol,
            (t_newmethod)symbol_new,
            NULL,
            sizeof (t_symbolobject),
            CLASS_DEFAULT,
            A_SYMBOL,
            A_NULL);
            
    class_addBang (c, (t_method)symbol_bang);
    class_addSymbol (c, (t_method)symbol_symbol);
    class_addAnything (c, (t_method)symbol_anything);
    
    symbol_class = c;
}

void symbol_destroy (void)
{
    class_free (symbol_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
