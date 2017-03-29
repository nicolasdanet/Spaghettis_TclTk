
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
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_behaviorGetRectangle         (t_gobj *, t_glist *, t_rectangle *);
static void canvas_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void canvas_behaviorSelected             (t_gobj *, t_glist *, int);
static void canvas_behaviorActivated            (t_gobj *, t_glist *, int);
static void canvas_behaviorDeleted              (t_gobj *, t_glist *);
static void canvas_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  canvas_behaviorMouse                (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_widgetbehavior canvas_widgetbehavior =        /* Shared. */
    {
        canvas_behaviorGetRectangle,
        canvas_behaviorDisplaced,
        canvas_behaviorSelected,
        canvas_behaviorActivated,
        canvas_behaviorDeleted,
        canvas_behaviorVisibilityChanged,
        canvas_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet *canvas_addInlet (t_glist *glist, t_pd *receiver, t_symbol *s)
{
    t_inlet *inlet = inlet_new (cast_object (glist), receiver, s, NULL);
    
    if (!glist_isLoading (glist)) {
    
        if (glist_hasParentOnScreen (glist)) {
            gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
            gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
            glist_updateLines (glist_getParent (glist), cast_object (glist));
        }
    
        canvas_resortInlets (glist);
    }
    
    return inlet;
}

t_outlet *canvas_addOutlet (t_glist *glist, t_symbol *s)
{
    t_outlet *outlet = outlet_new (cast_object (glist), s);
    
    if (!glist_isLoading (glist)) {
    
        if (glist_hasParentOnScreen (glist)) {
            gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 0);
            gobj_visibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
            glist_updateLines (glist_getParent (glist), cast_object (glist));
        }
        
        canvas_resortOutlets (glist);
    }
    
    return outlet;
}

void canvas_removeInlet (t_glist *glist, t_inlet *inlet)
{
    t_glist *o = glist_getParent (glist);
    
    int redraw = (o && !glist_isDeleting (o) && glist_isOnScreen (o) && glist_isWindowable (o));
    
    if (o) { canvas_deleteLinesByInlets (o, cast_object (glist), inlet, NULL); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), o, 0); }
        
    inlet_free (inlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), o, 1); }
    if (o) { glist_updateLines (o, cast_object (glist)); }
}

void canvas_removeOutlet (t_glist *glist, t_outlet *outlet)
{
    t_glist *o = glist_getParent (glist);
    
    int redraw = (o && !glist_isDeleting (o) && glist_isOnScreen (o) && glist_isWindowable (o));
    
    if (o) { canvas_deleteLinesByInlets (o, cast_object (glist), NULL, outlet); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), o, 0); }

    outlet_free (outlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), o, 1); }
    if (o) { glist_updateLines (o, cast_object (glist)); }
}

void canvas_resortInlets (t_glist *glist)
{
    int numberOfInlets = 0;
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    // 
    if (pd_class (y) == vinlet_class) { numberOfInlets++; }
    //
    }

    if (numberOfInlets > 1) {
    //
    int i;
    t_gobj **inlets = (t_gobj **)PD_MEMORY_GET (numberOfInlets * sizeof (t_gobj *));
    t_gobj **t = inlets;
        
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == vinlet_class) { *t++ = y; } }
    
    /* Take the most right inlet and put it first. */
    /* Remove it from the list. */
    /* Do it again. */
    
    for (i = numberOfInlets; i > 0; i--) {
    //
    int j = numberOfInlets;
    int maximumX = -PD_INT_MAX;
    t_gobj **mostRightObject = NULL;
    
    for (t = inlets; j--; t++) {
        if (*t) {
            t_rectangle r;
            gobj_getRectangle (*t, glist, &r);
            if (rectangle_getTopLeftX (&r) > maximumX) {
                maximumX = rectangle_getTopLeftX (&r); mostRightObject = t; 
            }
        }
    }
    
    if (mostRightObject) {
        inlet_moveFirst (vinlet_getInlet (cast_pd (*mostRightObject))); *mostRightObject = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (inlets);
    
    if (glist_hasParentOnScreen (glist)) {
        glist_updateLines (glist_getParent (glist), cast_object (glist));
    }
    //
    }
}

