
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

#define EDIT_GRIP_SIZE      4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

static void glist_actionResize (t_glist *glist, int a, int b)
{
    t_rectangle r;
    
    /* Points below are the top left coordinates of the resized object. */
    
    int startX = drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int startY = drag_getStartY (editor_getDrag (glist_getEditor (glist)));
    
    t_gobj *y  = glist_objectHit (glist, startX, startY, &r);

    if (y && cast_objectIfConnectable (y)) {
        if (object_isViewAsBox (cast_object (y))) { glist_actionResizeBox (glist, y, a - startX); }
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
#pragma mark -

void glist_key (t_glist *glist, t_keycode n, t_symbol *s)
{
    t_editor *e = glist_getEditor (glist);
    
    if (editor_getAction (e) == ACTION_MOVE) { editor_resetAction (e); }
    
    if (editor_hasSelectedBox (e)) {
        box_key (editor_getSelectedBox (e), n, s);
        if (editor_hasSelectedBoxDirty (e)) { 
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
#pragma mark -

static void canvas_proceedMouseClickRight (t_glist *glist, t_gobj *y, int positionX, int positionY)
{
    int canProperties = (!y || (y && class_hasPropertiesFunction (pd_class (y))));
    int canOpen = (y && class_hasMethod (pd_class (y), sym_open));
    
    glist_deselectAll (glist);
    
    sys_vGui ("::ui_menu::showPopup .x%lx %d %d %d %d\n",
                    glist, 
                    positionX, 
                    positionY, 
                    canProperties, 
                    canOpen);
}

static void canvas_proceedMouseClick (t_glist *glist, int positionX, int positionY, int modifier, int clicked)
{
    t_gobj *y = NULL;
    
    int k = 0;
        
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_rectangle t;
    
    if (gobj_hit (y, glist, positionX, positionY, &t)) {
    
        t_mouse m;
        
        m.m_x       = positionX;
        m.m_y       = positionY;
        m.m_shift   = (modifier & MODIFIER_SHIFT);
        m.m_ctrl    = (modifier & MODIFIER_CTRL);
        m.m_alt     = (modifier & MODIFIER_ALT);
        m.m_dbl     = (modifier & MODIFIER_DOUBLE);
        m.m_clicked = clicked;
    
        k = gobj_mouse (y, glist, &m);
                
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

static int canvas_proceedMouseHitResizeZone (t_object *object, int positionX, int positionY, int c, int d)
{
    if (object) {
        if (object_isViewAsBox (object) || (pd_class (object) == canvas_class)) {
            if (positionX > (c - EDIT_GRIP_SIZE) && positionY < (d - EDIT_GRIP_SIZE)) {
                return 1;
            }
        }
    }
     
    return 0;
}

static int canvas_proceedMouseHitOutlets (t_object *object,
    int positionX,
    int positionY,
    int a,  
    int c, 
    int d, 
    int *h)
{
    if (object) {
    //
    int numberOfOutlets = object_getNumberOfOutlets (object);
    
    if (numberOfOutlets && (positionY >= d - EDIT_GRIP_SIZE)) {
    //
    int closest = inlet_nearby (positionX, a, c, numberOfOutlets);
    int hotspot = a + inlet_middle ((c - a), closest, numberOfOutlets);

    PD_ASSERT (closest >= 0 && closest < numberOfOutlets);
    
    if ((positionX > (hotspot - EDIT_GRIP_SIZE)) && (positionX < (hotspot + EDIT_GRIP_SIZE))) {
        *h = hotspot; return closest;
    }
    //
    }
    //
    }
    
    return -1;
}

static int canvas_proceedMouseHit (t_glist *glist, int positionX, int positionY, int modifier, int clicked)
{
    t_rectangle r;
    
    t_gobj *y = glist_objectHit (glist, positionX, positionY, &r);
        
    if (!y) { return 0; }
    else {
    //
    t_object *object = cast_objectIfConnectable (y);
    int a = rectangle_getTopLeftX (&r);
    int b = rectangle_getTopLeftY (&r);
    int c = rectangle_getBottomRightX (&r);
    int d = rectangle_getBottomRightY (&r);
        
    if (modifier & MODIFIER_RIGHT) { canvas_proceedMouseClickRight (glist, y, positionX, positionY); }
    else if (modifier & MODIFIER_SHIFT) {
    
        if (clicked) {
        //
        t_box *text = editor_getSelectedBox (glist_getEditor (glist));
        
        if (object && text && (text == box_fetch (glist, object))) {
            box_mouse (text, positionX - a, positionY - b, BOX_SHIFT);
            editor_startAction (glist_getEditor (glist), ACTION_DRAG);
            drag_begin (editor_getDrag (glist_getEditor (glist)), a, b);
            
        } else {
            if (glist_objectIsSelected (glist, y)) { glist_objectDeselect (glist, y); }
            else { 
                glist_objectSelect (glist, y);
            }
        }
        //
        }
        
    } else {
        
        int n, h;
        
        if (canvas_proceedMouseHitResizeZone (object, positionX, positionY, c, d)) {
        
            if (!clicked) { glist_updateCursor (glist, CURSOR_RESIZE); }
            else {
                glist_objectSelectIfNotSelected (glist, y);
                editor_startAction (glist_getEditor (glist), ACTION_RESIZE);
                drag_begin (editor_getDrag (glist_getEditor (glist)), a, b);
            }  
                                             
        } else if ((n = canvas_proceedMouseHitOutlets (object, positionX, positionY, a, c, d, &h)) != -1) {
            
            if (!clicked) { glist_updateCursor (glist, CURSOR_CONNECT); }
            else {
                editor_startAction (glist_getEditor (glist), ACTION_CONNECT);
                drag_begin (editor_getDrag (glist_getEditor (glist)), h, d);
                glist_drawTemporary (glist, h, d, object_isSignalOutlet (object, n));
            }                                   
                
        } else if (clicked) {
        
            t_box *text = editor_getSelectedBox (glist_getEditor (glist));
            
            if (object && text && (text == box_fetch (glist, object))) {
                int flag = (modifier & MODIFIER_DOUBLE) ? BOX_DOUBLE : BOX_DOWN;
                box_mouse (text, positionX - a, positionY - b, flag);
                editor_startAction (glist_getEditor (glist), ACTION_DRAG);
                drag_begin (editor_getDrag (glist_getEditor (glist)), a, b);
                
            } else {
                glist_objectSelectIfNotSelected (glist, y);
                editor_startAction (glist_getEditor (glist), ACTION_MOVE);
            }
            
        } else { 
            glist_updateCursor (glist, CURSOR_NOTHING);
        }
    }
    //
    }
    
    return 1;
}

static int canvas_proceedMouseLines (t_glist *glist, int positionX, int positionY, int clicked)
{
    t_glist *canvas = glist_getView (glist);
    t_outconnect *connection = NULL;
    t_traverser t;
        
    traverser_start (&t, canvas);
    
    while ((connection = traverser_next (&t))) {
    //
    if (cord_hit (traverser_getCord (&t), positionX, positionY)) {
        if (clicked) { glist_lineSelect (canvas, &t); }
        return 1;
    }
    //
    }
    
    return 0;
}

static void canvas_proceedMouseLassoStart (t_glist *glist, int a, int b, int modifier)
{
    int newlyCreated = 0;
    
    if (!(modifier & MODIFIER_SHIFT)) { newlyCreated = glist_deselectAll (glist); }
    
    if (!newlyCreated) {
    //
    glist_drawLasso (glist, a, b);
    editor_startAction (glist_getEditor (glist), ACTION_REGION);
    drag_begin (editor_getDrag (glist_getEditor (glist)), a, b);
    //
    }
}

void glist_mouse (t_glist *glist, int a, int b, int modifier, int clicked)
{
    int hasShift     = (modifier & MODIFIER_SHIFT);
    int isRightClick = (modifier & MODIFIER_RIGHT);
    int isRunMode    = (modifier & MODIFIER_CTRL) || (!glist_hasEditMode (glist));
    
    if (clicked) { editor_motionReset (glist_getEditor (glist)); }

    if (!editor_hasAction (glist_getEditor (glist))) {

        drag_begin (editor_getDrag (glist_getEditor (glist)), a, b);

        if (isRunMode && !isRightClick) { canvas_proceedMouseClick (glist, a, b, modifier, clicked); }
        else if (canvas_proceedMouseHit (glist, a, b, modifier, clicked)) { } 
        else {
        
            if (isRightClick)    { canvas_proceedMouseClickRight (glist, NULL, a, b); }
            else if (!isRunMode) {
                if (!hasShift && canvas_proceedMouseLines (glist, a, b, clicked)) {
                } else if (clicked) {
                    canvas_proceedMouseLassoStart (glist, a, b, modifier);
                }
            }
            
            glist_updateCursor (glist, CURSOR_NOTHING);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
