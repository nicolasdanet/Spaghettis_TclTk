
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

static t_class *undoconnect_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undoconnect {
    t_undoaction    x_undo;                 /* Must be the first. */
    t_id            x_src;
    t_id            x_dest;
    int             x_m;
    int             x_n;
    } t_undoconnect;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoconnect_undo (t_undoconnect *z, t_symbol *s, int argc, t_atom *argv)
{
    glist_lineDisconnectByUnique (z->x_src, z->x_m, z->x_dest, z->x_n);
}

void undoconnect_redo (t_undoconnect *z, t_symbol *s, int argc, t_atom *argv)
{
    glist_lineConnectByUnique (z->x_src, z->x_m, z->x_dest, z->x_n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undoconnect_new (t_object *src, int m, t_object *dest, int n)
{
    t_undoaction *x  = (t_undoaction *)pd_new (undoconnect_class);
    t_undoconnect *z = (t_undoconnect *)x;
    
    int safe = (object_isSignalOutlet (src, m) == 0);
    
    x->ua_type  = UNDO_CONNECT;
    x->ua_safe  = safe;
    x->ua_label = sym_connect;
    
    z->x_src    = gobj_getUnique (cast_gobj (src));
    z->x_dest   = gobj_getUnique (cast_gobj (dest));
    z->x_m      = m;
    z->x_n      = n;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoconnect_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_undoconnect,
            NULL,
            NULL,
            sizeof (t_undoconnect),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addMethod (c, (t_method)undoconnect_undo, sym_undo, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)undoconnect_redo, sym_redo, A_GIMME, A_NULL);
    
    undoconnect_class = c;
}

void undoconnect_destroy (void)
{
    class_free (undoconnect_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
