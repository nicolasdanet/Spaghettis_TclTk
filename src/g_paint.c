
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PAINT_DRAW      0
#define PAINT_ERASE     1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void paint_proceedAllRecursive (t_glist *glist, int action)
{
    t_gobj *y = NULL;
    
    int visible = glist_isOnScreen (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_glist *z = gobj_isCanvas (y) ? cast_glist (y) : NULL;
    
    if (z) { paint_proceedAllRecursive (z, action); }
    else {
        if (visible && gobj_isScalar (y)) {
            switch (action) {
                case PAINT_DRAW  : gobj_visibilityChanged (y, glist, 1); break;
                case PAINT_ERASE : gobj_visibilityChanged (y, glist, 0); break;
            }
        }
    } 
    //
    }
}

static void paint_proceedAll (int action)
{
    t_glist *glist = NULL;

    for (glist = instance_getRoots(); glist; glist = glist_getNext (glist)) {
        paint_proceedAllRecursive (glist, action);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void paint_erase (void)
{
    paint_proceedAll (PAINT_ERASE);
}

void paint_draw (void)
{
    paint_proceedAll (PAINT_DRAW);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
