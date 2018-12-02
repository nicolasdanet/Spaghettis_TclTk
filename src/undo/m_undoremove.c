
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *undoremove_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undoremove {
    t_undoaction    x_undo;             /* Must be the first. */
    } t_undoremove;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undoremove_new (void)
{
    t_undoaction *x = (t_undoaction *)pd_new (undoremove_class);
    
    x->ua_type  = UNDO_REMOVE;
    x->ua_label = sym_remove;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoremove_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undoremove,
            NULL,
            NULL,
            sizeof (t_undoremove),
            CLASS_INVISIBLE,
            A_NULL);
    
    undoremove_class = c;
}

void undoremove_destroy (void)
{
    class_free (undoremove_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
