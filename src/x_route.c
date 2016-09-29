
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *route_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _route {
    t_object        x_obj;              /* Must be the first. */
    t_atomtype      x_type;
    t_int           x_size;
    t_atomoutlet    *x_vector;
    t_outlet        *x_outlet;
    } t_route;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int route_listForFloat (t_route *x, int argc, t_atom *argv)
{
    int k = 0;
    
    if (argc && IS_FLOAT (argv)) {
    //
    int i;
    
    for (i = x->x_size - 1; i >= 0; i--) {
    
        if (GET_FLOAT (&x->x_vector[i].ao_atom) == atom_getFloat (argv)) {
            if (argc > 1 && IS_SYMBOL (argv + 1)) {
                outlet_anything (x->x_vector[i].ao_outlet, GET_SYMBOL (argv + 1), argc - 2, argv + 2);
            } else { 
                outlet_list (x->x_vector[i].ao_outlet, argc - 1, argv + 1);
            }
            k = 1;
        }
    }
    //
    }
    
    return k;
}

static int route_listForSymbol (t_route *x, int argc, t_atom *argv)
{
    int i, k = 0;
    
    if (argc > 1) {
        for (i = x->x_size - 1; i >= 0; i--) {
        
            if (GET_SYMBOL (&x->x_vector[i].ao_atom) == &s_list) {
                if (argc > 0 && IS_SYMBOL (argv)) {
                    outlet_anything (x->x_vector[i].ao_outlet, GET_SYMBOL (argv), argc - 1, argv + 1);
                } else {
                    outlet_list (x->x_vector[i].ao_outlet, argc, argv);
                }
                k = 1;
            }
        }
        
    } else if (argc == 0) {
        for (i = x->x_size - 1; i >= 0; i--) {
        
            if (GET_SYMBOL (&x->x_vector[i].ao_atom) == &s_bang) {
                outlet_bang (x->x_vector[i].ao_outlet); 
                k = 1;
            }
        }
        
    } else if (IS_FLOAT (argv)) {
        for (i = x->x_size - 1; i >= 0; i--) {

            if (GET_SYMBOL (&x->x_vector[i].ao_atom) == &s_float) {
                outlet_float (x->x_vector[i].ao_outlet, GET_FLOAT (argv));
                k = 1;
            }
        }
        
    } else if (IS_SYMBOL (argv)) {
        for (i = x->x_size - 1; i >= 0; i--) {
        
            if (GET_SYMBOL (&x->x_vector[i].ao_atom) == &s_symbol) {
                outlet_symbol (x->x_vector[i].ao_outlet, GET_SYMBOL (argv));
                k = 1;
            }
        }
    }
    
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void route_list (t_route *x, t_symbol *dummy, int argc, t_atom *argv)
{
    int k = 0;
    
    if (x->x_type == A_FLOAT) { k = route_listForFloat (x, argc, argv); }
    else {
        k = route_listForSymbol (x, argc, argv);
    }

    if (!k) { outlet_list (x->x_outlet, argc, argv); }
}

static void route_anything (t_route *x, t_symbol *s, int argc, t_atom *argv)
{
    int i, k = 0;
    
    if (x->x_type == A_SYMBOL) {
        for (i = x->x_size - 1; i >= 0; i--) {
        
            if (GET_SYMBOL (&x->x_vector[i].ao_atom) == s) {
                if (!argc || !IS_SYMBOL (argv)) { outlet_list (x->x_vector[i].ao_outlet, argc, argv); }
                else {
                    outlet_anything (x->x_vector[i].ao_outlet, GET_SYMBOL (argv), argc - 1, argv + 1);
                } 
                k = 1;
            }
        }
    }
    
    if (!k) { outlet_anything (x->x_outlet, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *route_newPerform (int argc, t_atom *argv)
{
    t_route *x = (t_route *)pd_new (route_class);
    int i;
    
    x->x_type   = atom_getType (argv);
    x->x_size   = argc;
    x->x_vector = (t_atomoutlet *)PD_MEMORY_GET (x->x_size * sizeof (t_atomoutlet));
    
    for (i = 0; i < argc; i++) {
    //
    x->x_vector[i].ao_outlet = outlet_new (cast_object (x), &s_anything);
    
    if (x->x_type == A_FLOAT) { SET_FLOAT (&x->x_vector[i].ao_atom, atom_getFloatAtIndex (i, argc, argv)); } 
    else {
        SET_SYMBOL (&x->x_vector[i].ao_atom, atom_getSymbolAtIndex (i, argc, argv));
    }
    //
    }
    
    if (argc == 1) {
    //
    if (x->x_type == A_FLOAT) { inlet_newFloat (cast_object (x), ADDRESS_FLOAT (&x->x_vector[0].ao_atom)); }
    else {
        inlet_newSymbol (cast_object (x), ADDRESS_SYMBOL (&x->x_vector[0].ao_atom));
    }
    //
    }
    
    x->x_outlet = outlet_new (cast_object (x), &s_anything);
    
    return x;
}

static void *route_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 0) { t_atom a; SET_FLOAT (&a, 0.0); return route_newPerform (1, &a); }
    else {
        return route_newPerform (argc, argv);
    }
}

static void route_free (t_route *x)
{
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void route_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_route,
            (t_newmethod)route_new,
            (t_method)route_free,
            sizeof (t_route),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, route_list);
    class_addAnything (c, route_anything);
    
    route_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
