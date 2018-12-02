
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

static t_class *undotyping_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undotyping {
    t_undoaction    x_undo;             /* Must be the first. */
    t_undosnippet   *x_s1;
    t_undosnippet   *x_s2;
    } t_undotyping;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undotyping_undo (t_undotyping *z, t_symbol *s, int argc, t_atom *argv)
{
    undosnippet_paste (z->x_s1);
}

void undotyping_redo (t_undotyping *z, t_symbol *s, int argc, t_atom *argv)
{
    undosnippet_paste (z->x_s2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undotyping_new (t_gobj *o, t_undosnippet *s1, t_undosnippet *s2)
{
    t_undoaction *x = (t_undoaction *)pd_new (undotyping_class);
    t_undotyping *z = (t_undotyping *)x;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_TYPING;
    x->ua_label = sym_typing;
    
    PD_ASSERT (s1);
    PD_ASSERT (s2);
    
    z->x_s1 = s1;
    z->x_s2 = s2;
    
    return x;
}

static void undotyping_free (t_undotyping *z)
{
    undosnippet_free (z->x_s1);
    undosnippet_free (z->x_s2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undotyping_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undotyping,
            NULL,
            (t_method)undotyping_free,
            sizeof (t_undotyping),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undotyping_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undotyping_redo, sym_redo, A_GIMME, A_NULL);
    
    undotyping_class = c;
}

void undotyping_destroy (void)
{
    class_free (undotyping_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
