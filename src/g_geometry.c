
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_getGraphOnParentRectangle (t_gobj *, t_glist *, t_rectangle *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float glist_pixelToValueX (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeX (glist_getBounds (glist));
    t_float v = (t_float)0.0;
        
    if (!glist_isGraphOnParent (glist)) { v = f; }      /* Scalars. */
    else {
        if (glist_hasWindow (glist)) { v = f / rectangle_getWidth (glist_getWindowGeometry (glist)); }
        else {
            t_rectangle r;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist_getParent (glist), &r);
            v = (f - rectangle_getTopLeftX (&r)) / rectangle_getWidth (&r);
        }
    }

    return (bounds_getLeft (glist_getBounds (glist)) + (range * v));
}

t_float glist_pixelToValueY (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeY (glist_getBounds (glist));
    t_float v = (t_float)0.0;
        
    if (!glist_isGraphOnParent (glist)) { v = f; }      /* Scalars. */
    else {
        if (glist_hasWindow (glist)) { v = f / rectangle_getHeight (glist_getWindowGeometry (glist)); }
        else {
            t_rectangle r;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist_getParent (glist), &r);
            v = (f - rectangle_getTopLeftY (&r)) / rectangle_getHeight (&r);
        }
    }
    
    return (bounds_getTop (glist_getBounds (glist)) + (range * v));
}

t_float glist_valueToPixelX (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeX (glist_getBounds (glist));
    t_float v = (t_float)1.0;
    t_float x = (t_float)0.0;
    
    if (!glist_isGraphOnParent (glist)) { }     /* Scalars. */
    else {
        if (glist_hasWindow (glist)) { v = rectangle_getWidth (glist_getWindowGeometry (glist)); }
        else {
            t_rectangle r;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist_getParent (glist), &r);
            x = rectangle_getTopLeftX (&r);
            v = rectangle_getWidth (&r);
        }
    }
    
    return (x + (v * ((f - bounds_getLeft (glist_getBounds (glist))) / range)));
}

t_float glist_valueToPixelY (t_glist *glist, t_float f)
{
    t_float range = bounds_getRangeY (glist_getBounds (glist));
    t_float v = (t_float)1.0;
    t_float x = (t_float)0.0;
    
    if (!glist_isGraphOnParent (glist)) { }     /* Scalars. */
    else {
        if (glist_hasWindow (glist)) { v = rectangle_getHeight (glist_getWindowGeometry (glist)); }
        else {
            t_rectangle r;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist_getParent (glist), &r);
            x = rectangle_getTopLeftY (&r);
            v = rectangle_getHeight (&r);
        }
    }
    
    return (x + (v * ((f - bounds_getTop (glist_getBounds (glist))) / range)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float glist_valueForOnePixelX (t_glist *glist)
{
    return (glist_pixelToValueX (glist, (t_float)1.0) - glist_pixelToValueX (glist, (t_float)0.0));
}

t_float glist_valueForOnePixelY (t_glist *glist)
{
    return (glist_pixelToValueY (glist, (t_float)1.0) - glist_pixelToValueY (glist, (t_float)0.0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int glist_getPixelX (t_glist *glist, t_object *x)
{
    if (glist_isWindowable (glist)) { return object_getX (x); }
    else {
    //
    int n = glist_valueToPixelX (glist, bounds_getLeft (glist_getBounds (glist)));
    
    n -= rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    n += object_getX (x);
    
    return n;
    //
    }
}

int glist_getPixelY (t_glist *glist, t_object *x)
{
    if (glist_isWindowable (glist)) { return object_getY (x); }
    else {
    //
    int n = glist_valueToPixelY (glist, bounds_getTop (glist_getBounds (glist)));
    
    n -= rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    n += object_getY (x);
    
    return n;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
