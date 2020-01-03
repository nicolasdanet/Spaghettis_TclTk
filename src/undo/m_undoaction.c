
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
// MARK: -

void undoaction_releaseAllFrom (t_undoaction *chainToRelease, t_undomanager *x)
{
    t_undoaction *a = chainToRelease;
    
    while (a) {
    //
    t_undoaction *next = a->ua_next; pd_free (cast_pd (a)); a = next; if (x) { x->um_count--; }
    //
    }
}

void undoaction_release (t_undoaction *a, t_undomanager *x)
{
    t_undoaction *previous = a->ua_previous;
    t_undoaction *next     = a->ua_next;
    
    if (previous) { previous->ua_next = next; }
    if (next)     { next->ua_previous = previous; }
    
    a->ua_previous = NULL;
    a->ua_next     = NULL;
    
    undoaction_releaseAllFrom (a, x);
}

void undoaction_setInletsAndOutlets (t_undoaction *a, t_gobj *y)
{
    if (pd_class (y) == vinlet_class) {
        a->ua_inlet       = 1;
        a->ua_inletIndex  = vinlet_getIndex ((t_vinlet *)y);

    } else if (pd_class (y) == voutlet_class) {
        a->ua_outlet      = 1;
        a->ua_outletIndex = voutlet_getIndex ((t_voutlet *)y);
    }
}

int undoaction_getInletsAndOutlets (t_undoaction *a, t_items *i, t_items *o)
{
    if (a->ua_inlet == 1)       { items_setAtIndex (i, a->ua_inletIndex);  return 1; }
    else if (a->ua_outlet == 1) { items_setAtIndex (o, a->ua_outletIndex); return 1; }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
