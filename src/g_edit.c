
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
        break;
    }
    //
    }
    //
    }
    
    if (!object) {
    //
    int i, n = glist_objectGetNumberOf (glist);
    
    for (i = n - 1; i >= 0; i--) {
        y = glist_objectGetAt (glist, i);
        if (gobj_hit (y, glist, a, b, EDIT_GRIP_SIZE, &t)) {
            rectangle_setCopy (r, &t);
            object = y;
            break;
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
    gui_vAdd ("%s configure -cursor %s\n", glist_getTagAsString (glist), cursors[type]);
    //
    }
    
    lastType = type; lastGlist = glist;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_makeLineProceed (t_glist *glist, int a, int b, int end)
{
    t_rectangle r1;
    t_rectangle r2;
    
    t_gobj *t1 = drag_getObject (editor_getDrag (glist_getEditor (glist)));
    t_gobj *t2 = glist_objectHit (glist, a, b, &r2);
    
    if (end) { glist_eraseTemporary (glist); }
    else {
        glist_updateTemporary (glist, a, b);
    }

    if (t1 && t2) {
    //
    t_object *o1 = cast_objectIfConnectable (t1);
    t_object *o2 = cast_objectIfConnectable (t2);
    
    if (o1 && o2 && o1 != o2) {
    //
    int numberOfOutlets = object_getNumberOfOutlets (o1);
    int numberOfInlets  = object_getNumberOfInlets (o2);
    
    gobj_getRectangle (t1, glist, &r1);
    
    if (numberOfOutlets && numberOfInlets) {
    //
    int k  = drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int k1 = inlet_getClosest (k, numberOfOutlets, &r1);
    int k2 = inlet_getClosest (a, numberOfInlets, &r2);

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
    gobj_visibilityChanged (y, glist, 1);
    glist_updateLinesForObject (glist, cast_object (y));
    glist_setDirty (glist, 1);
}

static void glist_actionResizeGraph (t_glist *glist, t_gobj *y, int deltaX, int deltaY)
{
    if (!gobj_isCanvas (y)) { PD_BUG; }
    else {
    //
    t_rectangle *r = glist_getGraphGeometry (cast_glist (y));
    
    int w = rectangle_getWidth (r) + deltaX;
    int h = rectangle_getHeight (r) + deltaY;
    
    gobj_visibilityChanged (y, glist, 0);
    rectangle_setWidth (r, PD_MAX (EDIT_GRIP_SIZE, w));
    rectangle_setHeight (r, PD_MAX (EDIT_GRIP_SIZE, h));
    glist_updateRectangle (cast_glist (y));
    gobj_visibilityChanged (y, glist, 1);
    glist_updateLinesForObject (glist, cast_object (y));
    glist_setDirty (glist, 1);
    //
    }
}

static void glist_actionResize (t_glist *glist, int a, int b)
{
    t_gobj *y = drag_getObject (editor_getDrag (glist_getEditor (glist)));
    
    if (y && cast_objectIfConnectable (y)) {
    //
    t_rectangle r;
    
    gobj_getRectangle (y, glist, &r);
    
    if (object_isViewedAsBox (cast_object (y))) {
        glist_actionResizeBox (glist, y, a - rectangle_getTopLeftX (&r));
        
    } else {
        int c = rectangle_getBottomRightX (&r);
        int d = rectangle_getBottomRightY (&r);
        glist_actionResizeGraph (glist, y, a - c, b - d);
    }
    //
    }
}

void glist_actionEnd (t_glist *glist, int a, int b)
{
    t_editor *e = glist_getEditor (glist);
    
    int action = editor_getAction (e);
    
    if (action == ACTION_LINE)        { glist_makeLineEnd (glist, a, b);    }
    else if (action == ACTION_SIGNAL) { glist_makeLineEnd (glist, a, b);    }
    else if (action == ACTION_REGION) { glist_selectLassoEnd (glist, a, b); }
    else if (action == ACTION_RESIZE) { glist_redrawRequired (glist); }
    else if (action == ACTION_MOVE)   {
    //
    if (!drag_hasMoved (editor_getDrag (e))) {
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
    
    switch (editor_getAction (e)) {
    //
    case ACTION_MOVE    : editor_selectionDeplace (e);                          break;
    case ACTION_LINE    :
    case ACTION_SIGNAL  : glist_makeLineBegin (glist, a, b);                    break;
    case ACTION_REGION  : glist_selectLassoBegin (glist, a, b);                 break;
    case ACTION_PASS    : editor_motionProceed (e, a, b, m);                    break;
    case ACTION_DRAG    : box_mouse (box, a - startX, b - startY, BOX_DRAG);    break;
    case ACTION_RESIZE  : glist_actionResize (glist, a, b);                     break;
    default             : PD_BUG;
    //
    }
    
    drag_setEnd (editor_getDrag (e), a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_popUp (t_glist *glist, t_gobj *y, int a, int b)
{
    int canProperties = (!y || (y && class_hasPropertiesFunction (pd_class (y))));
    int canOpen       = (y && gobj_isCanvas (y));
    int canHelp       = (y != NULL);
    int canObject     = (glist_hasEditMode (glist));
    int canOrder      = (glist_objectGetNumberOfSelected (glist) != 0);
    
    if (y && gobj_isCanvas (y)) {
    //
    if (glist_isAbstraction (cast_glist (y))) { canProperties = 0; }
    //
    }
    
    if (glist_isAbstraction (glist)) { canProperties = 0; }
    
    gui_vAdd ("::ui_menu::showPopup %s %d %d %d %d %d %d %d\n",
                    glist_getTagAsString (glist), 
                    a, 
                    b, 
                    canProperties, 
                    canOpen, 
                    canHelp,
                    canObject,
                    canOrder);
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
    editor_startAction (e, ACTION_DRAG, t1, t2, NULL);
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
    resizable |= (gobj_isCanvas (y) && !glist_isAbstraction (cast_glist (y)));
    
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
            glist_objectSelectIfNotSelected (glist, y);
            editor_startAction (e, ACTION_RESIZE, a, b, y);
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
    int closest = inlet_getClosest (a, n, r);
    int hotspot = inlet_getMiddle (closest, n, r);

    if (PD_ABS (a - hotspot) < EDIT_GRIP_SIZE) {
    
        outlet = closest;
        
        if (!clicked) { glist_updateCursor (glist, CURSOR_CONNECT); }
        else {
            int t1 = hotspot;
            int t2 = rectangle_getBottomRightY (r);
            int action = object_isSignalOutlet (cast_object (y), outlet) ? ACTION_SIGNAL : ACTION_LINE;
            editor_startAction (e, action, t1, t2, y);
            glist_drawTemporary (glist, t1, t2);
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
    
    if (!isText) {
        glist_objectSelectIfNotSelected (glist, y);
        editor_startAction (e, ACTION_MOVE, a, b, y);
        
    } else {
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
    t_gobj *y = NULL; int k = 0;
    
    int i, n = glist_objectGetNumberOf (glist);
    
    for (i = n - 1; i >= 0; i--) {
    //
    t_rectangle r;
    
    y = glist_objectGetAt (glist, i);
    
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
    //
    glist_drawLasso (glist, a, b);
    editor_startAction (glist_getEditor (glist), ACTION_REGION, a, b, NULL);
    //
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

void glist_cancelEditingBox (t_glist *glist)
{
    if (editor_hasSelectedBox (glist_getEditor (glist))) { glist_deselectAll (glist); }
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