void canvas_resortOutlets (t_glist *glist)
{
    int numberOfOutlets = 0;
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    if (pd_class (y) == voutlet_class) { numberOfOutlets++; }
    //
    }

    if (numberOfOutlets > 1) {
    //
    int i;
    t_gobj **outlets = (t_gobj **)PD_MEMORY_GET (numberOfOutlets * sizeof (t_gobj *));
    t_gobj **t = outlets;
        
    for (y = glist->gl_graphics; y; y = y->g_next) { if (pd_class (y) == voutlet_class) { *t++ = y; } }
    
    /* Take the most right outlet and put it first. */
    /* Remove it from the list. */
    /* Do it again. */
    
    for (i = numberOfOutlets; i > 0; i--) {
    //
    int j = numberOfOutlets;
    int maximumX = -PD_INT_MAX;
    t_gobj **mostRightObject = NULL;
    
    for (t = outlets; j--; t++) {
        if (*t) {
            t_rectangle r;
            gobj_getRectangle (*t, glist, &r);
            if (rectangle_getTopLeftX (&r) > maximumX) {
                maximumX = rectangle_getTopLeftX (&r); mostRightObject = t; 
            }
        }
    }
    
    if (mostRightObject) {
        outlet_moveFirst (voutlet_getOutlet (cast_pd (*mostRightObject))); *mostRightObject = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (outlets);
    
    if (glist_hasParentOnScreen (glist)) {
        glist_updateLines (glist_getParent (glist), cast_object (glist));
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_redrawGraphOnParent (t_glist *glist)
{  
    if (glist_isOnScreen (glist)) {
    //
    glist_updateWindow (glist);
    
    if (glist_hasParentOnScreen (glist)) {
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 0); 
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_bounds (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    t_error err = bounds_setByAtoms (glist_getBounds (glist), argc, argv);
    
    if (!err) { canvas_redrawGraphOnParent (glist); }
    else {
        error_invalid (sym_graph, sym_bounds); 
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_getGraphOnParentRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_glist *x = cast_glist (z);
    
    PD_ASSERT (pd_class (z) == canvas_class);
    
    int a = object_getPixelX (cast_object (x), glist);
    int b = object_getPixelY (cast_object (x), glist);
    int c = a + rectangle_getWidth (glist_getGraphGeometry (x));
    int d = b + rectangle_getHeight (glist_getGraphGeometry (x));
    
    rectangle_set (r, a, b, c, d);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float canvas_pixelToValueX (t_glist *glist, t_float f)
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

t_float canvas_pixelToValueY (t_glist *glist, t_float f)
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

t_float canvas_valueToPixelX (t_glist *glist, t_float f)
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

t_float canvas_valueToPixelY (t_glist *glist, t_float f)
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

t_float canvas_valueForDeltaInPixelX (t_glist *glist, t_float f)
{ 
    return (f * canvas_valueForOnePixelX (glist));
}

t_float canvas_valueForDeltaInPixelY (t_glist *glist, t_float f)
{
    return (f * canvas_valueForOnePixelY (glist));
}

t_float canvas_valueForOnePixelX (t_glist *glist)
{
    return (canvas_pixelToValueX (glist, (t_float)1.0) - canvas_pixelToValueX (glist, (t_float)0.0));
}

t_float canvas_valueForOnePixelY (t_glist *glist)
{
    return (canvas_pixelToValueY (glist, (t_float)1.0) - canvas_pixelToValueY (glist, (t_float)0.0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnGetRectangle (z, glist, r); }
    else {
        canvas_getGraphOnParentRectangle (z, glist, r);
    }
}

static void canvas_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnDisplaced (z, glist, deltaX, deltaY); }
    else {
        object_setX (cast_object (z), object_getX (cast_object (z)) + deltaX);
        object_setY (cast_object (z), object_getY (cast_object (z)) + deltaY);
        canvas_redrawGraphOnParent (x);
        glist_updateLines (glist, cast_object (z));
    }
}

static void canvas_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_glist *x = cast_glist (z);
    
    glist_setSelected (x, (isSelected != 0));
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnSelected (z, glist, isSelected); }
    else {
    
    int color = glist_hasWindow (x) ? COLOR_MASKED : (glist_isSelected (x) ? COLOR_SELECTED : COLOR_NORMAL);
    
    sys_vGui (".x%lx.c itemconfigure %lxGRAPH -fill #%06x\n",
                    glist_getView (glist),
                    x,
                    color);
    }
}

static void canvas_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnActivated (z, glist, isActivated); }
}

static void canvas_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    t_glist *x = cast_glist (z);
    
    t_gobj *y = NULL;
    
    while ((y = x->gl_graphics)) { glist_objectRemove (x, y); }
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnDeleted (z, glist); }
    else {
        canvas_deleteLinesByObject (glist, cast_object (z));
    }
}

static void canvas_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_glist *x = cast_glist (z);

    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnVisibilityChanged (z, glist, isVisible); }
    else {
    //
    char tag[PD_STRING] = { 0 }; t_error err = string_sprintf (tag, PD_STRING, "%lxGRAPH", x);
    
    t_rectangle r;
    
    canvas_behaviorGetRectangle (z, glist, &r);
    
    if (!err) {
    //
    int a = rectangle_getTopLeftX (&r);
    int b = rectangle_getTopLeftY (&r);
    int c = rectangle_getBottomRightX (&r);
    int d = rectangle_getBottomRightY (&r);
        
    if (glist_hasWindow (x)) {
    //
    if (isVisible) {
        
        sys_vGui (".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d"
                        " -fill #%06x"
                        " -tags %s\n",
                        glist_getView (glist_getParent (x)),
                        a,
                        b,
                        a,
                        d,
                        c,
                        d,
                        c,
                        b,
                        a,
                        b,
                        COLOR_MASKED,
                        tag);
    } else {
                
        sys_vGui (".x%lx.c delete %s\n",
                        glist_getView (glist_getParent (x)),
                        tag);
    }
    //
    } else {
    //
    t_gobj *y = NULL;
                
    if (isVisible) {
        
        int i = 0;

        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                        " -fill #%06x"
                        " -tags %s\n",
                        glist_getView (glist_getParent (x)),
                        a,
                        b,
                        a,
                        d,
                        c,
                        d,
                        c,
                        b,
                        a,
                        b,
                        (glist_isSelected (x) ? COLOR_SELECTED : COLOR_NORMAL),
                        tag);
        
        for (y = x->gl_graphics; y; y = y->g_next) {
        //
        if (pd_class (y) == garray_class) {
        //
        sys_vGui (".x%lx.c create text %d %d -text {%s}"    // --
                        " -anchor nw"
                        " -font [::getFont %d]"             // --
                        " -fill #%06x"
                        " -tags %s\n",
                        glist_getView (glist_getParent (x)),
                        a,
                        b - (++i) * (int)font_getHostFontHeight (glist_getFontSize (x)),
                        garray_getName ((t_garray *)y)->s_name,
                        font_getHostFontSize (glist_getFontSize (x)),
                        (glist_isSelected (x) ? COLOR_SELECTED : COLOR_NORMAL),
                        tag);
        //
        }
        //
        }
        
        for (y = x->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, x, 1); }
            
    } else {
    
        sys_vGui (".x%lx.c delete %s\n",
                    glist_getView (glist_getParent (x)),
                    tag);
                    
        for (y = x->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, x, 0); }
    }
    //
    }
    //
    }
    //
    }
}

static int canvas_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_glist *x = cast_glist (z);

    if (!glist_isGraphOnParent (x)) { return (text_widgetBehavior.w_fnMouse (z, glist, m)); }
    else {
    //
    if (glist_hasWindow (x)) { return 0; }
    else {
        int k = 0;
        int a = m->m_x;
        int b = m->m_y;
        
        t_gobj *y = NULL;
            
        for (y = x->gl_graphics; y; y = y->g_next) {
            t_rectangle t;
            if (gobj_hit (y, x, a, b, &t)) {
                if ((k = gobj_mouse (y, x, m))) {
                    break;
                }
            }
        }

        return k; 
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
