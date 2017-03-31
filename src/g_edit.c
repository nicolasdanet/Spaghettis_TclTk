
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
#include "s_utf8.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EDITOR_GRIP_SIZE    4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_makeLine (t_glist *glist, int positionX, int positionY, int create)
{
    t_rectangle r1;
    t_rectangle r2;
    
    int previousX = drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int previousY = drag_getStartY (editor_getDrag (glist_getEditor (glist)));
    
    t_gobj *yA = glist_objectHit (glist, previousX, previousY, &r1);
    t_gobj *yB = glist_objectHit (glist, positionX, positionY, &r2);
    
    if (create) { sys_vGui (".x%lx.c delete TEMPORARY\n", glist_getView (glist)); }
    else {
        sys_vGui (".x%lx.c coords TEMPORARY %d %d %d %d\n",
                        glist_getView (glist),
                        previousX,
                        previousY,
                        positionX,
                        positionY);
    }

    if (yA && yB) {
    //
    t_object *object1 = cast_objectIfConnectable (yA);
    t_object *object2 = cast_objectIfConnectable (yB);
    
    if (object1 && object2 && object1 != object2) {
    //
    int numberOfOutlets = object_getNumberOfOutlets (object1);
    int numberOfInlets  = object_getNumberOfInlets (object2);
    
    if (numberOfOutlets && numberOfInlets) {
    //
    int a = rectangle_getTopLeftX (&r1);
    int c = rectangle_getBottomRightX (&r1);
    int d = rectangle_getBottomRightY (&r1);
    int m = rectangle_getTopLeftX (&r2);
    int n = rectangle_getTopLeftY (&r2);
    int o = rectangle_getBottomRightX (&r2);
    
    int closest1 = inlet_nearby (previousX, a, c, numberOfOutlets);
    int closest2 = inlet_nearby (positionX, m, o, numberOfInlets);
    
    PD_ASSERT (closest1 >= 0 && closest1 < numberOfOutlets);
    PD_ASSERT (closest2 >= 0 && closest2 < numberOfInlets);

    if (!glist_lineExist (glist, object1, closest1, object2, closest2)) {
    //
    if (object_isSignalOutlet (object1, closest1) && !object_isSignalInlet (object2, closest2)) { }
    else {
    //
    if (create) {
        t_outconnect *connection = object_connect (object1, closest1, object2, closest2);
        
        sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                        glist_getView (glist),
                        a + inlet_middle ((c - a), closest1, numberOfOutlets),
                        d,
                        m + inlet_middle ((o - m), closest2, numberOfInlets),
                        n,
                        (object_isSignalOutlet (object1, closest1) ? 2 : 1),
                        connection);
                        
        glist_setDirty (glist, 1);
        
    } else { 
        glist_updateCursor (glist, CURSOR_CONNECT);
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

static void canvas_makeLineStart (t_glist *glist, int positionX, int positionY)
{
    canvas_makeLine (glist, positionX, positionY, 0);
}

static void canvas_makeLineEnd (t_glist *glist, int positionX, int positionY)
{
    canvas_makeLine (glist, positionX, positionY, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_motionResize (t_glist *glist, t_float positionX, t_float positionY)
{
    t_rectangle r;
    
    int previousX = drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int previousY = drag_getStartY (editor_getDrag (glist_getEditor (glist)));
    
    t_gobj *y = glist_objectHit (glist, previousX, previousY, &r);
        
    if (y) {
    //
    t_object *object = cast_objectIfConnectable (y);
    
    if (object) {
    //
    if (object_isViewAsBox (object)) {
    //
    int a = rectangle_getTopLeftX (&r);
    int w = (int)((positionX - a) / font_getHostFontWidth (glist_getFontSize (glist)));
    object_setWidth (object, PD_MAX (1, w));
    gobj_visibilityChanged (y, glist, 0);
    glist_updateLines (glist, object);
    gobj_visibilityChanged (y, glist, 1);
    glist_setDirty (glist, 1);
    //
    } else if (pd_class (object) == canvas_class) {
    //
    t_glist *t = cast_glist (object);
    int w = positionX - drag_getEndX (editor_getDrag (glist_getEditor (glist)));
    int h = positionY - drag_getEndY (editor_getDrag (glist_getEditor (glist)));
    gobj_visibilityChanged (y, glist, 0);
    rectangle_setWidth (glist_getGraphGeometry (t), rectangle_getWidth (glist_getGraphGeometry (t)) + w);
    rectangle_setHeight (glist_getGraphGeometry (t), rectangle_getHeight (glist_getGraphGeometry (t)) + h);
    drag_setEnd (editor_getDrag (glist_getEditor (glist)), positionX, positionY);
    glist_updateLines (glist, object);
    gobj_visibilityChanged (y, glist, 1);
    glist_updateRectangle (t);
    glist_setDirty (glist, 1);
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

static void canvas_proceedMouseClickRight (t_glist *glist, t_gobj *y, int positionX, int positionY)
{
    int canProperties = (!y || (y && class_hasPropertiesFunction (pd_class (y))));
    int canOpen = (y && class_hasMethod (pd_class (y), sym_open));
    
    canvas_deselectAll (glist);
    
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
            if (positionX > (c - EDITOR_GRIP_SIZE) && positionY < (d - EDITOR_GRIP_SIZE)) {
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
    
    if (numberOfOutlets && (positionY >= d - EDITOR_GRIP_SIZE)) {
    //
    int closest = inlet_nearby (positionX, a, c, numberOfOutlets);
    int hotspot = a + inlet_middle ((c - a), closest, numberOfOutlets);

    PD_ASSERT (closest >= 0 && closest < numberOfOutlets);
    
    if ((positionX > (hotspot - EDITOR_GRIP_SIZE)) && (positionX < (hotspot + EDITOR_GRIP_SIZE))) {
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
            drag_setStart (editor_getDrag (glist_getEditor (glist)), a, b);
            
        } else {
            if (canvas_isObjectSelected (glist, y)) { canvas_deselectObject (glist, y); }
            else { 
                canvas_selectObject (glist, y);
            }
        }
        //
        }
        
    } else {
        
        int n, h;
        
        if (canvas_proceedMouseHitResizeZone (object, positionX, positionY, c, d)) {
        
            if (!clicked) { glist_updateCursor (glist, CURSOR_RESIZE); }
            else {
                canvas_selectObjectIfNotSelected (glist, y);
                editor_startAction (glist_getEditor (glist), ACTION_RESIZE);
                drag_set (editor_getDrag (glist_getEditor (glist)), a, b, positionX, positionY);
            }  
                                             
        } else if ((n = canvas_proceedMouseHitOutlets (object, positionX, positionY, a, c, d, &h)) != -1) {
            
            if (!clicked) { glist_updateCursor (glist, CURSOR_CONNECT); }
            else {
                editor_startAction (glist_getEditor (glist), ACTION_CONNECT);
                drag_setStart (editor_getDrag (glist_getEditor (glist)), h, d);
                
                sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags TEMPORARY\n",
                                glist_getView (glist),
                                h,
                                d,
                                h,
                                d,
                                (object_isSignalOutlet (object, n) ? 2 : 1));
            }                                   
                
        } else if (clicked) {
        
            t_box *text = editor_getSelectedBox (glist_getEditor (glist));
            
            if (object && text && (text == box_fetch (glist, object))) {
                int flag = (modifier & MODIFIER_DOUBLE) ? BOX_DOUBLE : BOX_DOWN;
                box_mouse (text, positionX - a, positionY - b, flag);
                editor_startAction (glist_getEditor (glist), ACTION_DRAG);
                drag_setStart (editor_getDrag (glist_getEditor (glist)), a, b);
                
            } else {
                canvas_selectObjectIfNotSelected (glist, y);
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
        if (clicked) {
            canvas_selectLine (canvas, 
                connection, 
                glist_objectGetIndexOf (canvas, cast_gobj (traverser_getSource (&t))), 
                traverser_getIndexOfOutlet (&t),
                glist_objectGetIndexOf (canvas, cast_gobj (traverser_getDestination (&t))), 
                traverser_getIndexOfInlet (&t));
        }
        
        return 1;
    }
    //
    }
    
    return 0;
}

static void canvas_proceedMouseLassoStart (t_glist *glist, int positionX, int positionY, int modifier)
{
    int newlyCreated = 0;
    
    if (!(modifier & MODIFIER_SHIFT)) { newlyCreated = canvas_deselectAll (glist); }
    
    if (!newlyCreated) {
    //
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -tags TEMPORARY\n",
                    glist_getView (glist),
                    positionX,
                    positionY,
                    positionX,
                    positionY);

    editor_startAction (glist_getEditor (glist), ACTION_REGION);
    drag_setStart (editor_getDrag (glist_getEditor (glist)), positionX, positionY);
    //
    }
}

static void canvas_proceedMouse (t_glist *glist, int a, int b, int modifier, int clicked)
{
    int hasShift     = (modifier & MODIFIER_SHIFT);
    int isRightClick = (modifier & MODIFIER_RIGHT);
    int isRunMode    = (modifier & MODIFIER_CTRL) || (!glist_hasEditMode (glist));
    
    if (clicked) { editor_motionReset (glist_getEditor (glist)); }

    if (!editor_hasAction (glist_getEditor (glist))) {

        drag_setStart (editor_getDrag (glist_getEditor (glist)), a, b);

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
#pragma mark -

void canvas_key (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    PD_ASSERT (sizeof (t_keycode) == sizeof (UCS4_CODE_POINT));
    
    if (argc > 1) { 
    //
    int isDown = ((int)(atom_getFloat (argv + 0)) != 0);
    int k = 0;
    
    t_symbol *s = sym__dummy;
    
    /* Assume key number is UTF-32. */
    
    UCS4_CODE_POINT n = IS_FLOAT (argv + 1) ? (UCS4_CODE_POINT)(GET_FLOAT (argv + 1)) : 0;
    
    /* Forbid following characters to avoid mislead interpretations at script level. */
    
    k |= (n == '{');    // --
    k |= (n == '}');    // --
    k |= (n == '\\');
    
    if (k) { error_ignored (sym_key); return; }

    /* Parse special keys. */
    
    if (IS_SYMBOL (argv + 1)) { 
    //
    s = GET_SYMBOL (argv + 1); 
    
    if (s == sym_Enter)         { n = 3;   }
    if (s == sym_BackSpace)     { n = 8;   }
    if (s == sym_Tab)           { n = 9;   }
    if (s == sym_Return)        { n = 10;  }
    if (s == sym_Escape)        { n = 27;  }
    if (s == sym_Space)         { n = 32;  }
    if (s == sym_Delete)        { n = 127; }
    //
    }
    
    if (IS_FLOAT (argv + 1)) {
    //
    switch (n) {
    //
    case 3   : s = sym_Enter;       break;
    case 8   : s = sym_BackSpace;   break;
    case 9   : s = sym_Tab;         break;
    case 10  : s = sym_Return;      break;
    case 13  : s = sym_Return;      break;
    case 27  : s = sym_Escape;      break;
    case 32  : s = sym_Space;       break;
    case 127 : s = sym_Delete;      break;
    //
    }
    
    /* Encode UTF-32 as UTF-8. */
    
    if (s == sym__dummy) {
        char t[UTF8_MAXIMUM_BYTES + 1] = { 0 };
        int size = u8_wc_toutf8 (t, n); 
        t[size] = 0;
        s = gensym (t);     
    }
    //
    }

    /* Report keystrokes to bounded objects. */
    
    if (pd_isThingQuiet (sym__key) && isDown && n)      { pd_float (pd_getThing (sym__key),   (t_float)n); }
    if (pd_isThingQuiet (sym__keyup) && !isDown && n)   { pd_float (pd_getThing (sym__keyup), (t_float)n); }
    
    if (pd_isThingQuiet (sym__keyname)) {
        t_atom a[2];
        SET_FLOAT (a + 0, isDown);
        SET_SYMBOL (a + 1, s);
        pd_list (pd_getThing (sym__keyname), 2, a);
    }
    
    /* Handle the event. */
    
    if (glist && isDown) {
    //
    if (editor_getAction (glist_getEditor (glist)) == ACTION_MOVE) { 
        editor_resetAction (glist_getEditor (glist)); 
    }
    
    if (editor_hasSelectedBox (glist_getEditor (glist))) {
        box_key (editor_getSelectedBox (glist_getEditor (glist)), (t_keycode)n, s);
        if (editor_hasSelectedBoxDirty (glist_getEditor (glist))) { 
            glist_setDirty (glist, 1); 
        }
        
    } else if (s == sym_Delete || s == sym_BackSpace) {
        if (editor_hasSelectedLine (glist_getEditor (glist))) { canvas_removeSelectedLine (glist); }
        else if (editor_hasSelection (glist_getEditor (glist))) { 
            canvas_removeSelectedObjects (glist); 
        }
    }
    //
    }
    //
    }
}

void canvas_click (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_visible (glist, 1);
}

void canvas_motion (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = atom_getFloatAtIndex (0, argc, argv);
    int b = atom_getFloatAtIndex (1, argc, argv);
    int m = atom_getFloatAtIndex (2, argc, argv);
        
    int action = editor_getAction (glist_getEditor (glist));
    int deltaX = a - drag_getStartX (editor_getDrag (glist_getEditor (glist)));
    int deltaY = b - drag_getStartY (editor_getDrag (glist_getEditor (glist)));
    
    instance_setDefaultCoordinates (glist, a, b);
    
    if (action == ACTION_MOVE) {
        editor_selectionDeplace (glist_getEditor (glist));
        drag_setEnd (editor_getDrag (glist_getEditor (glist)), a, b);
    
    } else if (action == ACTION_CONNECT) {
        canvas_makeLineStart (glist, a, b);
        
    } else if (action == ACTION_REGION)  {
        canvas_selectingByLassoStart (glist, a, b);
        
    } else if (action == ACTION_PASS)    {
        editor_motionProceed (glist_getEditor (glist), deltaX, deltaY, m);
        drag_setStart (editor_getDrag (glist_getEditor (glist)), a, b);
        
    } else if (action == ACTION_DRAG)    {
        t_box *text = editor_getSelectedBox (glist_getEditor (glist));
        if (text) { box_mouse (text, deltaX, deltaY, BOX_DRAG); }
                
    } else if (action == ACTION_RESIZE)  {
        canvas_motionResize (glist, a, b);

    } else {
        canvas_proceedMouse (glist, a, b, m, 0);
    }
}

void canvas_mouse (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int m = (int)atom_getFloatAtIndex (2, argc, argv);

    canvas_proceedMouse (glist, a, b, m, 1);
}

void canvas_mouseUp (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int action = editor_getAction (glist_getEditor (glist));
    
    if (action == ACTION_CONNECT)     { canvas_makeLineEnd (glist, a, b); }
    else if (action == ACTION_REGION) { canvas_selectingByLassoEnd (glist, a, b); }
    else if (action == ACTION_MOVE)   {
    //
    if (glist_objectGetNumberOfSelected (glist) == 1) {
        gobj_activated (selection_getObject (editor_getSelection (glist_getEditor (glist))), glist, 1);
    }
    //
    }

    editor_resetAction (glist_getEditor (glist));
}

void canvas_window (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    rectangle_setByAtoms (glist_getWindowGeometry (glist), argc, argv);
    
    if (glist_isArray (glist)) { glist_updateWindow (glist); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_editmode (t_glist *glist, t_float f)
{
    int state = (int)(f != 0.0);
     
    if (glist_hasEditMode (glist) != state) {
    //
    glist_setEditMode (glist, state);
    
    if (state) { glist_drawAllCommentBars (glist); }
    else {
        canvas_deselectAll (glist); glist_eraseAllCommentBars (glist);
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

void canvas_cut (t_glist *glist)
{
    if (!glist_hasEditMode (glist)) { return; }
    else {
    //
    if (editor_hasSelectedLine (glist_getEditor (glist))) { canvas_removeSelectedLine (glist); }
    else if (editor_hasSelectedBox (glist_getEditor (glist))) {
        canvas_copy (glist);
        box_key (editor_getSelectedBox (glist_getEditor (glist)), (t_keycode)127, sym_Delete);
        glist_setDirty (glist, 1);
        
    } else if (editor_hasSelection (glist_getEditor (glist))) {
        canvas_copy (glist);
        canvas_removeSelectedObjects (glist);
    }
    //
    }
}

void canvas_copy (t_glist *glist)
{
    if (!glist_hasEditMode (glist)) { return; }
    else {
    //
    if (editor_hasSelectedBox (glist_getEditor (glist))) {
        char *t = NULL;
        int s = 0;
        box_getSelection (editor_getSelectedBox (glist_getEditor (glist)), &t, &s);
        sys_gui ("clipboard clear\n");
        sys_vGui ("clipboard append {%.*s}\n", s, t);       /* < http://stackoverflow.com/a/13289324 > */
        
    } else {
        clipboard_copy (instance_getClipboard(), glist);
    }
    //
    }
}

void canvas_paste (t_glist *glist)
{
    if (!glist_hasEditMode (glist)) { return; }
    else {
    //
    if (editor_hasSelectedBox (glist_getEditor (glist))) {
        sys_vGui ("::ui_bind::pasteText .x%lx\n", glist);
    } else {
        clipboard_paste (instance_getClipboard(), glist);
    }
    //
    }
}

void canvas_duplicate (t_glist *glist)
{
    if (!glist_hasEditMode (glist)) { return; }
    else {
    //
    canvas_copy (glist);
    clipboard_paste (instance_getClipboard(), glist);
    //
    }
}

void canvas_selectAll (t_glist *glist)
{
    if (!glist_hasEditMode (glist)) { return; }
    else {
    //
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (!canvas_isObjectSelected (glist, y)) { canvas_selectObject (glist, y); }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
