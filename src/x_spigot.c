
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *spigot_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _spigot {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_state;
    t_outlet    *x_outlet;
    } t_spigot;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void spigot_bang (t_spigot *x)
{
    if (x->x_state) { outlet_bang (x->x_outlet); }
}

static void spigot_float (t_spigot *x, t_float f)
{
    if (x->x_state) { outlet_float (x->x_outlet, f); }
}

static void spigot_symbol (t_spigot *x, t_symbol *s)
{
    if (x->x_state) { outlet_symbol (x->x_outlet, s); }
}

static void spigot_pointer (t_spigot *x, t_gpointer *gp)
{
    if (x->x_state) { outlet_pointer (x->x_outlet, gp); }
}

static void spigot_list (t_spigot *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_state) { outlet_list (x->x_outlet, argc, argv); }
}

static void spigot_anything (t_spigot *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_state) { outlet_anything (x->x_outlet, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *spigot_new (t_float f)
{
    t_spigot *x = (t_spigot *)pd_new (spigot_class);
    
    x->x_state  = f;
    x->x_outlet = outlet_new (cast_object (x), &s_anything);
        
    inlet_newFloat (cast_object (x), &x->x_state);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void spigot_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_spigot,
            (t_newmethod)spigot_new,
            NULL,
            sizeof (t_spigot),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addBang (c, (t_method)spigot_bang);
    class_addFloat (c, (t_method)spigot_float);
    class_addSymbol (c, (t_method)spigot_symbol);
    class_addPointer (c, (t_method)spigot_pointer);
    class_addList (c, (t_method)spigot_list);
    class_addAnything (c, (t_method)spigot_anything);
    
    spigot_class = c;
}

void spigot_destroy (void)
{
    CLASS_FREE (spigot_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
