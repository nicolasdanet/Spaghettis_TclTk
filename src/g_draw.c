
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

void glist_behaviorVisibilityChanged   (t_gobj *, t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_updateTitle (t_glist *glist)
{
    if (glist_hasWindow (glist)) {

        sys_vGui ("::ui_patch::setTitle %s {%s} {%s} %d\n",  // --
                        glist_getTagAsString (glist),
                        environment_getDirectoryAsString (glist_getEnvironment (glist)),
                        glist_getName (glist)->s_name,
                        glist_getDirty (glist));
    }
}

void glist_updateWindow (t_glist *glist)
{
    if (glist_isWindowable (glist) && glist_isOnScreen (glist)) { 
        glist_windowMapped (glist, 0);
        glist_windowMapped (glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_updateGraphOnParent (t_glist *glist)
{  
    if (glist_isOnScreen (glist)) {
    //
    if (glist_hasWindow (glist)) { glist_updateWindow (glist); }
    else {
        PD_ASSERT (glist_isParentOnScreen (glist));
        glist_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 0); 
        glist_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
    }
    //
    }
}

/* The rectangle drawn onto the parent. */

void glist_updateRectangleOnParent (t_glist *glist)
{   
    int isSelected = glist_isSelected (glist);
    int hasWindow  = glist_hasWindow (glist);
    int color      = hasWindow ? COLOR_MASKED : (isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    
    t_glist *view  = glist_getView (glist_getParent (glist));
    
    sys_vGui ("%s.c itemconfigure %lxGRAPH -fill #%06x\n",
                    glist_getTagAsString (view),
                    glist,
                    color);
}

/* The dashed rectangle drawn to show the area. */

void glist_updateRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
    //
    sys_vGui ("%s.c delete RECTANGLE\n", glist_getTagAsString (glist));
    
    glist_drawRectangle (glist);
    //
    }
    //
    }
}

void glist_updateLasso (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c coords LASSO %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    drag_getStartX (editor_getDrag (glist_getEditor (glist))),
                    drag_getStartY (editor_getDrag (glist_getEditor (glist))),
                    a,
                    b);
    //
    }
    //
    }
}

void glist_updateTemporary (t_glist *glist, int a, int b, int c, int d)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c coords TEMPORARY %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    c,
                    d);
    //
    }
    //
    }
}

void glist_updateLineSelected (t_glist *glist, int isSelected)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (isSelected) {
    
    sys_vGui ("%s.c itemconfigure %lxLINE -fill blue\n",
                    glist_getTagAsString (glist),
                    editor_getSelectedLineConnection (glist_getEditor (glist)));
                    
    } else {
    
    sys_vGui ("%s.c itemconfigure %lxLINE -fill black\n",
                    glist_getTagAsString (glist),
                    editor_getSelectedLineConnection (glist_getEditor (glist)));
    }
    //
    }
}

void glist_updateLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c coords %lxLINE %d %d %d %d\n",
                    glist_getTagAsString (glist),
                    cord_getConnection (c),
                    cord_getStartX (c),
                    cord_getStartY (c),
                    cord_getEndX (c),
                    cord_getEndY (c));

    sys_vGui ("%s.c itemconfigure %lxLINE -width %d\n",
                    glist_getTagAsString (glist),
                    cord_getConnection (c),
                    cord_isSignal (c) ? 2 : 1);
    //
    }
    //
    }
}

void glist_updateLinesForObject (t_glist *glist, t_object *o)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if (traverser_getSource (&t) == o || traverser_getDestination (&t) == o) {
        glist_updateLine (glist, traverser_getCord (&t));
    }
    //
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* For compatibility several names can be rendered. */

static void glist_drawArrayName (t_glist *glist)
{
    if (glist_isArray (glist)) {
    //
    int isSelected = glist_isSelected (glist);
    int color      = isSelected ? COLOR_SELECTED : COLOR_NORMAL;
    
    t_glist *view  = glist_getView (glist_getParent (glist));
    
    t_rectangle r;

    glist_getRectangleOnParent (glist, &r);

    {
        int a = rectangle_getTopLeftX (&r);
        int b = rectangle_getTopLeftY (&r);
        int k = font_getHostFontSize (glist_getFontSize (view));
        int h = (int)font_getHostFontHeight (glist_getFontSize (view));
        int i = 0;
        
        t_gobj *y = NULL;
        
        for (y = glist->gl_graphics; y; y = y->g_next) {
        //
        if (pd_class (y) == garray_class) {
        //
        sys_vGui ("%s.c create text %d %d -text {%s}"   // --
                        " -anchor nw"
                        " -font [::getFont %d]"         // --
                        " -fill #%06x"
                        " -tags %lxGRAPH\n",
                        glist_getTagAsString (view),
                        a,
                        b - (++i) * h,
                        garray_getName ((t_garray *)y)->s_name,
                        k,
                        color,
                        glist);
        //
        }
        //
        }
    }
    //
    }
}

