
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

static t_class *undoresizebox_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undoresizebox {
    t_undoaction    x_undo;                 /* Must be the first. */
    int             x_old;
    int             x_new;
    } t_undoresizebox;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizebox_collapse (t_undoaction *kept, t_undoaction *deleted)
{
    t_undoresizebox *a = (t_undoresizebox *)kept;
    t_undoresizebox *b = (t_undoresizebox *)deleted;
    
    PD_ASSERT (pd_class (kept)    == undoresizebox_class);
    PD_ASSERT (pd_class (deleted) == undoresizebox_class);
    
    a->x_old = b->x_old;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizebox_undo (t_undoresizebox *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    
    glist_resizeBoxByUnique (undoaction_getUnique (x), z->x_old);
}

void undoresizebox_redo (t_undoresizebox *z, t_symbol *s, int argc, t_atom *argv)
{
    t_undoaction *x = (t_undoaction *)z;
    
    glist_resizeBoxByUnique (undoaction_getUnique (x), z->x_new);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undoresizebox_new (t_gobj *o, int m, int n)
{
    t_undoaction *x = (t_undoaction *)pd_new (undoresizebox_class);
    t_undoresizebox *z = (t_undoresizebox *)x;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_RESIZE_BOX;
    x->ua_label = sym_resize;
    
    z->x_old    = m;
    z->x_new    = n;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoresizebox_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undoresizebox,
            NULL,
            NULL,
            sizeof (t_undoresizebox),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undoresizebox_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undoresizebox_redo, sym_redo, A_GIMME, A_NULL);
    
    undoresizebox_class = c;
}

void undoresizebox_destroy (void)
{
    class_free (undoresizebox_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
