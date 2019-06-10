
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *undoresizegraph_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undoresizegraph {
    t_undoaction    x_undo;             /* Must be the first. */
    t_rectangle     x_old;
    t_rectangle     x_resized;
    } t_undoresizegraph;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizegraph_collapse (t_undoaction *newestKept, t_undoaction *olderDeleted)
{
    t_undoresizegraph *a = (t_undoresizegraph *)newestKept;
    t_undoresizegraph *b = (t_undoresizegraph *)olderDeleted;
    
    PD_ASSERT (pd_class (newestKept)   == undoresizegraph_class);
    PD_ASSERT (pd_class (olderDeleted) == undoresizegraph_class);
    
    rectangle_setCopy (&a->x_old, &b->x_old);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizegraph_undo (t_undoresizegraph *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    
    glist_setGraphByUnique (undoaction_getUnique (x), &z->x_old);
}

void undoresizegraph_redo (t_undoresizegraph *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    
    glist_setGraphByUnique (undoaction_getUnique (x), &z->x_resized);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undoresizegraph_proceed (t_gobj *o, t_rectangle *old, t_rectangle *resized, t_symbol *label)
{
    t_undoaction *x = (t_undoaction *)pd_new (undoresizegraph_class);
    t_undoresizegraph *z = (t_undoresizegraph *)x;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_RESIZE_GRAPH;
    x->ua_safe  = 1;
    x->ua_label = label;
    
    rectangle_setCopy (&z->x_old, old);
    rectangle_setCopy (&z->x_resized, resized);
    
    return x;
}

t_undoaction *undoresizegraph_new (t_gobj *o, t_rectangle *old, t_rectangle *resized)
{
    return undoresizegraph_proceed (o, old, resized, sym_resize);
}

t_undoaction *undomovegraph_new (t_gobj *o, t_rectangle *old, t_rectangle *moved)
{
    return undoresizegraph_proceed (o, old, moved, sym_move);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizegraph_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undoresizegraph,
            NULL,
            NULL,
            sizeof (t_undoresizegraph),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undoresizegraph_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undoresizegraph_redo, sym_redo, A_GIMME, A_NULL);

    undoresizegraph_class = c;
}

void undoresizegraph_destroy (void)
{
    class_free (undoresizegraph_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
