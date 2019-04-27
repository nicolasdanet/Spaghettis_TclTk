
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int instance_pendingRequired (t_gobj *y)
{
    if (instance_hasPending()) {
    //
    return class_hasPendingRequired (pd_class (y));
    //
    }
    
    return 0;
}

t_gobj *instance_pendingFetch (t_gobj *y)
{
    if (instance_hasPending()) {
    //
    t_id u1 = gobj_getUnique (y);
    t_id s1 = gobj_getSource (y);
    
    t_gobj *t = instance_get()->pd_pending;
    
    while (t) {
    //
    t_id u2 = gobj_getUnique (t);
    t_id s2 = gobj_getSource (t);
        
    if (u1 == u2 || u1 == s2 || s1 == u2 || s1 == s2) { return t; } else { t = t->g_next; }
    //
    }
    //
    }
    
    return NULL;
}

void instance_pendingAdd (t_gobj *y)
{
    if (class_hasDismissFunction (pd_class (y))) { (*class_getDismissFunction (pd_class (y))) (y); }

    y->g_next = instance_get()->pd_pending; instance_get()->pd_pending = y;
}

void instance_pendingRelease (void)
{
    t_gobj *y = instance_get()->pd_pending; instance_get()->pd_pending = NULL;
    
    while (y) { t_gobj *t = y; y = y->g_next; pd_free (cast_pd (t)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
