
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
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *canvas_class;
extern t_class  *garray_class;
extern t_class  *vinlet_class;
extern t_class  *voutlet_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_behaviorGetRectangle         (t_gobj *, t_glist *, int *, int *, int *, int *);
static void canvas_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void canvas_behaviorSelected             (t_gobj *, t_glist *, int);
static void canvas_behaviorActivated            (t_gobj *, t_glist *, int);
static void canvas_behaviorDeleted              (t_gobj *, t_glist *);
static void canvas_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  canvas_behaviorMouse                (t_gobj *, t_glist *, int, int, int, int, int, int, int);

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

t_inlet *canvas_addInlet (t_glist *glist, t_pd *owner, t_symbol *s)
{
    t_inlet *inlet = inlet_new (cast_object (glist), owner, s, NULL);
    
    if (!glist->gl_isLoading) {
    
        if (glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 0);
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 1);
            canvas_updateLinesByObject (glist->gl_parent, cast_object (glist));
        }
    
        canvas_resortInlets (glist);
    }
    
    return inlet;
}

t_outlet *canvas_addOutlet (t_glist *glist, t_pd *dummy, t_symbol *s)
{
    t_outlet *outlet = outlet_new (cast_object (glist), s);
    
    if (!glist->gl_isLoading) {
    
        if (glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 0);
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 1);
            canvas_updateLinesByObject (glist->gl_parent, cast_object (glist));
        }
        
        canvas_resortOutlets (glist);
    }
    
    return outlet;
}

void canvas_removeInlet (t_glist *glist, t_inlet *inlet)
{
    t_glist *owner = glist->gl_parent;
    
    int redraw = (owner && !owner->gl_isDeleting && canvas_isMapped (owner) && canvas_canHaveWindow (owner));
    
    if (owner)  { canvas_deleteLinesByInlets (owner, cast_object (glist), inlet, NULL); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 0); }
        
    inlet_free (inlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 1); }
    if (owner)  { canvas_updateLinesByObject (owner, cast_object (glist)); }
}

