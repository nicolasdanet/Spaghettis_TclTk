
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *swap_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _swap {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f1;
    t_float     x_f2;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_swap;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void swap_bang (t_swap *x)
{
    outlet_float (x->x_outletRight, x->x_f1);
    outlet_float (x->x_outletLeft,  x->x_f2);
}

static void swap_float (t_swap *x, t_float f)
{
    x->x_f1 = f; swap_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *swap_new (t_float f)
{
    t_swap *x = (t_swap *)pd_new (swap_class);
    
    x->x_f1 = 0;
    x->x_f2 = f;
    x->x_outletLeft  = outlet_newFloat (cast_object (x));
    x->x_outletRight = outlet_newFloat (cast_object (x));
    
    inlet_newFloat (cast_object (x), &x->x_f2);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void swap_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_swap,
            (t_newmethod)swap_new,
            NULL,
            sizeof (t_swap),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);

    class_addCreator ((t_newmethod)swap_new, sym_fswap, A_DEFFLOAT, A_NULL);
    
    class_addBang (c, (t_method)swap_bang);
    class_addFloat (c, (t_method)swap_float);
    
    swap_class = c;
}

void swap_destroy (void)
{
    class_free (swap_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
