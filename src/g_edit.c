
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EDIT_GRIP_SIZE      5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_gobj *glist_objectHit (t_glist *glist, int a, int b, t_rectangle *r)
{
    t_gobj *y = NULL;
    t_gobj *object = NULL;
    
    t_rectangle t;
    
    rectangle_set (r, 0, 0, 0, 0);
    
    if (glist_objectGetNumberOfSelected (glist) > 1) {
    //
    t_selection *s = NULL;
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
    //
    if (gobj_hit (selection_getObject (s), glist, a, b, EDIT_GRIP_SIZE, &t)) {
        rectangle_setCopy (r, &t);
        object = selection_getObject (s); 
    }
    //
    }
    //
    }
    
    if (!object) {
    //
    int k = -PD_INT_MAX;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_hit (y, glist, a, b, EDIT_GRIP_SIZE, &t)) {
            if (rectangle_getTopLeftX (&t) > k) {
                rectangle_setCopy (r, &t);
                object = y; k = rectangle_getTopLeftX (&t);
            }
        }
    }
    //
    }

    return object;
}

static void glist_updateCursor (t_glist *glist, int type)
{
    static t_glist *lastGlist = NULL;           /* Static. */
    static int lastType = CURSOR_NOTHING;       /* Static. */
    static char *cursors[] =                    /* Static. */
        {
            "left_ptr",             // CURSOR_NOTHING
            "hand2",                // CURSOR_CLICK
            "hand2",                // CURSOR_OVER
            "sb_v_double_arrow",    // CURSOR_THICKEN
            "circle",               // CURSOR_CONNECT
            "sb_h_double_arrow"     // CURSOR_RESIZE
        };
    
    if (type == CURSOR_CLICK) { type = CURSOR_NOTHING; }
    
    type = PD_CLAMP (type, CURSOR_NOTHING, CURSOR_RESIZE);
    
    PD_ASSERT (glist_hasWindow (glist));
    
    if (lastGlist != glist || lastType != type) {
    //
    sys_vGui ("%s configure -cursor %s\n", glist_getTagAsString (glist), cursors[type]);
    //
    }
    
    lastType = type; lastGlist = glist;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static void glist_actionResizeBox (t_glist *glist, t_gobj *y, int width)
{
    int w = (int)(width / font_getHostFontWidth (glist_getFontSize (glist)));
    
    gobj_visibilityChanged (y, glist, 0);
    object_setWidth (cast_object (y), PD_MAX (1, w));
    glist_updateLinesForObject (glist, cast_object (y));
    gobj_visibilityChanged (y, glist, 1);
    glist_setDirty (glist, 1);
}

static void glist_actionResizeGraph (t_glist *glist, t_gobj *y, int deltaX, int deltaY)
{
    if (pd_class (y) != canvas_class) { PD_BUG; }
    else {
    //
    t_rectangle *r = glist_getGraphGeometry (cast_glist (y));
    
    int w = rectangle_getWidth (r) + deltaX;
    int h = rectangle_getHeight (r) + deltaY;
    
    gobj_visibilityChanged (y, glist, 0);
    rectangle_setWidth (r, PD_MAX (EDIT_GRIP_SIZE, w));
    rectangle_setHeight (r, PD_MAX (EDIT_GRIP_SIZE, h));
    glist_updateLinesForObject (glist, cast_object (y));
    glist_updateRectangle (cast_glist (y));
    gobj_visibilityChanged (y, glist, 1);
    glist_setDirty (glist, 1);
    //
    }
}

static void glist_actionResize (t_glist *glist, int a, int b)
{
    t_rectangle r;
    
    /* Points below are the top left coordinates of the resized object. */
    
    int startX = drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int startY = drag_getStartY (editor_getDrag (glist_getEditor (glist)));
    
    t_gobj *y  = glist_objectHit (glist, startX, startY, &r);

    if (y && cast_objectIfConnectable (y)) {
        if (object_isViewedAsBox (cast_object (y))) { glist_actionResizeBox (glist, y, a - startX); }
        else {
            int c = rectangle_getBottomRightX (&r);
            int d = rectangle_getBottomRightY (&r);
            glist_actionResizeGraph (glist, y, a - c, b - d);
        }
    }
}

void glist_actionEnd (t_glist *glist, int a, int b)
{
    t_editor *e = glist_getEditor (glist);
    
    int action = editor_getAction (e);
    
    if (action == ACTION_CONNECT)     { glist_makeLineEnd (glist, a, b);    }
    else if (action == ACTION_REGION) { glist_selectLassoEnd (glist, a, b); }
    else if (action == ACTION_MOVE)   {
    //
    if (!drag_moved (editor_getDrag (e))) {
    if (glist_objectGetNumberOfSelected (glist) == 1) {
        gobj_activated (selection_getObject (editor_getSelection (e)), glist, 1);
    }
    }
    //
    }
    
    editor_resetAction (e);
}

void glist_action (t_glist *glist, int a, int b, int m)
{
    t_editor *e = glist_getEditor (glist);
    
    t_box *box  = editor_getSelectedBox (e);
    int startX  = drag_getStartX (editor_getDrag (e));
    int startY  = drag_getStartY (editor_getDrag (e));
    int endX    = drag_getEndX (editor_getDrag (e));
    int endY    = drag_getEndY (editor_getDrag (e));
    
    switch (editor_getAction (e)) {
    //
    case ACTION_MOVE    : editor_selectionDeplace (e);                          break;
    case ACTION_CONNECT : glist_makeLineBegin (glist, a, b);                    break;
    case ACTION_REGION  : glist_selectLassoBegin (glist, a, b);                 break;
    case ACTION_PASS    : editor_motionProceed (e, a - endX, b - endY, m);      break; 
    case ACTION_DRAG    : box_mouse (box, a - startX, b - startY, BOX_DRAG);    break;
    case ACTION_RESIZE  : glist_actionResize (glist, a, b);                     break;
    default             : PD_BUG;
    //
    }
    
    drag_set (editor_getDrag (e), a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_popUp (t_glist *glist, t_gobj *y, int a, int b)
{
    int canProperties = (!y || (y && class_hasPropertiesFunction (pd_class (y))));
    int canOpen = (y && class_hasMethod (pd_class (y), sym_open));
    int canHelp = (y != NULL);
    
    if (y && (pd_class (y) == canvas_class)) {
    //
    if (glist_isAbstraction (cast_glist (y))) { canProperties = 0; }
    //
    }
    
    if (glist_isAbstraction (glist)) { canProperties = 0; }
    
    glist_deselectAll (glist);
    
    sys_vGui ("::ui_menu::showPopup %s %d %d %d %d %d\n",
                    glist_getTagAsString (glist), 
                    a, 
                    b, 
                    canProperties, 
                    canOpen, 
                    canHelp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_mouseOverEditDrag (t_glist *glist, t_box *box, int a, int b, int flag, t_rectangle *r)
{
    t_editor *e = glist_getEditor (glist);
    int t1 = rectangle_getTopLeftX (r);
    int t2 = rectangle_getTopLeftY (r);
    box_mouse (box, a - t1, b - t2, flag);
    editor_startAction (e, ACTION_DRAG, t1, t2);
}

static void glist_mouseOverEditShift (t_glist *glist, t_gobj *y, int a, int b, int clicked, t_rectangle *r)
{
    if (clicked) {
    //
    t_editor *e = glist_getEditor (glist);
    t_box *box  = editor_getSelectedBox (e);
    
    int isText = cast_objectIfConnectable (y) && box && (box == box_fetch (glist, cast_object (y)));
    
    if (!isText) { glist_objectSwapSelected (glist, y); }
    else {
        glist_mouseOverEditDrag (glist, box, a, b, BOX_SHIFT, r);
    }
    //
    }
}

static int glist_mouseOverEditResize (t_glist *glist, t_gobj *y, int a, int b, int clicked, t_rectangle *r)
{
    t_editor *e = glist_getEditor (glist);
    
    int k = 0;
    int resizable = 0;
    
    if (cast_objectIfConnectable (y)) {
    //
    resizable |= object_isViewedAsBox (cast_object (y));
    resizable |= ((pd_class (y) == canvas_class) && !glist_isAbstraction (cast_glist (y)));
    
    if (resizable) {
        int w = rectangle_getBottomRightX (r) - EDIT_GRIP_SIZE;
        int h = rectangle_getBottomRightY (r) - (object_isViewedAsBox (cast_object (y)) ? EDIT_GRIP_SIZE : 0);
        if (a > w && b < h) { k = 1; }
    }
    //
    }
    
    if (k) {
    
        if (!clicked) { glist_updateCursor (glist, CURSOR_RESIZE); }
        else {
            int t1 = rectangle_getTopLeftX (r);
            int t2 = rectangle_getTopLeftY (r);
            glist_objectSelectIfNotSelected (glist, y);
            editor_startAction (e, ACTION_RESIZE, t1, t2);
        }
    }
    
    return k;
}

static int glist_mouseOverEditLine (t_glist *glist, t_gobj *y, int a, int b, int clicked, t_rectangle *r)
{
    t_editor *e = glist_getEditor (glist);
    
    int n, outlet = -1;
    
    if (cast_objectIfConnectable (y)) {
    if ((n = object_getNumberOfOutlets (cast_object (y)))) {
    if ((b >= rectangle_getBottomRightY (r) - EDIT_GRIP_SIZE)) {
    //
    int closest = inlet_closest (a, n, r);
    int hotspot = inlet_middle (closest, n, r);

    if (PD_ABS (a - hotspot) < EDIT_GRIP_SIZE) {
    
        outlet = closest;
        
        if (!clicked) { glist_updateCursor (glist, CURSOR_CONNECT); }
        else {
            int t1 = hotspot;
            int t2 = rectangle_getBottomRightY (r);
            glist_drawTemporary (glist, t1, t2, object_isSignalOutlet (cast_object (y), outlet));
            editor_startAction (e, ACTION_CONNECT, t1, t2);
        }
    }
    //
    }
    }
    }
    
    return (outlet != -1);
}

static void glist_mouseOverEditMove (t_glist *glist, t_gobj *y, int a, int b, int m, t_rectangle *r)
{
    t_editor *e = glist_getEditor (glist);
    t_box *box  = editor_getSelectedBox (e);
    
    int isText = cast_objectIfConnectable (y) && box && (box == box_fetch (glist, cast_object (y)));
    
    if (!isText) { glist_objectSelectIfNotSelected (glist, y); editor_startAction (e, ACTION_MOVE, a, b); }
    else {
        glist_mouseOverEditDrag (glist, box, a, b, (m & MODIFIER_DOUBLE) ? BOX_DOUBLE : BOX_DOWN, r);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int glist_mouseOverEdit (t_glist *glist, int a, int b, int m, int clicked)
{
    t_rectangle r;
    
    t_gobj *y = glist_objectHit (glist, a, b, &r);              /* With a tolerance zone up and down. */
        
    if (y) {
    //
    t_rectangle dummy; int over = gobj_hit (y, glist, a, b, 0, &dummy);             /* Strictly over. */
    
    if (!over) { return glist_mouseOverEditLine (glist, y, a, b, clicked, &r); }
    else {
        if (m & MODIFIER_RIGHT)      { glist_popUp (glist, y, a, b); }
        else if (m & MODIFIER_SHIFT) { glist_mouseOverEditShift (glist, y, a, b, clicked, &r); }
        else {
            if (glist_mouseOverEditResize (glist, y, a, b, clicked, &r))    { }
            else if (glist_mouseOverEditLine (glist, y, a, b, clicked, &r)) { }
            else if (clicked) { glist_mouseOverEditMove (glist, y, a, b, m, &r); } 
            else { 
                glist_updateCursor (glist, CURSOR_NOTHING);
            }
        }
        
        return 1;
    }
    //
    }
    
    return 0;
}

static void glist_mouseOverRun (t_glist *glist, int a, int b, int m, int clicked)
{
    t_gobj *y = NULL;
    
    int k = 0;
        
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_rectangle r;
    
    if (gobj_hit (y, glist, a, b, 0, &r)) {
    
        t_mouse t;
        
        t.m_x       = a;
        t.m_y       = b;
        t.m_shift   = (m & MODIFIER_SHIFT);
        t.m_ctrl    = (m & MODIFIER_CTRL);
        t.m_alt     = (m & MODIFIER_ALT);
        t.m_dbl     = (m & MODIFIER_DOUBLE);
        t.m_clicked = clicked;
    
        k = gobj_mouse (y, glist, &t);
                
        if (k) { break; }
    }
    //
    }
    
    if (!clicked) {
        if (y && k) { glist_updateCursor (glist, k); }
        else {
            glist_updateCursor (glist, CURSOR_NOTHING);
        }
    }
}

static int glist_mouseHitLines (t_glist *glist, int a, int b, int clicked)
{
    t_outconnect *connection = NULL;
    t_traverser t;
        
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
        if (cord_hit (traverser_getCord (&t), a, b)) {
            if (clicked) { glist_lineSelect (glist, &t); } return 1;
        }
    }
    
    return 0;
}

static void glist_mouseLasso (t_glist *glist, int a, int b, int m)
{
    int recreated = 0;
    
    if (!(m & MODIFIER_SHIFT)) { recreated = glist_deselectAll (glist); }
    
    if (!recreated) {
        glist_drawLasso (glist, a, b); editor_startAction (glist_getEditor (glist), ACTION_REGION, a, b);
    }
}

void glist_mouse (t_glist *glist, int a, int b, int m, int clicked)
{
    t_editor *e = glist_getEditor (glist);
    
    int hasShift     = (m & MODIFIER_SHIFT);
    int isRightClick = (m & MODIFIER_RIGHT);
    int isRunMode    = (m & MODIFIER_CTRL) || (!glist_hasEditMode (glist));
    
    if (clicked) { editor_motionReset (e); }

    PD_ASSERT (!editor_hasAction (e));
    
    if (isRunMode && !isRightClick) { glist_mouseOverRun (glist, a, b, m, clicked); }
    else if (glist_mouseOverEdit (glist, a, b, m, clicked)) { } 
    else {
    //
    if (isRightClick)    { glist_popUp (glist, NULL, a, b); }
    else if (!isRunMode) {
        if (!hasShift && glist_mouseHitLines (glist, a, b, clicked)) { }
        else {
            if (clicked) { glist_mouseLasso (glist, a, b, m); }
        }
    }
    
    glist_updateCursor (glist, CURSOR_NOTHING);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_key (t_glist *glist, t_keycode n, t_symbol *s)
{
    t_editor *e = glist_getEditor (glist);
    
    if (editor_getAction (e) == ACTION_MOVE) { editor_resetAction (e); }
    
    if (editor_hasSelectedBox (e)) {
        box_key (editor_getSelectedBox (e), n, s);
        if (editor_hasSelectedBoxDirty (e)) {
            glist_updateLinesForObject (glist, box_getObject (editor_getSelectedBox (e)));
            glist_setDirty (glist, 1);
        }
        
    } else if (s == sym_Delete || s == sym_BackSpace) {
        if (editor_hasSelectedLine (e))   { glist_lineDeleteSelected (glist); }
        else if (editor_hasSelection (e)) {
            glist_objectRemoveSelected (glist);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
