
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

static t_class *bag_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _bagelement {
    t_float             e_value;
    struct _bagelement  *e_next;
    } t_bagelement;

typedef struct _bag {
    t_object            x_obj;              /* Must be the first. */
    t_float             x_velocity;
    t_bagelement        *x_elements;
    t_outlet            *x_outlet;
    } t_bag;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bag_removeAll (t_bag *x, int dump)
{
    t_bagelement *e = NULL;
    
    while (e = x->x_elements) {
    //
    if (dump) { outlet_float (x->x_outlet, e->e_value); }
    x->x_elements = e->e_next; PD_MEMORY_FREE (e);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bag_float(t_bag *x, t_float f)
{
    t_bagelement *bagelement, *e2, *e3;
    if (x->x_velocity != 0)
    {
        bagelement = (t_bagelement *)PD_MEMORY_GET(sizeof *bagelement);
        bagelement->e_next = 0;
        bagelement->e_value = f;
        if (!x->x_elements) x->x_elements = bagelement;
        else    /* LATER replace with a faster algorithm */
        {
            for (e2 = x->x_elements; e3 = e2->e_next; e2 = e3)
                ;
            e2->e_next = bagelement;
        }
    }
    else
    {
        if (!x->x_elements) return;
        if (x->x_elements->e_value == f)
        {
            bagelement = x->x_elements;
            x->x_elements = x->x_elements->e_next;
            PD_MEMORY_FREE(bagelement);
            return;
        }
        for (e2 = x->x_elements; e3 = e2->e_next; e2 = e3)
            if (e3->e_value == f)
        {
            e2->e_next = e3->e_next;
            PD_MEMORY_FREE(e3);
            return;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bag_flush (t_bag *x)
{
    bag_removeAll (x, 1);
}

static void bag_clear (t_bag *x)
{
    bag_removeAll (x, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *bag_new (void)
{
    t_bag *x = (t_bag *)pd_new (bag_class);
    
    x->x_velocity = 0;
    x->x_elements = NULL;
    x->x_outlet   = outlet_new (cast_object (x), &s_float);
    
    inlet_newFloat (cast_object (x), &x->x_velocity);
    
    return x;
}

static void bag_free (t_bag *x)
{                                                                   
    bag_clear (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void bag_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_bag, 
            (t_newmethod)bag_new,
            (t_method)bag_free,
            sizeof (t_bag),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addFloat (c, bag_float);
    
    class_addMethod (c, (t_method)bag_flush,    sym_flush,  A_NULL);
    class_addMethod (c, (t_method)bag_clear,    sym_clear,  A_NULL);
    
    bag_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
