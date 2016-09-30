
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *pack_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pack {
    t_object    x_obj;              /* Must be the first. */
    t_int       x_size;
    t_atom      *x_atoms;
    t_gpointer  *x_pointers;
    t_outlet    *x_outlet;
    } t_pack;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pack_bang (t_pack *x)
{
    outlet_list (x->x_outlet, x->x_size, x->x_atoms);
}

static void pack_float (t_pack *x, t_float f)
{
    if (IS_FLOAT (x->x_atoms)) { SET_FLOAT (x->x_atoms, f); }
    else {
        warning_badType (sym_pack, sym_float);
    }
    
    pack_bang (x);
}

static void pack_symbol (t_pack *x, t_symbol *s)
{
    if (IS_SYMBOL (x->x_atoms)) { SET_SYMBOL (x->x_atoms, s); }
    else {
        warning_badType (sym_pack, sym_symbol);
    }
    
    pack_bang (x);
}

static void pack_pointer (t_pack *x, t_gpointer *gp)
{
    if (IS_POINTER (x->x_atoms)) { gpointer_setByCopy (gp, x->x_pointers); }
    else {
        warning_badType (sym_pack, sym_pointer);
    }
    
    pack_bang (x);
}

static void pack_list (t_pack *x, t_symbol *s, int argc, t_atom *argv)
{
    object_list (cast_object (x), argc, argv);
}

static void pack_anything (t_pack *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_object (x), pack_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *pack_newPerform (int argc, t_atom *argv)
{
    t_pack *x = (t_pack *)pd_new (pack_class);
    int i;
    
    x->x_size     = argc;
    x->x_atoms    = (t_atom *)PD_MEMORY_GET (x->x_size * sizeof (t_atom));
    x->x_pointers = (t_gpointer *)PD_MEMORY_GET (x->x_size * sizeof (t_gpointer));

    for (i = 0; i < x->x_size; i++) {
    //
    gpointer_init (x->x_pointers + i); SET_FLOAT (x->x_atoms + i, 0.0);
    //
    }

    for (i = 0; i < x->x_size; i++) {
    //
    t_atom *a = argv + i; t_symbol *t = atom_getSymbol (a);
    
    if (t == sym_s) {
        SET_SYMBOL (x->x_atoms + i, &s_symbol);
        if (i) { inlet_newSymbol (cast_object (x), ADDRESS_SYMBOL (x->x_atoms + i)); }
        
    } else if (t == sym_p) {
        SET_POINTER (x->x_atoms + i, x->x_pointers + i);
        if (i) { inlet_newPointer (cast_object (x), x->x_pointers + i); }
        
    } else {
        SET_FLOAT (x->x_atoms + i, atom_getFloat (a));
        if (i) { inlet_newFloat (cast_object (x), ADDRESS_FLOAT (x->x_atoms + i)); }
        if (!IS_FLOAT (a) && t != sym_f) { warning_badType (sym_pack, t); }
    }
    //
    }
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    return x;
}

static void *pack_new (t_symbol *s, int argc, t_atom *argv)
{
    if (argc) { return pack_newPerform (argc, argv); }
    else {
        t_atom a[2]; SET_FLOAT (&a[0], 0.0); SET_FLOAT (&a[1], 0.0); return pack_newPerform (2, &a);
    }
}

static void pack_free (t_pack *x)
{
    int i;
    
    for (i = 0; i < x->x_size; i++) { gpointer_unset (x->x_pointers + i); }
    
    PD_MEMORY_FREE (x->x_pointers);
    PD_MEMORY_FREE (x->x_atoms);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pack_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pack,
            (t_newmethod)pack_new,
            (t_method)pack_free,
            sizeof (t_pack),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, pack_bang);
    class_addFloat (c, pack_float);
    class_addSymbol (c, pack_symbol);
    class_addPointer (c, pack_pointer);
    class_addList (c, pack_list);
    class_addAnything (c, pack_anything);
    
    pack_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
