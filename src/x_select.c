
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
    t_atomtype          x_type;
    t_int               x_size;
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
    int k = 0;
    
    if (x->x_type == A_FLOAT) {
    //
    int i;
    
    for (i = x->x_size - 1; i >= 0; i--) {
        if (GET_FLOAT (&x->x_vector[i].ao_atom) == f) {
            outlet_bang (x->x_vector[i].ao_outlet);
            k = 1; 
        }
    }
    //
    }
    
    if (!k) { outlet_float (x->x_outlet, f); }
}

static void select2_symbol (t_select2 *x, t_symbol *s)
{
    int k = 0;
    
    if (x->x_type == A_SYMBOL) {
    //
    int i;
    
    for (i = x->x_size - 1; i >= 0; i--) {
        if (GET_SYMBOL (&x->x_vector[i].ao_atom) == s) {
            outlet_bang (x->x_vector[i].ao_outlet);
            k = 1; 
        }
    }
    //
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
    x->x_type   = atom_getType (argv);
        
    for (i = 0; i < argc; i++) {
    //
    x->x_vector[i].ao_outlet = outlet_new (cast_object (x), &s_bang);
    
    if (x->x_type == A_FLOAT) { SET_FLOAT (&x->x_vector[i].ao_atom, atom_getFloatAtIndex (i, argc, argv)); }
    else {
        SET_SYMBOL (&x->x_vector[i].ao_atom, atom_getSymbolAtIndex (i, argc, argv));
    }
    //
    }
    
    x->x_outlet = outlet_new (cast_object (x), &s_anything);

    return x;
}

static void select2_free (t_select2 *x)
{
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *select_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 0) { t_atom a; SET_FLOAT (&a, 0.0); return select1_new (1, &a); }
    if (argc == 1) { return select1_new (argv, argv); }

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
    
    class_addFloat (select1_class,  select1_float);
    class_addFloat (select2_class,  select2_float);
    class_addSymbol (select1_class, select1_symbol);
    class_addSymbol (select2_class, select2_symbol);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