void canvas_removeOutlet (t_glist *glist, t_outlet *outlet)
{
    t_glist *owner = glist->gl_parent;
    
    int redraw = (owner && !owner->gl_isDeleting && canvas_isMapped (owner) && canvas_canHaveWindow (owner));
    
    if (owner)  { canvas_deleteLinesByInlets (owner, cast_object (glist), NULL, outlet); }
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 0); }

    outlet_free (outlet);
    
    if (redraw) { gobj_visibilityChanged (cast_gobj (glist), owner, 1); }
    if (owner)  { canvas_updateLinesByObject (owner, cast_object (glist)); }
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
            int a, b, c, d;
            gobj_getRectangle (*t, glist, &a, &b, &c, &d);
            if (a > maximumX) { maximumX = a; mostRightObject = t; }
        }
    }
    
    if (mostRightObject) {
        object_moveInletFirst (cast_object (glist), vinlet_getInlet (cast_pd (*mostRightObject)));
        *mostRightObject = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (inlets);
    
    if (glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
        canvas_updateLinesByObject (glist->gl_parent, cast_object (glist));
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
            int a, b, c, d;
            gobj_getRectangle (*t, glist, &a, &b, &c, &d);
            if (a > maximumX) { maximumX = a; mostRightObject = t; }
        }
    }
    
    if (mostRightObject) {
        object_moveOutletFirst (cast_object (glist), voutlet_getOutlet (cast_pd (*mostRightObject)));
        *mostRightObject = NULL;
    }
    //
    }
    
    PD_MEMORY_FREE (outlets);
    
    if (glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
        canvas_updateLinesByObject (glist->gl_parent, cast_object (glist));
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_redrawGraphOnParent (t_glist *glist)
{  
    if (canvas_isMapped (glist)) {
    //
    if (canvas_canHaveWindow (glist)) { canvas_redraw (glist); }
    
    if (glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist->gl_parent, 0); 
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist->gl_parent, 1);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_bounds (t_glist *glist, t_float a, t_float b, t_float c, t_float d)
{
    if ((a == b) || (c == d)) { error_invalid (sym_graph, sym_bounds); }
    else {
        glist->gl_valueLeft   = a;
        glist->gl_valueRight  = c;
        glist->gl_valueTop    = b;
        glist->gl_valueBottom = d;
    
        canvas_redrawGraphOnParent (glist);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_getGraphOnParentRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_glist *x = cast_glist (z);
    
    PD_ASSERT (pd_class (z) == canvas_class);
    
    int xA = text_getPixelX (cast_object (x), glist);
    int yA = text_getPixelY (cast_object (x), glist);
    int xB = xA + x->gl_graphWidth;
    int yB = yA + x->gl_graphHeight;

    *a = xA;
    *b = yA;
    *c = xB;
    *d = yB;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float canvas_pixelToValueX (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueRight - glist->gl_valueLeft;
    t_float v = (t_float)0.0;
        
    if (!glist->gl_isGraphOnParent) { v = f; }      /* Scalars. */
    else {
        if (glist->gl_hasWindow)    { v = f / (glist->gl_windowBottomRightX - glist->gl_windowTopLeftX); }
        else {
            int a, b, c, d;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist->gl_parent, &a, &b, &c, &d);
            v = (f - a) / (c - a);
        }
    }

    return (glist->gl_valueLeft + (range * v));
}

t_float canvas_pixelToValueY (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueBottom - glist->gl_valueTop;
    t_float v = (t_float)0.0;
        
    if (!glist->gl_isGraphOnParent) { v = f; }      /* Scalars. */
    else {
        if (glist->gl_hasWindow)    { v = f / (glist->gl_windowBottomRightY - glist->gl_windowTopLeftY); }
        else {
            int a, b, c, d;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist->gl_parent, &a, &b, &c, &d);
            v = (f - b) / (d - b);
        }
    }
    
    return (glist->gl_valueTop + (range * v));
}

t_float canvas_valueToPixelX (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueRight - glist->gl_valueLeft;
    t_float v = (t_float)1.0;
    t_float x = (t_float)0.0;
    
    if (!glist->gl_isGraphOnParent) { }     /* Scalars. */
    else {
        if (glist->gl_hasWindow)    { v = glist->gl_windowBottomRightX - glist->gl_windowTopLeftX; }
        else {
            int a, b, c, d;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist->gl_parent, &a, &b, &c, &d);
            x = a;
            v = c - a;
        }
    }
    
    return (x + (v * ((f - glist->gl_valueLeft) / range)));
}

t_float canvas_valueToPixelY (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueBottom - glist->gl_valueTop;
    t_float v = (t_float)1.0;
    t_float x = (t_float)0.0;
    
    if (!glist->gl_isGraphOnParent) { }     /* Scalars. */
    else {
        if (glist->gl_hasWindow)    { v = glist->gl_windowBottomRightY - glist->gl_windowTopLeftY; }
        else {
            int a, b, c, d;
            canvas_getGraphOnParentRectangle (cast_gobj (glist), glist->gl_parent, &a, &b, &c, &d);
            x = b;
            v = d - b;
        }
    }
    
    return (x + (v * ((f - glist->gl_valueTop) / range)));
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

static void canvas_behaviorGetRectangle (t_gobj *z,
    t_glist *glist,
    int *a,
    int *b,
    int *c,
    int *d)
{
    int xA, yA, xB, yB;

    t_glist *x = cast_glist (z);
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnGetRectangle (z, glist, &xA, &yA, &xB, &yB); }
    else {
        canvas_getGraphOnParentRectangle (z, glist, &xA, &yA, &xB, &yB);
    }
    
    *a = xA;
    *b = yA;
    *c = xB;
    *d = yB;
}

static void canvas_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_glist *x = cast_glist (z);
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnDisplaced (z, glist, deltaX, deltaY); }
    else {
        cast_object (z)->te_xCoordinate += deltaX;
        cast_object (z)->te_yCoordinate += deltaY;
        canvas_redrawGraphOnParent (x);
        canvas_updateLinesByObject (glist, cast_object (z));
    }
}

