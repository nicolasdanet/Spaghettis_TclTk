
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

static void canvas_redrawAllScalars (t_glist *glist, int action)
{
    t_gobj *y = NULL;
    
    int visible = canvas_isMapped (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    if (visible && pd_class (y) == scalar_class) {
    //
    switch (action) {
        case SCALAR_REDRAW  : scalar_redraw (cast_scalar (y), glist);   break;
        case SCALAR_DRAW    : gobj_visibilityChanged (y, glist, 1);     break;
        case SCALAR_ERASE   : gobj_visibilityChanged (y, glist, 0);     break;
    }
    //
    } 

    if (pd_class (y) == canvas_class) { canvas_redrawAllScalars (cast_glist (y), action); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Note that the functions below are experimentals. */
/* The template argument are not used and everything is redrawn instead. */

void canvas_paintAllScalarsByTemplate (t_template *dummy, int action)
{
    t_glist *glist = NULL;

    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) {
    //
    canvas_redrawAllScalars (glist, action);
    //
    }
}

void canvas_paintAllScalarsByView (t_glist *glist, int action)
{
    canvas_paintAllScalarsByTemplate (NULL, action);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
