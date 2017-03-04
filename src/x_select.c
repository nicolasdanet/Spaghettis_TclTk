
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *select1_class;                  /* Shared. */
static t_class *select2_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _select1 {
    t_object            x_obj;                  /* Must be the first. */
    t_atom              x_atom;
    t_outlet            *x_outletLeft;
    t_outlet            *x_outletRight;
    } t_select1;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _select2 {
    t_object            x_obj;                  /* Must be the first. */
    int                 x_size;
    t_atomoutlet        *x_vector;
    t_outlet            *x_outlet;
    } t_select2;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void select1_float (t_select1 *x, t_float f)
{
    if (IS_FLOAT (&x->x_atom) && (f == GET_FLOAT (&x->x_atom))) { outlet_bang (x->x_outletLeft); }
    else {
        outlet_float (x->x_outletRight, f);
    }
}

static void select1_symbol (t_select1 *x, t_symbol *s)
{
    if (IS_SYMBOL (&x->x_atom) && (s == GET_SYMBOL (&x->x_atom))) { outlet_bang (x->x_outletLeft); }
    else { 
        outlet_symbol (x->x_outletRight, s);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void select2_float (t_select2 *x, t_float f)
{
    int i, k = 0;
    
    for (i = x->x_size - 1; i >= 0; i--) {

        t_atom a; SET_FLOAT (&a, f); 
        
        if (atomoutlet_isEqualTo (x->x_vector + i, &a)) {
            k |= 1; outlet_bang (atomoutlet_getOutlet (x->x_vector + i));
        }
    }
    
    if (!k) { outlet_float (x->x_outlet, f); }
}

static void select2_symbol (t_select2 *x, t_symbol *s)
{
    int i, k = 0;
    
    for (i = x->x_size - 1; i >= 0; i--) {

        t_atom a; SET_SYMBOL (&a, s); 
        
        if (atomoutlet_isEqualTo (x->x_vector + i, &a)) {
            k |= 1; outlet_bang (atomoutlet_getOutlet (x->x_vector + i));
        }
    }
    
    if (!k) { outlet_symbol (x->x_outlet, s); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *select1_new (int argc, t_atom *argv)
{
    t_select1 *x = (t_select1 *)pd_new (select1_class);
    
    x->x_atom = *argv;
    x->x_outletLeft = outlet_new (cast_object (x), &s_bang);
    
    if (IS_FLOAT (argv)) { x->x_outletRight = outlet_new (cast_object (x), &s_float); }
    else {
        x->x_outletRight = outlet_new (cast_object (x), &s_symbol);
    }
    
    if (IS_FLOAT (argv)) { inlet_newFloat (cast_object (x), ADDRESS_FLOAT (&x->x_atom)); } 
    else {
        inlet_newSymbol (cast_object (x), ADDRESS_SYMBOL (&x->x_atom));
    }
    
    return x;
}

static void *select2_new (int argc, t_atom *argv)
{
    t_select2 *x = (t_select2 *)pd_new (select2_class);
    int i;
        
    x->x_size   = argc;
    x->x_vector = (t_atomoutlet *)PD_MEMORY_GET (x->x_size * sizeof (t_atomoutlet));

    for (i = 0; i < argc; i++) {
        atomoutlet_makeTypedOutlet (x->x_vector + i, cast_object (x), &s_bang, argv + i, 0);
    }
    
    x->x_outlet = outlet_new (cast_object (x), &s_anything);

    return x;
}

static void select2_free (t_select2 *x)
{
    int i;
    
    for (i = 0; i < x->x_size; i++) { atomoutlet_release (x->x_vector + i); }
    
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *select_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 0) { t_atom a; SET_FLOAT (&a, (t_float)0.0); return select1_new (1, &a); }
    if (argc == 1) { return select1_new (argc, argv); }

    return select2_new (argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void select_setup (void)
{
    select1_class = class_new (sym_select,
                        NULL,
                        NULL,
                        sizeof (t_select1),
                        CLASS_DEFAULT,
                        A_NULL);
    
    select2_class = class_new (sym_select,
                        NULL,
                        (t_method)select2_free,
                        sizeof (t_select2),
                        CLASS_DEFAULT,
                        A_NULL);
    
    class_addCreator ((t_newmethod)select_new, sym_select,  A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)select_new, sym_sel,     A_GIMME, A_NULL);
    
    class_addFloat (select1_class,  (t_method)select1_float);
    class_addFloat (select2_class,  (t_method)select2_float);
    class_addSymbol (select1_class, (t_method)select1_symbol);
    class_addSymbol (select2_class, (t_method)select2_symbol);
}

void select_destroy (void)
{
    CLASS_FREE (select1_class);
    CLASS_FREE (select2_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
