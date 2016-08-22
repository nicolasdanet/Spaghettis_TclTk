
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class          *canvas_class;
extern t_class          *scalar_class;
extern t_pdinstance     *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PAINT_REDRAW    0
#define PAINT_DRAW      1
#define PAINT_ERASE     2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void paint_performAllRecursive (t_glist *glist, int action)
{
    t_gobj *y = NULL;
    
    int visible = canvas_isMapped (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_glist *z = canvas_castToGlistChecked (cast_pd (y));
    
    if (z) { paint_performAllRecursive (z, action); }
    else {
        if (visible && pd_class (y) == scalar_class) {
            switch (action) {
                case PAINT_REDRAW   : scalar_redraw (cast_scalar (y), glist);   break;
                case PAINT_DRAW     : gobj_visibilityChanged (y, glist, 1);     break;
                case PAINT_ERASE    : gobj_visibilityChanged (y, glist, 0);     break;
            }
        }
    } 
    //
    }
}

static void paint_performAll (int action)
{
    t_glist *glist = NULL;

    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) {
    //
    paint_performAllRecursive (glist, action);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void paint_scalarsEraseAll (void)
{
    paint_performAll (PAINT_ERASE);
}

void paint_scalarsDrawAll (void)
{
    paint_performAll (PAINT_DRAW);
}

void paint_scalarsRedrawAll (void)
{
    paint_performAll (PAINT_REDRAW);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
