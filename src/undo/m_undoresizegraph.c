
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
    int             x_deltaX;
    int             x_deltaY;
    } t_undoresizegraph;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizegraph_collapse (t_undoaction *kept, t_undoaction *deleted)
{
    t_undoresizegraph *a = (t_undoresizegraph *)kept;
    t_undoresizegraph *b = (t_undoresizegraph *)deleted;
    
    PD_ASSERT (pd_class (kept)    == undoresizegraph_class);
    PD_ASSERT (pd_class (deleted) == undoresizegraph_class);
    
    a->x_deltaX += b->x_deltaX;
    a->x_deltaY += b->x_deltaY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizegraph_undo (t_undoresizegraph *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    int m = -(z->x_deltaX);
    int n = -(z->x_deltaY);
    
    glist_resizeGraphByUnique (undoaction_getUnique (x), m, n);
}

void undoresizegraph_redo (t_undoresizegraph *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    int m = z->x_deltaX;
    int n = z->x_deltaY;
    
    glist_resizeGraphByUnique (undoaction_getUnique (x), m, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undoresizegraph_new (t_gobj *o, int deltaX, int deltaY)
{
    t_undoaction *x = (t_undoaction *)pd_new (undoresizegraph_class);
    t_undoresizegraph *z = (t_undoresizegraph *)x;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_RESIZE_GRAPH;
    x->ua_label = sym_resize;
    
    z->x_deltaX = deltaX;
    z->x_deltaY = deltaY;
    
    return x;
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