void glist_drawRectangleOnParent (t_glist *glist)
{
    int isSelected   = glist_isSelected (glist);
    int hasWindow    = glist_hasWindow (glist);
    int color        = hasWindow ? COLOR_MASKED : (isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    const char *type = hasWindow ? "polygon" : "line";
    
    t_glist *view = glist_getView (glist_getParent (glist));
    
    t_rectangle r;
    
    glist_getRectangleOnParent (glist, &r);
    
    {
        int a = rectangle_getTopLeftX (&r);
        int b = rectangle_getTopLeftY (&r);
        int c = rectangle_getBottomRightX (&r);
        int d = rectangle_getBottomRightY (&r);
        
        sys_vGui ("%s.c create %s %d %d %d %d %d %d %d %d %d %d"
                        " -fill #%06x"
                        " -tags %lxGRAPH\n",
                        glist_getTagAsString (view),
                        type,
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
                        color,
                        glist);
    }
    
    if (!hasWindow) { glist_drawArrayName (glist); }
}

void glist_drawRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
    //
    int a = rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    int b = rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    int c = rectangle_getBottomRightX (glist_getGraphGeometry (glist));
    int d = rectangle_getBottomRightY (glist_getGraphGeometry (glist));
    
    sys_vGui ("%s.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"  // --
                    " -fill #%06x"
                    " -tags RECTANGLE\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    c,
                    b,
                    c,
                    d,
                    a,
                    d,
                    a,
                    b, 
                    COLOR_GOP);
    }
    //
    }
}

void glist_drawLasso (t_glist *glist, int a, int b)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c create rectangle %d %d %d %d -tags LASSO\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    a,
                    b);
    //
    }
    //
    }
}

void glist_drawTemporary (t_glist *glist, int a, int b, int isSignal)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c create line %d %d %d %d -width %d -tags TEMPORARY\n",
                    glist_getTagAsString (glist),
                    a,
                    b,
                    a,
                    b,
                    isSignal ? 2 : 1);
    //
    }
    //
    }
}

void glist_drawLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    glist_getTagAsString (glist),
                    cord_getStartX (c),
                    cord_getStartY (c),
                    cord_getEndX (c),
                    cord_getEndY (c),
                    cord_isSignal (c) ? 2 : 1,
                    cord_getConnection (c));
    //
    }
    //
    }
}

void glist_drawAllLines (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) { glist_drawLine (glist, traverser_getCord (&t)); }
    //
    }
    //
    }
}

static void glist_drawAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        t_object *o = NULL;
        if ((o = cast_objectIfConnectable (y)) && object_isComment (o)) {
            box_draw (box_fetch (glist, o));
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_eraseRectangleOnParent (t_glist *glist)
{
    t_glist *view = glist_getView (glist_getParent (glist));
    
    sys_vGui ("%s.c delete %lxGRAPH\n", glist_getTagAsString (view), glist);
}

void glist_eraseLasso (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete LASSO\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

void glist_eraseTemporary (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete TEMPORARY\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

void glist_eraseLine (t_glist *glist, t_cord *c)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete %lxLINE\n", glist_getTagAsString (glist), cord_getConnection (c));
    //
    }
    //
    }
}

static void glist_eraseAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete COMMENTBAR\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_windowEdit (t_glist *glist, int isEditMode)
{
    if (isEditMode != glist_hasEditMode (glist)) {
    //
    glist_setEditMode (glist, isEditMode);
    
    if (isEditMode) { glist_drawAllCommentBars (glist); }
    else {
        glist_deselectAll (glist); glist_eraseAllCommentBars (glist);
    }
    
    if (glist_isOnScreen (glist)) {
        sys_vGui ("::ui_patch::setEditMode %s %d\n", glist_getTagAsString (glist), glist_hasEditMode (glist));
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void glist_windowMappedEraseContent (t_glist *glist)
{
    sys_vGui ("%s.c delete all\n", glist_getTagAsString (glist)); 
    
    glist_setMapped (glist, 0);
}

static void glist_windowMappedDrawContent (t_glist *glist)
{
    t_gobj *y = NULL;
    t_selection *s = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, glist, 1); }
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
        gobj_selected (selection_getObject (s), glist, 1);
    }

    glist_setMapped (glist, 1);
    glist_drawAllLines (glist);
    glist_drawRectangle (glist);
}

/* When a window is put or removed from screen. */
/* It can result from normal, withdrawn or iconic states. */

void glist_windowMapped (t_glist *glist, int isMapped)
{
    if (isMapped != glist_isOnScreen (glist)) {
    //
    PD_ASSERT (glist_hasWindow (glist));
    
    if (isMapped) { glist_windowMappedDrawContent (glist); }
    else {
        glist_windowMappedEraseContent (glist);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_windowOpen (t_glist *glist)
{
    if (glist_hasWindow (glist)) { sys_vGui ("::bringToFront %s\n", glist_getTagAsString (glist)); }
    else {
    //
    t_rectangle *t = glist_getWindowGeometry (glist);
    
    sys_vGui ("::ui_patch::create %s %d %d +%d+%d %d\n",    // --
                    glist_getTagAsString (glist),
                    rectangle_getWidth (t),
                    rectangle_getHeight (t),
                    rectangle_getTopLeftX (t),
                    rectangle_getTopLeftY (t),
                    glist_hasEditMode (glist));
                    
    glist_setWindow (glist, 1); glist_updateTitle (glist);
    //
    }
}

void glist_windowClose (t_glist *glist)
{
    if (glist_hasWindow (glist)) { 
    //
    glist_deselectAll (glist);
    
    if (glist_isOnScreen (glist)) { glist_windowMapped (glist, 0); }

    sys_vGui ("destroy %s\n", glist_getTagAsString (glist));
    
    /* If it is a GOP opened in its own window it needs to be redrawn on parent. */
    /* Note that the window state influence the way it is rendered. */
    
    if (glist_isGraphOnParent (glist)) {
    if (glist_hasParent (glist)) {
        t_glist *t = glist_getParent (glist);
        if (!glist_isDeleting (t)) {
            if (glist_isOnScreen (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 0); }
            glist_setWindow (glist, 0);
            if (glist_isOnScreen (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 1); }
        }
    }
    }
    
    glist_setWindow (glist, 0);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
