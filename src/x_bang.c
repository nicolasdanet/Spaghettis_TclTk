
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *bang_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _bang {
    t_object    x_obj;
    t_outlet    *x_outlet;
    } t_bang;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void bang_bang (t_bang *x)
{
    outlet_bang (x->x_outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Called by the t_bangmethod of the object maker class. */

static void *bang_newBySlot (t_pd *dummy)
{
    t_bang *x = (t_bang *)pd_new (bang_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_bang);
    
    instance_setNewestObject (cast_pd (x));
    
    return x;
}

static void *bang_newByRegular (void)
{
    return bang_newBySlot (NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bang_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (&s_bang,
            (t_newmethod)bang_newBySlot,
            NULL,
            sizeof (t_bang),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addCreator ((t_newmethod)bang_newByRegular, sym_b, A_NULL);
    
    class_addBang (c, (t_method)bang_bang);
    class_addFloat (c, (t_method)bang_bang);
    class_addSymbol (c, (t_method)bang_bang);
    class_addList (c, (t_method)bang_bang);
    class_addAnything (c, (t_method)bang_bang);
    
    bang_class = c;
}

void bang_destroy (void)
{
    class_free (bang_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
