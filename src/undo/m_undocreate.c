
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

static t_class *undocreate_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undocreate {
    t_undoaction    x_undo;             /* Must be the first. */
    t_undosnippet   *x_snippet;
    } t_undocreate;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undocreate_undo (t_undocreate *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    
    glist_objectRemoveByUnique (undoaction_getUnique (x));
}

void undocreate_redo (t_undocreate *z, t_symbol *s, int argc, t_atom *argv)
{
    undosnippet_load (z->x_snippet); undosnippet_z (z->x_snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Take ownership of snippet. */

t_undoaction *undocreate_new (t_gobj *o, t_undosnippet *snippet)
{
    t_undoaction *x = (t_undoaction *)pd_new (undocreate_class);
    t_undocreate *z = (t_undocreate *)x;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_CREATE;
    x->ua_label = sym_create;
    
    PD_ASSERT (snippet);
    
    z->x_snippet = snippet;

    return x;
}

static void undocreate_free (t_undocreate *z)
{
    undosnippet_free (z->x_snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undocreate_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undocreate,
            NULL,
            (t_method)undocreate_free,
            sizeof (t_undocreate),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undocreate_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undocreate_redo, sym_redo, A_GIMME, A_NULL);
    
    undocreate_class = c;
}

void undocreate_destroy (void)
{
    class_free (undocreate_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
