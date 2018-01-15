
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *midirealtimein_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _midirealtimein {
    t_object    x_obj;                          /* Must be the first. */
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_midirealtimein;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void midirealtimein_list (t_midirealtimein *x, t_symbol *s, int argc, t_atom *argv)
{
    int byte = (int)atom_getFloatAtIndex (0, argc, argv);
    int port = (int)atom_getFloatAtIndex (1, argc, argv);
    
    outlet_float (x->x_outletRight, (t_float)port);
    outlet_float (x->x_outletLeft,  (t_float)byte);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *midirealtimein_new (void)
{
    t_midirealtimein *x = (t_midirealtimein *)pd_new (midirealtimein_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_float);
    
    pd_bind (cast_pd (x), sym__midirealtimein);
    
    return x;
}

static void midirealtimein_free (t_midirealtimein *x)
{
    pd_unbind (cast_pd (x), sym__midirealtimein);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midirealtimein_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_midirealtimein, 
            (t_newmethod)midirealtimein_new,
            (t_method)midirealtimein_free, 
            sizeof (t_midirealtimein),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFFLOAT,
            A_NULL);
            
    class_addList (c, (t_method)midirealtimein_list);
    
    midirealtimein_class = c;
}

void midirealtimein_destroy (void)
{
    class_free (midirealtimein_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
