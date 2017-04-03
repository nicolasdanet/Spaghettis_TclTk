
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void glist_makeLineProceed (t_glist *glist, int a, int b, int end)
{
    int startX = drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int startY = drag_getStartY (editor_getDrag (glist_getEditor (glist)));
    
    t_rectangle r1;
    t_rectangle r2;
    
    t_gobj *t1 = glist_objectHit (glist, startX, startY, &r1);
    t_gobj *t2 = glist_objectHit (glist, a, b, &r2);
    
    if (end) { glist_eraseTemporary (glist); }
    else {
        glist_updateTemporary (glist, startX, startY, a, b);
    }

    if (t1 && t2) {
    //
    t_object *o1 = cast_objectIfConnectable (t1);
    t_object *o2 = cast_objectIfConnectable (t2);
    
    if (o1 && o2 && o1 != o2) {
    //
    int numberOfOutlets = object_getNumberOfOutlets (o1);
    int numberOfInlets  = object_getNumberOfInlets (o2);
    
    if (numberOfOutlets && numberOfInlets) {
    //
    int k1 = inlet_closest (startX, numberOfOutlets, &r1);
    int k2 = inlet_closest (a, numberOfInlets, &r2);

    if (!glist_lineExist (glist, o1, k1, o2, k2)) {
    //
    if (object_isSignalOutlet (o1, k1) && !object_isSignalInlet (o2, k2)) { }
    else {
    //
    if (!end) { glist_updateCursor (glist, CURSOR_CONNECT); }
    else {
        t_cord t; cord_make (&t, object_connect (o1, k1, o2, k2), o1, k1, o2, k2, glist);
        glist_drawLine (glist, &t);
        glist_setDirty (glist, 1);
    }
    
    return;
    //
    }
    //
    }
    //
    }
    //
    }
    //
    }
    
    glist_updateCursor (glist, CURSOR_NOTHING);
}

void glist_makeLineBegin (t_glist *glist, int a, int b)
{
    glist_makeLineProceed (glist, a, b, 0);
}

void glist_makeLineEnd (t_glist *glist, int a, int b)
{
    glist_makeLineProceed (glist, a, b, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void glist_motionResizeBox (t_glist *glist, t_gobj *y, int width)
{
    int w = (int)(width / font_getHostFontWidth (glist_getFontSize (glist)));
    
    gobj_visibilityChanged (y, glist, 0);
    object_setWidth (cast_object (y), PD_MAX (1, w));
    glist_updateLinesForObject (glist, cast_object (y));
    gobj_visibilityChanged (y, glist, 1);
    glist_setDirty (glist, 1);
}

static void glist_motionResizeGraph (t_glist *glist, t_gobj *y, int deltaX, int deltaY)
{
    if (pd_class (y) != canvas_class) { PD_BUG; }
    else {
    //
    t_rectangle *r = glist_getGraphGeometry (cast_glist (y));
    
    gobj_visibilityChanged (y, glist, 0);
    rectangle_setWidth (r, rectangle_getWidth (r) + deltaX);
    rectangle_setHeight (r, rectangle_getHeight (r) + deltaY);
    glist_updateLinesForObject (glist, cast_object (y));
    glist_updateRectangle (cast_glist (y));
    gobj_visibilityChanged (y, glist, 1);
    glist_setDirty (glist, 1);
    //
    }
}

void glist_motionResize (t_glist *glist, int a, int b)
{
    t_rectangle dummy;
    
    /* Points below are the coordinates of the box of the resized object. */
    
    int startX = drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int startY = drag_getStartY (editor_getDrag (glist_getEditor (glist)));
    int endX   = drag_getEndX (editor_getDrag (glist_getEditor (glist)));
    int endY   = drag_getEndY (editor_getDrag (glist_getEditor (glist)));

    t_gobj *y  = glist_objectHit (glist, startX, startY, &dummy);

    if (y && cast_objectIfConnectable (y)) {
    
        if (object_isViewAsBox (cast_object (y))) { glist_motionResizeBox (glist, y, a - startX); }
        else {
            glist_motionResizeGraph (glist, y, a - endX, b - endY);
        }
    }
    
    drag_setEnd (editor_getDrag (glist_getEditor (glist)), a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
