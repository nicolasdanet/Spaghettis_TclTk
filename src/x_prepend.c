
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_atomoutlet.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *prepend_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _prepend {
    t_object        x_obj;          /* Must be the first. */
    int             x_size;
    t_atomoutlet    *x_vector;
    t_outlet        *x_outlet;
    } t_prepend;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void prepend_proceed (t_prepend *x, int argc, t_atom *argv)
{
    int n = x->x_size + argc;
    
    if (n == 0) { outlet_bang (x->x_outlet); }
    else {
    //
    t_atom *a = NULL;
    int i;
        
    PD_ATOMS_ALLOCA (a, n);
    
    for (i = 0; i < x->x_size; i++) { atomoutlet_copyAtom (x->x_vector + i, a + i); }
    
    atom_copyAtoms (argv, argc, a + x->x_size, argc);
    
    if (n == 1) {
        if (IS_SYMBOL (a)) { outlet_symbol (x->x_outlet, GET_SYMBOL (a)); }
        else if (IS_POINTER (a)) { outlet_pointer (x->x_outlet, GET_POINTER (a)); }
        else {
            outlet_float (x->x_outlet, atom_getFloat (a));
        }
    } else {
        if (IS_SYMBOL (a)) { outlet_anything (x->x_outlet, GET_SYMBOL (a), n - 1, a + 1); }
        else {
            outlet_list (x->x_outlet, n, a);
        }
    }
    
    PD_ATOMS_FREEA (a, n);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void prepend_bang (t_prepend *x)
{
    prepend_proceed (x, 0, NULL);
}

static void prepend_float (t_prepend *x, t_float f)
{
    t_atom a;
    SET_FLOAT (&a, f);
    prepend_proceed (x, 1, &a);
}

static void prepend_symbol (t_prepend *x, t_symbol *s)
{
    t_atom a; SET_SYMBOL(&a, s); prepend_proceed (x, 1, &a);
}

static void prepend_pointer (t_prepend *x, t_gpointer *gp)
{
    t_atom a; SET_POINTER (&a, gp); prepend_proceed (x, 1, &a);
}

static void prepend_list (t_prepend *x, t_symbol *s, int argc, t_atom *argv)
{
    prepend_proceed (x, argc, argv);
}

static void prepend_anything (t_prepend *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)prepend_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void prepend_set (t_prepend *x, t_symbol *dummy, int argc, t_atom *argv)
{
    int i;
    
    if (x->x_vector) { PD_MEMORY_FREE (x->x_vector); }
    
    x->x_size   = argc;
    x->x_vector = (t_atomoutlet *)PD_MEMORY_GET (x->x_size * sizeof (t_atomoutlet));

    for (i = 0; i < x->x_size; i++) {
        atomoutlet_make (x->x_vector + i, cast_object (x), ATOMOUTLET_NONE, NULL, argv + i);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *prepend_new (t_symbol *s, int argc, t_atom *argv)
{
    t_prepend *x = (t_prepend *)pd_new (prepend_class);

    prepend_set (x, s, argc, argv);
    
    x->x_outlet = outlet_new (cast_object (x), &s_anything);
    
    inlet_new2 (x, sym_set);
    
    return x;
}

static void prepend_free (t_prepend *x)
{
    int i;
    
    for (i = 0; i < x->x_size; i++) { atomoutlet_release (x->x_vector + i); }
    
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void prepend_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_prepend,
            (t_newmethod)prepend_new,
            (t_method)prepend_free,
            sizeof (t_prepend),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)prepend_bang);
    class_addFloat (c, (t_method)prepend_float);
    class_addSymbol (c, (t_method)prepend_symbol);
    class_addPointer (c, (t_method)prepend_pointer);
    class_addList (c, (t_method)prepend_list);
    class_addAnything (c, (t_method)prepend_anything);
    
    class_addMethod (c, (t_method)prepend_set, sym__inlet2, A_GIMME, A_NULL);

    prepend_class = c;
}

void prepend_destroy (void)
{
    class_free (prepend_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
