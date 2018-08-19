
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *undodelete_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undodelete {
    t_undoaction    x_undo;             /* Must be the first. */
    t_undosnippet   *x_snippet;
    } t_undodelete;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undodelete_undo (t_undodelete *z, t_symbol *s, int argc, t_atom *argv)
{
    undosnippet_load (z->x_snippet); undosnippet_z (z->x_snippet);
}

void undodelete_redo (t_undodelete *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    
    glist_objectRemoveByUnique (undoaction_getUnique (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undodelete_new (t_gobj *o, t_undosnippet *snippet)
{
    t_undoaction *x = (t_undoaction *)pd_new (undodelete_class);
    t_undodelete *z = (t_undodelete *)x;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_DELETE;
    x->ua_label = sym_delete;
    
    PD_ASSERT (snippet);
    
    z->x_snippet = snippet;
    
    return x;
}

static void undodelete_free (t_undodelete *z)
{
    undosnippet_free (z->x_snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undodelete_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undodelete,
            NULL,
            (t_method)undodelete_free,
            sizeof (t_undodelete),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undodelete_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undodelete_redo, sym_redo, A_GIMME, A_NULL);
    
    undodelete_class = c;
}

void undodelete_destroy (void)
{
    class_free (undodelete_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
