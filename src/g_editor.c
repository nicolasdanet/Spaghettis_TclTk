
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

extern t_class              *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist                     *editor_pasteCurrentCanvas;                         /* Static. */
int                         editor_pasteOffsetWhileConnectingObjects;           /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_buffer             *editor_buffer;                                     /* Static. */
static int                  editor_pasteCount;                                  /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EDITOR_GRIP_SIZE        4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EDITOR_PASTE_OFFSET     20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_taskDisplace (t_glist *glist)
{
    int deltaX = glist->gl_editor->e_newX - glist->gl_editor->e_previousX;
    int deltaY = glist->gl_editor->e_newY - glist->gl_editor->e_previousY;
    
    canvas_displaceSelectedObjects (glist, deltaX, deltaY);
        
    glist->gl_editor->e_previousX = glist->gl_editor->e_newX;
    glist->gl_editor->e_previousY = glist->gl_editor->e_newY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_makeLine (t_glist *glist, int positionX, int positionY, int create)
{
    t_rectangle r1;
    t_rectangle r2;
    
    int previousX = glist->gl_editor->e_previousX;
    int previousY = glist->gl_editor->e_previousY;
    
    t_gobj *yA = canvas_getHitObject (glist, previousX, previousY, &r1);
    t_gobj *yB = canvas_getHitObject (glist, positionX, positionY, &r2);
    
    if (create) { sys_vGui (".x%lx.c delete TEMPORARY\n", canvas_getView (glist)); }
    else {
        sys_vGui (".x%lx.c coords TEMPORARY %d %d %d %d\n",
                        canvas_getView (glist),
                        glist->gl_editor->e_previousX,
                        glist->gl_editor->e_previousY,
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

    if (!canvas_hasLine (glist, object1, closest1, object2, closest2)) {
    //
    if (object_isSignalOutlet (object1, closest1) && !object_isSignalInlet (object2, closest2)) { }
    else {
    //
    if (create) {
        t_outconnect *connection = object_connect (object1, closest1, object2, closest2);
        
        sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                        canvas_getView (glist),
                        a + inlet_middle ((c - a), closest1, numberOfOutlets),
                        d,
                        m + inlet_middle ((o - m), closest2, numberOfInlets),
                        n,
                        (object_isSignalOutlet (object1, closest1) ? 2 : 1),
                        connection);
                        
        canvas_dirty (glist, 1);
        
    } else { 
        canvas_setCursorType (glist, CURSOR_CONNECT);
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
    
    canvas_setCursorType (glist, CURSOR_NOTHING);
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
    
    int previousX = glist->gl_editor->e_previousX;
    int previousY = glist->gl_editor->e_previousY;
    
    t_gobj *y = canvas_getHitObject (glist, previousX, previousY, &r);
        
    if (y) {
    //
    t_object *object = cast_objectIfConnectable (y);
    
    if (object) {
    //
    if (object_isViewAsBox (object)) {
        int a = rectangle_getTopLeftX (&r);
        int w = (int)((positionX - a) / font_getHostFontWidth (canvas_getFontSize (glist)));
        object_setWidth (object, PD_MAX (1, w));
        gobj_visibilityChanged (y, glist, 0);
        canvas_updateLinesByObject (glist, object);
        gobj_visibilityChanged (y, glist, 1);
        canvas_dirty (glist, 1);
        
    } else if (pd_class (object) == canvas_class) {
        t_glist *t = cast_glist (object);
        int w = positionX - glist->gl_editor->e_newX;
        int h = positionY - glist->gl_editor->e_newY;
        gobj_visibilityChanged (y, glist, 0);
        rectangle_setWidth (&t->gl_geometryGraph, rectangle_getWidth (&t->gl_geometryGraph) + w);
        rectangle_setHeight (&t->gl_geometryGraph, rectangle_getHeight (&t->gl_geometryGraph) + h);
        glist->gl_editor->e_newX = positionX;
        glist->gl_editor->e_newY = positionY;
        canvas_updateLinesByObject (glist, object);
        gobj_visibilityChanged (y, glist, 1);
        canvas_updateGraphOnParentRectangle (t);
        canvas_dirty (glist, 1);
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_proceedMouseResetGrabbed (t_glist *glist)
{
    if (glist->gl_editor->e_grabbed) { canvas_setMotionFunction (glist, NULL, NULL, 0, 0); }
    
    PD_ASSERT (!glist->gl_editor->e_grabbed);
}

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
        if (y && k) { canvas_setCursorType (glist, k); }
        else {
            canvas_setCursorType (glist, CURSOR_NOTHING);
        }
    }
}

static int canvas_proceedMouseHitResizeZone (t_object *object, int positionX, int positionY, int c, int d)
{
    if (object) {
        if (object_isViewAsBox (object) || cast_glistChecked (cast_pd (object))) {
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
    
    t_gobj *y = canvas_getHitObject (glist, positionX, positionY, &r);
        
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
        t_boxtext *text = glist->gl_editor->e_selectedText;
        
        if (object && text && (text == boxtext_fetch (glist, object))) {
            boxtext_mouse (text, positionX - a, positionY - b, BOXTEXT_SHIFT);
            glist->gl_editor->e_action = ACTION_DRAG;
            glist->gl_editor->e_previousX = a;
            glist->gl_editor->e_previousY = b;
            
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
        
            if (!clicked) { canvas_setCursorType (glist, CURSOR_RESIZE); }
            else {
                canvas_selectObjectIfNotSelected (glist, y);
                glist->gl_editor->e_action = ACTION_RESIZE;
                glist->gl_editor->e_previousX = a;
                glist->gl_editor->e_previousY = b;
                glist->gl_editor->e_newX = positionX;
                glist->gl_editor->e_newY = positionY;
            }  
                                             
        } else if ((n = canvas_proceedMouseHitOutlets (object, positionX, positionY, a, c, d, &h)) != -1) {
            
            if (!clicked) { canvas_setCursorType (glist, CURSOR_CONNECT); }
            else {
                glist->gl_editor->e_action = ACTION_CONNECT;
                glist->gl_editor->e_previousX = h;
                glist->gl_editor->e_previousY = d;
                
                sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags TEMPORARY\n",
                                canvas_getView (glist),
                                h,
                                d,
                                h,
                                d,
                                (object_isSignalOutlet (object, n) ? 2 : 1));
            }                                   
                
        } else if (clicked) {
        
            t_boxtext *text = glist->gl_editor->e_selectedText;
            
            if (object && text && (text == boxtext_fetch (glist, object))) {
                int flag = (modifier & MODIFIER_DOUBLE) ? BOXTEXT_DOUBLE : BOXTEXT_DOWN;
                boxtext_mouse (text, positionX - a, positionY - b, flag);
                glist->gl_editor->e_action = ACTION_DRAG;
                glist->gl_editor->e_previousX = a;
                glist->gl_editor->e_previousY = b;
                
            } else {
                canvas_selectObjectIfNotSelected (glist, y);
                glist->gl_editor->e_action = ACTION_MOVE;
            }
            
        } else { 
            canvas_setCursorType (glist, CURSOR_NOTHING);
        }
    }
    //
    }
    
    return 1;
}

static int canvas_proceedMouseLines (t_glist *glist, int positionX, int positionY, int clicked)
{
    t_glist *canvas = canvas_getView (glist);
    t_outconnect *connection = NULL;
    t_linetraverser t;
        
    linetraverser_start (&t, canvas);
    
    while ((connection = linetraverser_next (&t))) {
    //
    if (cord_hit (linetraverser_getCord (&t), positionX, positionY)) {
        if (clicked) {
            canvas_selectLine (canvas, 
                connection, 
                canvas_getIndexOfObject (canvas, cast_gobj (linetraverser_getSource (&t))), 
                linetraverser_getIndexOfOutlet (&t),
                canvas_getIndexOfObject (canvas, cast_gobj (linetraverser_getDestination (&t))), 
                linetraverser_getIndexOfInlet (&t));
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
                    canvas_getView (glist),
                    positionX,
                    positionY,
                    positionX,
                    positionY);

    glist->gl_editor->e_action = ACTION_REGION;
    glist->gl_editor->e_previousX = positionX;
    glist->gl_editor->e_previousY = positionY;
    //
    }
}

static void canvas_proceedMouse (t_glist *glist, int a, int b, int modifier, int clicked)
{
    int hasShift     = (modifier & MODIFIER_SHIFT);
    int isRightClick = (modifier & MODIFIER_RIGHT);
    int isRunMode    = (modifier & MODIFIER_CTRL) || (!glist->gl_isEditMode);
    
    if (clicked) { canvas_proceedMouseResetGrabbed (glist); glist->gl_editor->e_action = ACTION_NONE; }

    if (glist->gl_editor->e_action == ACTION_NONE) {

        glist->gl_editor->e_previousX = a;
        glist->gl_editor->e_previousY = b;

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
            
            canvas_setCursorType (glist, CURSOR_NOTHING);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_proceedCopy (t_glist *glist)
{
    if (glist->gl_editor->e_selectedObjects) {
    //
    t_buffer *b = buffer_new();

    t_gobj *y = NULL;
    t_outconnect *connection = NULL;
    t_linetraverser t;
    
    editor_pasteCount = 0;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (canvas_isObjectSelected (glist, y)) { gobj_save (y, b); }
    }
    
    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
    //
    int m = canvas_isObjectSelected (glist, cast_gobj (linetraverser_getSource (&t)));
    int n = canvas_isObjectSelected (glist, cast_gobj (linetraverser_getDestination (&t)));
    
    if (m && n) {
        buffer_vAppend (b, "ssiiii;", 
            sym___hash__X, 
            sym_connect,
            canvas_getIndexOfObjectAmongSelected (glist, cast_gobj (linetraverser_getSource (&t))),
            linetraverser_getIndexOfOutlet (&t),
            canvas_getIndexOfObjectAmongSelected (glist, cast_gobj (linetraverser_getDestination (&t))),
            linetraverser_getIndexOfInlet (&t));
    }
    //
    }
    
    buffer_free (editor_buffer); 
    editor_buffer = b;
    //
    }
}

static void canvas_proceedPaste (t_glist *glist)
{
    t_gobj *y = NULL;
    t_selection *s = NULL;
    int numberOfObjectsAlreadyThere = 0;
    int i = 0;
    int n = EDITOR_PASTE_OFFSET * ++editor_pasteCount;
    int state = dsp_suspend();
     
    t_pd *boundX = instance_getBoundX();
    
    instance_setBoundX (cast_pd (glist));

    canvas_deselectAll (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) { numberOfObjectsAlreadyThere++; }

    editor_pasteCurrentCanvas = glist;
    editor_pasteOffsetWhileConnectingObjects = numberOfObjectsAlreadyThere;
        
    buffer_eval (editor_buffer, NULL, 0, NULL);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (i >= numberOfObjectsAlreadyThere) { canvas_selectObject (glist, y); }
        i++;
    }
    
    editor_pasteCurrentCanvas = NULL;
    
    dsp_resume (state);
    
    instance_setBoundX (boundX);
        
    for (s = glist->gl_editor->e_selectedObjects; s; s = s->sel_next) {
        gobj_displaced (s->sel_what, glist, n, n);
    }
    
    for (s = glist->gl_editor->e_selectedObjects; s; s = s->sel_next) {
        if (pd_class (s->sel_what) == canvas_class) { canvas_loadbang (cast_glist (s->sel_what)); }
    }
    
    canvas_dirty (glist, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_key (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
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
    
    if (glist && glist->gl_editor && isDown) {
    //
    if (glist->gl_editor->e_action == ACTION_MOVE) { glist->gl_editor->e_action = ACTION_NONE; }
    
    if (glist->gl_editor->e_selectedText) {
        boxtext_key (glist->gl_editor->e_selectedText, (t_keycode)n, s);
        if (glist->gl_editor->e_isTextDirty) { 
            canvas_dirty (glist, 1); 
        }
        
    } else if (s == sym_Delete || s == sym_BackSpace) {
        if (glist->gl_editor->e_isSelectedline) { canvas_removeSelectedLine (glist); }
        else if (glist->gl_editor->e_selectedObjects) { 
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
    if (glist->gl_editor) {
    //
    int a = atom_getFloatAtIndex (0, argc, argv);
    int b = atom_getFloatAtIndex (1, argc, argv);
    int m = atom_getFloatAtIndex (2, argc, argv);
        
    int action = glist->gl_editor->e_action;
    int deltaX = a - glist->gl_editor->e_previousX;
    int deltaY = b - glist->gl_editor->e_previousY;
    
    canvas_setLastMotionCoordinates (glist, a, b);
    
    if (action == ACTION_MOVE) {
        clock_unset (glist->gl_editor->e_clock);
        clock_delay (glist->gl_editor->e_clock, 5.0);
        glist->gl_editor->e_newX = a;
        glist->gl_editor->e_newY = b;
    
    } else if (action == ACTION_CONNECT) {
        canvas_makeLineStart (glist, a, b);
        
    } else if (action == ACTION_REGION)  {
        canvas_selectingByLassoStart (glist, a, b);
        
    } else if (action == ACTION_PASS)    {
        PD_ASSERT (glist->gl_editor->e_fnMotion);
        (*glist->gl_editor->e_fnMotion) (cast_pd (glist->gl_editor->e_grabbed), deltaX, deltaY, m);
        glist->gl_editor->e_previousX = a;
        glist->gl_editor->e_previousY = b;
        
    } else if (action == ACTION_DRAG)    {
        t_boxtext *text = glist->gl_editor->e_selectedText;
        if (text) { boxtext_mouse (text, deltaX, deltaY, BOXTEXT_DRAG); }
                
    } else if (action == ACTION_RESIZE)  {
        canvas_motionResize (glist, a, b);

    } else {
        canvas_proceedMouse (glist, a, b, m, 0);
    }
    //
    }
}

void canvas_mouse (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int m = (int)atom_getFloatAtIndex (2, argc, argv);

    if (glist->gl_editor) { canvas_proceedMouse (glist, a, b, m, 1); } 
}

void canvas_mouseUp (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    
    if (glist->gl_editor) {
    //
    int action = glist->gl_editor->e_action;
    
    if (action == ACTION_CONNECT)     { canvas_makeLineEnd (glist, a, b); }
    else if (action == ACTION_REGION) { canvas_selectingByLassoEnd (glist, a, b); }
    else if (action == ACTION_MOVE)   {
    //
    if (canvas_getNumberOfSelectedObjects (glist) == 1) {
        gobj_activated (glist->gl_editor->e_selectedObjects->sel_what, glist, 1);
    }
    //
    }

    glist->gl_editor->e_action = ACTION_NONE;
    //
    }
}

void canvas_window (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    rectangle_setByAtoms (&glist->gl_geometryWindow, argc, argv);
    
    /* Redraw a GOP opened in its own window (required for graph arrays). */
    
    if (canvas_canHaveWindow (glist) && canvas_isGraph (glist)) { canvas_redraw (glist); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_editmode (t_glist *glist, t_float f)
{
    int state = (int)(f != 0.0);
     
    if (glist->gl_isEditMode != state) {
    //
    glist->gl_isEditMode = state;
    
    if (state) {
    //
    if (canvas_isMapped (glist) && canvas_canHaveWindow (glist)) {
    //
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        t_object *o = NULL;
        if ((o = cast_objectIfConnectable (y)) && object_isComment (o)) {
            t_boxtext *text = boxtext_fetch (glist, o);
            canvas_drawBox (glist, o, boxtext_getTag (text), 1);
        }
    }
    //
    }
    //
    } else {
    //
    canvas_deselectAll (glist);
    
    if (canvas_isMapped (glist) && canvas_canHaveWindow (glist)) {
        sys_vGui (".x%lx.c delete COMMENTBAR\n", canvas_getView (glist));
    }
    //
    }
    
    if (canvas_isMapped (glist)) {
        sys_vGui ("::ui_patch::setEditMode .x%lx %d\n", glist, glist->gl_isEditMode);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_cut (t_glist *glist)
{
    if (!glist->gl_editor || !glist->gl_isEditMode) { return; }
    else {
    //
    if (glist->gl_editor->e_isSelectedline)    { canvas_removeSelectedLine (glist); }
    else if (glist->gl_editor->e_selectedText) {
        canvas_copy (glist);
        boxtext_key (glist->gl_editor->e_selectedText, (t_keycode)127, sym_Delete);
        canvas_dirty (glist, 1);
        
    } else if (glist->gl_editor->e_selectedObjects) {
        canvas_copy (glist);
        canvas_removeSelectedObjects (glist);
    }
    //
    }
}

void canvas_copy (t_glist *glist)
{
    if (!glist->gl_editor || !glist->gl_isEditMode) { return; }
    else {
    //
    if (glist->gl_editor->e_selectedText) {
        char *t = NULL;
        int s = 0;
        boxtext_getSelection (glist->gl_editor->e_selectedText, &t, &s);
        sys_gui ("clipboard clear\n");
        sys_vGui ("clipboard append {%.*s}\n", s, t);       /* < http://stackoverflow.com/a/13289324 > */
        
    } else {
        canvas_proceedCopy (glist);
    }
    //
    }
}

void canvas_paste (t_glist *glist)
{
    if (!glist->gl_editor || !glist->gl_isEditMode) { return; }
    else {
    //
    if (glist->gl_editor->e_selectedText) {
        sys_vGui ("::ui_bind::pasteText .x%lx\n", glist);
    } else {
        canvas_proceedPaste (glist);
    }
    //
    }
}

void canvas_duplicate (t_glist *glist)
{
    if (!glist->gl_editor || !glist->gl_isEditMode) { return; }
    else {
    //
    canvas_copy (glist);
    canvas_proceedPaste (glist);
    //
    }
}

void canvas_selectAll (t_glist *glist)
{
    if (!glist->gl_editor || !glist->gl_isEditMode) { return; }
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
#pragma mark -

static t_editor *editor_new (t_glist *owner)
{
    t_editor *x = (t_editor *)PD_MEMORY_GET (sizeof (t_editor));
 
    x->e_buffer     = buffer_new();
    x->e_clock      = clock_new ((void *)owner, (t_method)canvas_taskDisplace);
    x->e_guiconnect = guiconnect_new (cast_pd (owner));
    
    return x;
}

static void editor_free (t_editor *x)
{
    guiconnect_release (x->e_guiconnect);
    clock_free (x->e_clock);
    buffer_free (x->e_buffer);

    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_createEditorIfNone (t_glist *glist)
{
    if (!glist->gl_editor) {
    //
    t_gobj *y = NULL;
            
    glist->gl_editor = editor_new (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        t_object *o = NULL;
        if ((o = cast_objectIfConnectable (y))) { boxtext_new (glist, o); }
    }
    //
    }
}

void canvas_destroyEditorIfAny (t_glist *glist)
{
    if (glist->gl_editor) {
    //
    t_boxtext *text = NULL;
    
    canvas_deselectAll (glist);
    while ((text = glist->gl_editor->e_boxtexts)) { boxtext_free (text); }
    
    editor_free (glist->gl_editor);
    glist->gl_editor = NULL;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void editor_initialize (void)
{
    PD_ASSERT (sizeof (t_keycode) == sizeof (UCS4_CODE_POINT));
    
    editor_buffer = buffer_new();
}

void editor_release (void)
{
    if (editor_buffer) { buffer_free (editor_buffer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
