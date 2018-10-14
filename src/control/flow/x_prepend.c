
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../list/x_list.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *prepend_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _prepend {
    t_object        x_obj;          /* Must be the first. */
    t_listinlet     x_listinlet;
    t_outlet        *x_outlet;
    } t_prepend;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void prepend_proceed (t_prepend *x, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    int n = listinlet_getSize (&x->x_listinlet) + argc;
    
    if (n == 0) { outlet_bang (x->x_outlet); }
    else {
    //
    t_listinlet cache; listinlet_init (&cache);
    
    PD_ATOMS_ALLOCA (t, n);
    
    atom_copyAtoms (argv, argc, t + listinlet_getSize (&x->x_listinlet), argc);
    
    if (!listinlet_hasPointer (&x->x_listinlet)) { listinlet_copyAtomsUnchecked (&x->x_listinlet, t); }
    else {
        listinlet_clone (&x->x_listinlet, &cache); listinlet_copyAtomsUnchecked (&cache, t);
    }
    
    if (n == 1) {
        if (IS_SYMBOL (t)) { outlet_symbol (x->x_outlet, GET_SYMBOL (t)); }
        else if (IS_POINTER (t)) { outlet_pointer (x->x_outlet, GET_POINTER (t)); }
        else {
            outlet_float (x->x_outlet, atom_getFloat (t));
        }
    } else {
        if (IS_SYMBOL (t)) { outlet_anything (x->x_outlet, GET_SYMBOL (t), n - 1, t + 1); }
        else {
            outlet_list (x->x_outlet, n, t);
        }
    }
    
    PD_ATOMS_FREEA (t, n);
    
    listinlet_clear (&cache);
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
    t_atom a; SET_SYMBOL (&a, s); prepend_proceed (x, 1, &a);
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

static t_buffer *prepend_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_prepend *x = (t_prepend *)z;
    t_buffer *b  = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    listinlet_listGet (&x->x_listinlet, b);
    buffer_invalidatePointers (b);
    
    return b;
    //
    }
    
    return NULL;
}

static void prepend_restore (t_prepend *x, t_symbol *s, int argc, t_atom *argv)
{
    listinlet_listSet (&x->x_listinlet, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *prepend_new (t_symbol *s, int argc, t_atom *argv)
{
    t_prepend *x = (t_prepend *)pd_new (prepend_class);

    listinlet_init (&x->x_listinlet);
    listinlet_listSet (&x->x_listinlet, argc, argv);
    
    x->x_outlet = outlet_newAnything (cast_object (x));
    
    inlet_new (cast_object (x), cast_pd (&x->x_listinlet), NULL, NULL);

    return x;
}

static void prepend_free (t_prepend *x)
{
    listinlet_clear (&x->x_listinlet);
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

    class_addMethod (c, (t_method)prepend_restore, sym__restore, A_GIMME, A_NULL);

    class_setDataFunction (c, prepend_functionData);
    
    prepend_class = c;
}

void prepend_destroy (void)
{
    class_free (prepend_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
