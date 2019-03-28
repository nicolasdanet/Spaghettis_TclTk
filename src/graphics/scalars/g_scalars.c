
/* Copyright (c) 1997-2019 Miller Puckette and others. */

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

#define SCALARS_TAG         0
#define SCALARS_COUNT       1
#define SCALARS_SEMICOLON   2
#define SCALARS_SIZE        3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *scalars_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _scalars {
    t_object    x_obj;                  /* Must be the first. */
    int         x_keep;
    t_symbol    *x_target;
    t_slots     *x_slots;
    t_glist     *x_owner;
    } t_scalars;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_clear (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_glist *scalars_fetch (t_scalars *x)
{
    if (x->x_target != &s_) {
    //
    t_glist *glist = cast_glist (symbol_getThingByClass (symbol_makeBindIfNot (x->x_target), canvas_class));
    
    if (glist) { return glist; }
    else {
        error_canNotFind (sym_scalars, x->x_target); return NULL;
    }
    //
    }
    
    return x->x_owner;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalars_save (t_scalars *x, t_symbol *s, int argc, t_atom *argv)
{
    t_glist *glist = scalars_fetch (x);
    
    if (glist) {
    //
    t_gobj *y   = NULL;
    t_buffer *b = buffer_new();
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_isScalar (y)) {
            t_buffer *t = buffer_new();
            scalar_serialize (cast_scalar (y), t);
            buffer_appendSymbol (b, sym_scalar);
            buffer_appendFloat (b, buffer_getSize (t));
            buffer_appendSemicolon (b);
            buffer_appendBuffer (b, t);
            buffer_free (t);
        }
    }
    
    slots_set (x->x_slots, argc ? argv : NULL, b);
    
    buffer_free (b);
    
    if (x->x_keep) { glist_setDirty (x->x_owner, 1); }
    //
    }
}

static void scalars_load (t_scalars *x, t_symbol *s, int argc, t_atom *argv)
{
    t_glist *glist = scalars_fetch (x);

    if (glist) {
    //
    t_buffer *b = buffer_new();
    t_error err = slots_get (x->x_slots, argc ? argv : NULL, b);
    
    if (!err) {
    //
    t_iterator *iter = iterator_new (buffer_getSize (b), buffer_getAtoms (b), 1);
    t_atom *atoms = NULL;
    int count;
    
    canvas_clear (glist);
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    if (count == SCALARS_SIZE) {
        if (atom_getSymbol (atoms + SCALARS_TAG) == sym_scalar) {
            int size = atom_getFloat (atoms + SCALARS_COUNT);
            PD_ASSERT (IS_SEMICOLON (atoms + SCALARS_SEMICOLON));
            if (size > 0 && (iterator_get (iter) + size <= buffer_getSize (b))) {
                glist_objectMakeScalar (glist, size, atoms + SCALARS_SIZE);
                iterator_skip (iter, size);
            }
        }
    }
    //
    }
    
    iterator_free (iter);
    //
    } else { error_canNotFind (sym_scalars, sym_slot); }
    
    buffer_free (b);
    //
    }
}

static void scalars_remove (t_scalars *x, t_symbol *s, int argc, t_atom *argv)
{
    if (slots_remove (x->x_slots, argc ? argv : NULL) == PD_ERROR_NONE) {
        if (x->x_keep) { glist_setDirty (x->x_owner, 1); }
    }
}

static void scalars_clear (t_scalars *x)
{
    if (!slots_isEmpty (x->x_slots)) {
        slots_clear (x->x_slots); if (x->x_keep) { glist_setDirty (x->x_owner, 1); }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalars_read (t_scalars *x, t_symbol *name)
{
    slots_clear (x->x_slots);
    
    if (buffer_fileRead (slots_getRaw (x->x_slots), name, x->x_owner)) { error_failsToRead (name); }
}

static void scalars_write (t_scalars *x, t_symbol *name)
{
    t_symbol *directory = environment_getDirectory (glist_getEnvironment (x->x_owner));
    
    if (slots_isEmpty (x->x_slots) || buffer_fileWrite (slots_getRaw (x->x_slots), name, directory)) {
        error_failsToWrite (name);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *scalars_functionData (t_gobj *z, int flags)
{
    t_scalars *x = (t_scalars *)z;
    
    if (SAVED_DEEP (flags) || x->x_keep) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_serialize (b, slots_getRaw (x->x_slots));
    
    return b;
    //
    }
    
    return NULL;
}

static void scalars_restore (t_scalars *x, t_symbol *s, int argc, t_atom *argv)
{
    t_scalars *old = (t_scalars *)instance_pendingFetch (cast_gobj (x));

    slots_clear (x->x_slots);
    
    if (!old) { buffer_deserialize (slots_getRaw (x->x_slots), argc, argv); }
    else {
    //
    t_slots *t = x->x_slots; x->x_slots = old->x_slots; old->x_slots = t;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *scalars_new (t_symbol *s, int argc, t_atom *argv)
{
    t_scalars *x = (t_scalars *)pd_new (scalars_class);
    
    x->x_keep   = 0;
    x->x_target = &s_;
    
    while (argc && IS_SYMBOL (argv)) {
    
        t_symbol *t = GET_SYMBOL (argv);
        
        if (t == sym___dash__keep) { x->x_keep = 1; argc--; argv++; }
        else {
            break;
        }
    }
    
    if (!error__options (s, argc, argv)) {
        if (argc && IS_SYMBOL (argv)) {
            x->x_target = GET_SYMBOL (argv);
            argc--; argv++;
        }
    }
    
    /* Dollar expansion is zero in abstraction opened as patch. */
    
    if (argc && IS_FLOAT (argv) && (GET_FLOAT (argv) == 0.0)) { argc--; argv++; }
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x->x_slots = slots_new();
    x->x_owner = instance_contextGetCurrent();
    
    return x;
}

static void scalars_free (t_scalars *x)
{
    slots_free (x->x_slots);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalars_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_scalars,
            (t_newmethod)scalars_new,
            (t_method)scalars_free,
            sizeof (t_scalars),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
   
    class_addMethod (c, (t_method)scalars_read,     sym_read,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)scalars_write,    sym_write,      A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)scalars_save,     sym_save,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)scalars_load,     sym_load,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)scalars_remove,   sym_remove,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)scalars_clear,    sym_clear,      A_NULL);
    class_addMethod (c, (t_method)scalars_restore,  sym__restore,   A_GIMME, A_NULL);
    
    class_setDataFunction (c, scalars_functionData);
    class_requirePending (c);

    scalars_class = c;
}

void scalars_destroy (void)
{
    class_free (scalars_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
