
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class          *canvas_class;
extern t_class          *scalar_class;
extern t_pdinstance     *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PAINT_DRAW      0
#define PAINT_ERASE     1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void paint_proceedAllRecursive (t_glist *glist, int action)
{
    t_gobj *y = NULL;
    
    int visible = canvas_isMapped (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_glist *z = cast_glistChecked (cast_pd (y));
    
    if (z) { paint_proceedAllRecursive (z, action); }
    else {
        if (visible && pd_class (y) == scalar_class) {
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

    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) {
        paint_proceedAllRecursive (glist, action);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