static void canvas_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_glist *x = cast_glist (z);
    
    x->gl_isSelected = (isSelected != 0);
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnSelected (z, glist, isSelected); }
    else {
        sys_vGui (".x%lx.c itemconfigure %lxGRAPH -fill #%06x\n",
                        canvas_getView (glist),
                        x,
                        x->gl_hasWindow ? COLOR_MASKED : (x->gl_isSelected ? COLOR_SELECTED : COLOR_NORMAL));
    }
}

static void canvas_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    t_glist *x = cast_glist (z);
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnActivated (z, glist, isActivated); }
}

static void canvas_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    t_glist *x = cast_glist (z);
    
    t_gobj *y = NULL;
    
    while ((y = x->gl_graphics)) { canvas_removeObject (x, y); }
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnDeleted (z, glist); }
    else {
        canvas_deleteLinesByObject (glist, cast_object (z));
    }
}

static void canvas_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_glist *x = cast_glist (z);

    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnVisibilityChanged (z, glist, isVisible); }
    else {
    //
    char tag[PD_STRING] = { 0 };
    t_error err = string_sprintf (tag, PD_STRING, "%lxGRAPH", x);
    int xA, yA, xB, yB;
    
    canvas_behaviorGetRectangle (z, glist, &xA, &yA, &xB, &yB);
    
    PD_ASSERT (!err);
        
    if (x->gl_hasWindow) {
    //
    if (isVisible) {
        
        sys_vGui (".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d"
                        " -fill #%06x"
                        " -tags %s\n",
                        canvas_getView (x->gl_parent),
                        xA,
                        yA,
                        xA,
                        yB,
                        xB,
                        yB,
                        xB,
                        yA,
                        xA,
                        yA,
                        COLOR_MASKED,
                        tag);
    } else {
                
        sys_vGui (".x%lx.c delete %s\n",
                        canvas_getView (x->gl_parent),
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
                        canvas_getView (x->gl_parent),
                        xA,
                        yA,
                        xA,
                        yB,
                        xB,
                        yB,
                        xB,
                        yA,
                        xA,
                        yA,
                        (x->gl_isSelected ? COLOR_SELECTED : COLOR_NORMAL),
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
                        canvas_getView (x->gl_parent),
                        xA,
                        yA - (++i) * (int)font_getHostFontHeight (canvas_getFontSize (x)),
                        garray_getName ((t_garray *)y)->s_name,
                        font_getHostFontSize (canvas_getFontSize (x)),
                        (x->gl_isSelected ? COLOR_SELECTED : COLOR_NORMAL),
                        tag);
        //
        }
        //
        }
        
        for (y = x->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, x, 1); }
            
    } else {
    
        sys_vGui (".x%lx.c delete %s\n",
                    canvas_getView (x->gl_parent),
                    tag);
                    
        for (y = x->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, x, 0); }
    }
    //
    }
    //
    }
}

static int canvas_behaviorMouse (t_gobj *z,
    t_glist *glist,
    int a,
    int b,
    int shift,
    int ctrl,
    int alt,
    int dbl,
    int clicked)
{
    t_glist *x = cast_glist (z);

    if (!x->gl_isGraphOnParent) {
        return (text_widgetBehavior.w_fnMouse (z, glist, a, b, shift, ctrl, alt, dbl, clicked));
        
    } else {
    //
    if (x->gl_hasWindow) { return 0; }
    else {
        int k = 0;
        
        t_gobj *y = NULL;
            
        for (y = x->gl_graphics; y; y = y->g_next) {
            int xA, yA, xB, yB;
            if (gobj_hit (y, x, a, b, &xA, &yA, &xB, &yB)) {
                if ((k = gobj_mouse (y, x, a, b, shift, ctrl, alt, 0, clicked))) {
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
