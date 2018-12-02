
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
