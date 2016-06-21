
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
#include "g_canvas.h"
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
static int  canvas_behaviorClicked              (t_gobj *, t_glist *, int, int, int, int, int, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_widgetbehavior canvas_widgetbehavior =
    {
        canvas_behaviorGetRectangle,
        canvas_behaviorDisplaced,
        canvas_behaviorSelected,
        canvas_behaviorActivated,
        canvas_behaviorDeleted,
        canvas_behaviorVisibilityChanged,
        canvas_behaviorClicked,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_redrawGraphOnParent (t_glist *glist)
{  
    if (canvas_isMapped (glist)) {
    //
    if (canvas_canHaveWindow (glist)) {
    //
    t_gobj *y = NULL;
    t_linetraverser t;
    t_outconnect *connection = NULL;
    
    canvas_deleteGraphOnParentRectangle (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        gobj_visibilityChanged (y, glist, 0);
        gobj_visibilityChanged (y, glist, 1);
    }

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
        sys_vGui (".x%lx.c coords %lxLINE %d %d %d %d\n",
                        canvas_getView (glist),
                        connection,
                        t.tr_lineStartX,
                        t.tr_lineStartY,
                        t.tr_lineEndX,
                        t.tr_lineEndY);
    }
    
    canvas_drawGraphOnParentRectangle (glist);
    //
    }
    
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
        object_moveInletFirst (cast_object (glist), vinlet_getit (cast_pd (*mostRightObject)));
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
        object_moveOutletFirst (cast_object (glist), voutlet_getit (cast_pd (*mostRightObject)));
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

void canvas_bounds (t_glist *glist, t_float a, t_float b, t_float c, t_float d)
{
    if ((a == b) || (c == d)) { post_error (PD_TRANSLATE ("graph: invalid bounds")); }
    else {
    //
    glist->gl_valueLeft     = a;
    glist->gl_valueRight    = c;
    glist->gl_valueTop      = b;
    glist->gl_valueBottom   = d;
        
    canvas_redrawGraphOnParent (glist);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_ticksX (t_glist *glist, t_float pt, t_float i, t_float f)
{
    glist->gl_tickX.k_point     = pt;
    glist->gl_tickX.k_increment = i;
    glist->gl_tickX.k_period    = f;
    
    canvas_redrawGraphOnParent (glist);
}

void canvas_ticksY (t_glist *glist, t_float pt, t_float i, t_float f)
{
    glist->gl_tickY.k_point     = pt;
    glist->gl_tickY.k_increment = i;
    glist->gl_tickY.k_period    = f;
    
    canvas_redrawGraphOnParent (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_getGraphOnParentRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_glist *x = cast_glist (z);
    
    PD_ASSERT (pd_class (z) == canvas_class);
    
    int x1 = text_getPositionX (cast_object (x), glist);
    int y1 = text_getPositionY (cast_object (x), glist);
    int x2 = x1 + x->gl_graphWidth;
    int y2 = y1 + x->gl_graphHeight;

    *a = x1;
    *b = y1;
    *c = x2;
    *d = y2;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float canvas_positionToValueX (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueRight - glist->gl_valueLeft;
    t_float v = 0.0;
        
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

t_float canvas_positionToValueY (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueBottom - glist->gl_valueTop;
    t_float v = 0.0;
        
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

t_float canvas_valueToPositionX (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueRight - glist->gl_valueLeft;
    t_float v = 1.0;
    t_float x = 0.0;
    
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

t_float canvas_valueToPositionY (t_glist *glist, t_float f)
{
    t_float range = glist->gl_valueBottom - glist->gl_valueTop;
    t_float v = 1.0;
    t_float x = 0.0;
    
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

t_float canvas_deltaPositionToValueX (t_glist *glist, t_float f)
{ 
    return (f * (canvas_positionToValueX (glist, 1.0) - canvas_positionToValueX (glist, 0.0)));
}

t_float canvas_deltaPositionToValueY (t_glist *glist, t_float f)
{
    return (f * (canvas_positionToValueY (glist, 1.0) - canvas_positionToValueY (glist, 0.0)));
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
    int x1, y1, x2, y2;

    t_glist *x = cast_glist (z);
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnGetRectangle (z, glist, &x1, &y1, &x2, &y2); }
    else {
        canvas_getGraphOnParentRectangle (z, glist, &x1, &y1, &x2, &y2);
    }
    
    *a = x1;
    *b = y1;
    *c = x2;
    *d = y2;
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
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnSelected (z, glist, isSelected); }
    else {
        if (canvas_hasGraphOnParentTitle (x)) { 
            text_widgetBehavior.w_fnSelected (z, glist, isSelected);
        }

        sys_vGui (".x%lx.c itemconfigure GRAPH%lx -fill #%06x\n",
                        canvas_getView (glist),
                        (t_int)x,
                        (isSelected? COLOR_SELECTED : COLOR_NORMAL));
    }
}

static void canvas_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    t_glist *x = cast_glist (z);
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnActivated (z, glist, isActivated); }
    else {
        if (canvas_hasGraphOnParentTitle (x)) { 
            text_widgetBehavior.w_fnActivated (z, glist, isActivated);
        }
    }
}

static void canvas_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    t_glist *x = cast_glist (z);
    
    t_gobj *y = NULL;
    
    while (y = x->gl_graphics)  { canvas_removeObject (x, y); }
    
    if (!x->gl_isGraphOnParent) { text_widgetBehavior.w_fnDeleted (z, glist); }
    else {
        canvas_deleteLinesByObject (glist, cast_object (z));
    }
}

static void canvas_behaviorVisibilityChanged (t_gobj *gr, t_glist *parent_glist, int vis)
{
    t_glist *x = (t_glist *)gr;
    char tag[50];
    t_gobj *g;
    int x1, y1, x2, y2;
        /* ordinary subpatches: just act like a text object */
    if (!x->gl_isGraphOnParent)
    {
        text_widgetBehavior.w_fnVisibilityChanged(gr, parent_glist, vis);
        return;
    }

    if (vis && canvas_hasGraphOnParentTitle (x))
        boxtext_draw(boxtext_fetch(parent_glist, &x->gl_obj));
    canvas_behaviorGetRectangle(gr, parent_glist, &x1, &y1, &x2, &y2);
    if (!vis)
        boxtext_erase(boxtext_fetch(parent_glist, &x->gl_obj));

    sprintf(tag, "GRAPH%lx", (t_int)x);
    /*
    if (vis)
        canvas_drawInletsAndOutlets(parent_glist, &x->gl_obj,
            tag, 1, x1, y1, x2, y2);
    else canvas_eraseInletsAndOutlets(parent_glist, &x->gl_obj, tag); */
        /* if we look like a graph but have been moved to a toplevel,
        just show the bounding rectangle */
    if (x->gl_hasWindow)
    {
        if (vis)
        {
            sys_vGui(".x%lx.c create polygon\
 %d %d %d %d %d %d %d %d %d %d -tags [list %s graph] -fill #c0c0c0\n",
                canvas_getView(x->gl_parent),
                x1, y1, x1, y2, x2, y2, x2, y1, x1, y1, tag);
        }
        else
        {
            sys_vGui(".x%lx.c delete %s\n",
                canvas_getView(x->gl_parent), tag);
        }
        return;
    }
        /* otherwise draw (or erase) us as a graph inside another glist. */
    if (vis)
    {
        int i;
        t_float f;
        t_gobj *g;
        t_symbol *arrayname;
        t_garray *ga;
        /* char *ylabelanchor =
            (x->gl_ylabelx > 0.5*(x->gl_valueLeft + x->gl_valueRight) ? "w" : "e");
        char *xlabelanchor =
            (x->gl_xlabely > 0.5*(x->gl_valueTop + x->gl_valueBottom) ? "s" : "n"); */
            
            /* draw a rectangle around the graph */
        sys_vGui(".x%lx.c create line\
            %d %d %d %d %d %d %d %d %d %d -tags [list %s graph]\n",
            canvas_getView(x->gl_parent),
            x1, y1, x1, y2, x2, y2, x2, y1, x1, y1, tag);
        
            /* if there's just one "garray" in the graph, write its name
                along the top */
        for (i = (y1 < y2 ? y1 : y2)-1, g = x->gl_graphics; g; g = g->g_next)
            if (g->g_pd == garray_class &&
                !garray_getname((t_garray *)g, &arrayname))
        {
            i -= (int)font_getHostFontHeight(canvas_getFontSize(x));
            sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor nw\
             -font [::getFont %d] -tags [list %s label graph]\n",
             (long)canvas_getView(x), x1, i, arrayname->s_name,
                font_getHostFontSize(canvas_getFontSize(x)), tag);
        }
        
            /* draw ticks on horizontal borders.  If lperb field is
            zero, this is disabled. */
        if (x->gl_tickX.k_period)
        {
            t_float upix, lpix;
            if (y2 < y1)
                upix = y1, lpix = y2;
            else upix = y2, lpix = y1;
            for (i = 0, f = x->gl_tickX.k_point;
                f < 0.99 * x->gl_valueRight + 0.01*x->gl_valueLeft; i++,
                    f += x->gl_tickX.k_increment)
            {
                int tickpix = (i % x->gl_tickX.k_period ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)canvas_valueToPositionX(x, f), (int)upix,
                    (int)canvas_valueToPositionX(x, f), (int)upix - tickpix, tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)canvas_valueToPositionX(x, f), (int)lpix,
                    (int)canvas_valueToPositionX(x, f), (int)lpix + tickpix, tag);
            }
            for (i = 1, f = x->gl_tickX.k_point - x->gl_tickX.k_increment;
                f > 0.99 * x->gl_valueLeft + 0.01*x->gl_valueRight;
                    i++, f -= x->gl_tickX.k_increment)
            {
                int tickpix = (i % x->gl_tickX.k_period ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)canvas_valueToPositionX(x, f), (int)upix,
                    (int)canvas_valueToPositionX(x, f), (int)upix - tickpix, tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)canvas_valueToPositionX(x, f), (int)lpix,
                    (int)canvas_valueToPositionX(x, f), (int)lpix + tickpix, tag);
            }
        }

            /* draw ticks in vertical borders*/
        if (x->gl_tickY.k_period)
        {
            t_float ubound, lbound;
            if (x->gl_valueBottom < x->gl_valueTop)
                ubound = x->gl_valueTop, lbound = x->gl_valueBottom;
            else ubound = x->gl_valueBottom, lbound = x->gl_valueTop;
            for (i = 0, f = x->gl_tickY.k_point;
                f < 0.99 * ubound + 0.01 * lbound;
                    i++, f += x->gl_tickY.k_increment)
            {
                int tickpix = (i % x->gl_tickY.k_period ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x1, (int)canvas_valueToPositionY(x, f), 
                    x1 + tickpix, (int)canvas_valueToPositionY(x, f), tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x2, (int)canvas_valueToPositionY(x, f), 
                    x2 - tickpix, (int)canvas_valueToPositionY(x, f), tag);
            }
            for (i = 1, f = x->gl_tickY.k_point - x->gl_tickY.k_increment;
                f > 0.99 * lbound + 0.01 * ubound;
                    i++, f -= x->gl_tickY.k_increment)
            {
                int tickpix = (i % x->gl_tickY.k_period ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x1, (int)canvas_valueToPositionY(x, f), 
                    x1 + tickpix, (int)canvas_valueToPositionY(x, f), tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x2, (int)canvas_valueToPositionY(x, f), 
                    x2 - tickpix, (int)canvas_valueToPositionY(x, f), tag);
            }
        }
        /*
        for (i = 0; i < x->gl_nxlabels; i++)
            sys_vGui(".x%lx.c create text\
 %d %d -text {%s} -font [::getFont %d] -anchor %s -tags [list %s label graph]\n",
                canvas_getView(x),
                (int)canvas_valueToPositionX(x, atof(x->gl_xlabel[i]->s_name)),
                (int)canvas_valueToPositionY(x, x->gl_xlabely),
                x->gl_xlabel[i]->s_name,
                     canvas_getFontSize(x), xlabelanchor, tag);

    
        for (i = 0; i < x->gl_nylabels; i++)
            sys_vGui(".x%lx.c create text\
 %d %d -text {%s} -font [::getFont %d] -anchor %s -tags [list %s label graph]\n",
                canvas_getView(x),
                (int)canvas_valueToPositionX(x, x->gl_ylabelx),
                (int)canvas_valueToPositionY(x, atof(x->gl_ylabel[i]->s_name)),
                x->gl_ylabel[i]->s_name,
                canvas_getFontSize(x), ylabelanchor, tag);
        */
            /* draw contents of graph as glist */
        for (g = x->gl_graphics; g; g = g->g_next)
            gobj_visibilityChanged(g, x, 1);
    }
    else
    {
        sys_vGui(".x%lx.c delete %s\n",
            canvas_getView(x->gl_parent), tag);
        for (g = x->gl_graphics; g; g = g->g_next)
            gobj_visibilityChanged(g, x, 0);
    }
}

static int canvas_behaviorClicked (t_gobj *z, t_glist *glist, int xpix, int ypix, int shift, int ctrl, int alt, int dbl, int doit)
{
    t_glist *x = (t_glist *)z;
    t_gobj *y;
    int clickreturned = 0;
    if (!x->gl_isGraphOnParent)
        return (text_widgetBehavior.w_fnClicked(z, glist,
            xpix, ypix, shift, ctrl, alt, dbl, doit));
    else if (x->gl_hasWindow)
        return (0);
    else
    {
        for (y = x->gl_graphics; y; y = y->g_next)
        {
            int x1, y1, x2, y2;
                /* check if the object wants to be clicked */
            if (gobj_hit(y, x, xpix, ypix, &x1, &y1, &x2, &y2)
                &&  (clickreturned = gobj_click(y, x, xpix, ypix,
                    shift, ctrl, alt, 0, doit)))
                        break;
        }
        if (!doit)
        {
            if (y)
                canvas_setCursorType(canvas_getView(x), clickreturned);
            else canvas_setCursorType(canvas_getView(x), CURSOR_NOTHING);
        }
        return (clickreturned); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
