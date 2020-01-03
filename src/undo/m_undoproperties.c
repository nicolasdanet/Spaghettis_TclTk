
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *undoproperties_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undoproperties {
    t_undoaction    x_undo;                 /* Must be the first. */
    t_undosnippet   *x_s1;
    t_undosnippet   *x_s2;
    } t_undoproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoproperties_undo (t_undoproperties *z, t_symbol *s, int argc, t_atom *argv)
{
    undosnippet_message (z->x_s1);
}

void undoproperties_redo (t_undoproperties *z, t_symbol *s, int argc, t_atom *argv)
{
    undosnippet_message (z->x_s2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undoproperties_new (t_gobj *o, t_undosnippet *s1, t_undosnippet *s2)
{
    t_undoaction *x = (t_undoaction *)pd_new (undoproperties_class);
    t_undoproperties *z = (t_undoproperties *)x;
    
    /* Changing garray's size in property window requires to rebuild the graph. */
    /* Assume it is rarely the case, optimistically set the undo action as safe. */
    /* Note that the rebuild will be properly handled in the resizing function anyway. */
    
    int safe = 1;
    
    x->ua_id    = gobj_getUnique (o);
    x->ua_type  = UNDO_PROPERTIES;
    x->ua_safe  = safe;
    x->ua_label = sym_properties;
    
    PD_ASSERT (s1);
    PD_ASSERT (s2);
    
    z->x_s1 = s1;
    z->x_s2 = s2;
    
    return x;
}

static void undoproperties_free (t_undoproperties *z)
{
    undosnippet_free (z->x_s1);
    undosnippet_free (z->x_s2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoproperties_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undoproperties,
            NULL,
            (t_method)undoproperties_free,
            sizeof (t_undoproperties),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undoproperties_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undoproperties_redo, sym_redo, A_GIMME, A_NULL);
    
    undoproperties_class = c;
}

void undoproperties_destroy (void)
{
    class_free (undoproperties_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
