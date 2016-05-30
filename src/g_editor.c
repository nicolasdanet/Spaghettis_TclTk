
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
#include "s_system.h"
#include "s_utf8.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class              *canvas_class;
extern t_pd                 pd_canvasMaker;
extern t_widgetbehavior     text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist                     *editor_canvasCurrentlyPastingOn;                   /* Shared. */
int                         editor_indexOffsetConnectingPastedObjects;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_buffer             *editor_buffer;                                     /* Shared. */
static int                  editor_pasteCount;                                  /* Shared. */

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
    int a, b, c, d;
    int m, n, o, p;
    
    int previousX = glist->gl_editor->e_previousX;
    int previousY = glist->gl_editor->e_previousY;
    
    t_gobj *y1 = canvas_getHitObject (glist, previousX, previousY, &a, &b, &c, &d);
    t_gobj *y2 = canvas_getHitObject (glist, positionX, positionY, &m, &n, &o, &p);
    
    if (create) { sys_vGui (".x%lx.c delete TEMPORARY\n", glist); }
    else {
        sys_vGui (".x%lx.c coords TEMPORARY %d %d %d %d\n",
                        glist,
                        glist->gl_editor->e_previousX,
                        glist->gl_editor->e_previousY,
                        positionX,
                        positionY);
    }

    if (y1 && y2) {
    //
    t_object *object1 = canvas_castToObjectIfPatchable (y1);
    t_object *object2 = canvas_castToObjectIfPatchable (y2);
    
    if (object1 && object2 && object1 != object2) {
    //
    int numberOfOutlets = object_numberOfOutlets (object1);
    int numberOfInlets  = object_numberOfInlets (object2);
    
    if (numberOfOutlets && numberOfInlets) {
    //
    int closest1 = INLET_NEXTTO (previousX, a, c, numberOfOutlets);
    int closest2 = INLET_NEXTTO (positionX, m, o, numberOfInlets);
    
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
                        a + INLET_MIDDLE ((c - a), closest1, numberOfOutlets),
                        d,
                        m + INLET_MIDDLE ((o - m), closest2, numberOfInlets),
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
    int a, b, c, d;
        
    t_gobj *y = canvas_getHitObject (glist, 
                        glist->gl_editor->e_previousX, 
                        glist->gl_editor->e_previousY,
                        &a, &b, &c, &d);
        
    if (y) {
    //
    t_object *object = canvas_castToObjectIfPatchable (y);
    
    if (object) {
    //
    if (canvas_objectIsBox (object)) {
        int w = (positionX - a) / font_getHostFontWidth (canvas_getFontSize (glist));
        object->te_width = PD_MAX (1, w);
        gobj_visibilityChanged (y, glist, 0);
        canvas_updateLinesByObject (glist, object);
        gobj_visibilityChanged (y, glist, 1);
        canvas_dirty (glist, 1);
        
    } else if (pd_class (object) == canvas_class) {
        gobj_visibilityChanged (y, glist, 0);
        cast_glist (object)->gl_width  += positionX - glist->gl_editor->e_newX;
        cast_glist (object)->gl_height += positionY - glist->gl_editor->e_newY;
        glist->gl_editor->e_newX = positionX;
        glist->gl_editor->e_newY = positionY;
        canvas_updateLinesByObject (glist, object);
        gobj_visibilityChanged (y, glist, 1);
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

static void canvas_performMouseResetGrabbed (t_glist *glist)
{
    if (glist->gl_editor->e_grabbed) {
        if (glist->gl_editor->e_fnKey) { 
            (*glist->gl_editor->e_fnKey) (glist->gl_editor->e_grabbed, 0); 
        }
        glist_grab (glist, NULL, NULL, NULL, 0, 0);
    }
    
    PD_ASSERT (!glist->gl_editor->e_grabbed);
}

static void canvas_performMouseClickRight (t_glist *glist, t_gobj *object, int positionX, int positionY)
{
    int canProperties = (!object || (object && class_hasPropertiesFunction (pd_class (object))));
    int canOpen = (object && class_hasMethod (pd_class (object), sym_open));
    
    sys_vGui ("::ui_menu::showPopup .x%lx %d %d %d %d\n",
                    glist, 
                    positionX, 
                    positionY, 
                    canProperties, 
                    canOpen);
}

static void canvas_performMouseClick (t_glist *glist, int positionX, int positionY, int modifier,int clicked)
{
    t_gobj *y = NULL;
    
    int a, b, c, d, k = 0;
        
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    if (gobj_hit (y, glist, positionX, positionY, &a, &b, &c, &d)) {
    
        k = gobj_click (y, glist, 
                positionX, 
                positionY, 
                (modifier & MODIFIER_SHIFT), 
                (modifier & MODIFIER_CTRL), 
                (modifier & MODIFIER_ALT), 
                (modifier & MODIFIER_DOUBLE), 
                clicked);
                
        if (k) { break; }
    }
    //
    }
    
    if (!clicked) {
        if (y && k) { canvas_setCursorType (glist, CURSOR_CLICK); }
        else {
            canvas_setCursorType (glist, CURSOR_NOTHING);
        }
    }
}

static int canvas_performMouseHitResizeZone (t_object *object, int positionX, int positionY, int c, int d)
{
    if (object) {
        if (canvas_objectIsBox (object) || canvas_castToGlistChecked (cast_pd (object))) {
            if (positionX > (c - EDITOR_GRIP_SIZE) && positionY < (d - EDITOR_GRIP_SIZE)) {
                return 1;
            }
        }
    }
     
    return 0;
}

static int canvas_performMouseHitOutlets (t_object *object,
    int positionX,
    int positionY,
    int a,  
    int c, 
    int d, 
    int *h)
{
    if (object) {
    //
    int numberOfOutlets = object_numberOfOutlets (object);
    
    if (numberOfOutlets && (positionY >= d - EDITOR_GRIP_SIZE)) {
    //
    int closest = INLET_NEXTTO (positionX, a, c, numberOfOutlets);
    int hotspot = a + INLET_MIDDLE ((c - a), closest, numberOfOutlets);

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

static int canvas_performMouseHit (t_glist *glist, int positionX, int positionY, int modifier, int clicked)
{
    int n, h, a, b, c, d;
    
    t_gobj *y = canvas_getHitObject (glist, positionX, positionY, &a, &b, &c, &d);
        
    if (!y) { return 0; }
    else {
    //
    t_object *object = canvas_castToObjectIfPatchable (y);

    if (modifier & MODIFIER_RIGHT) { canvas_performMouseClickRight (glist, y, positionX, positionY); }
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
          
        if (canvas_performMouseHitResizeZone (object, positionX, positionY, c, d)) {
        
            if (!clicked) { canvas_setCursorType (glist, CURSOR_RESIZE); }
            else {
                canvas_selectObjectIfNotSelected (glist, y);
                glist->gl_editor->e_action = ACTION_RESIZE;
                glist->gl_editor->e_previousX = a;
                glist->gl_editor->e_previousY = b;
                glist->gl_editor->e_newX = positionX;
                glist->gl_editor->e_newY = positionY;
            }  
                                             
        } else if ((n = canvas_performMouseHitOutlets (object, positionX, positionY, a, c, d, &h)) != -1) {
            
            if (!clicked) { canvas_setCursorType (glist, CURSOR_CONNECT); }
            else {
                glist->gl_editor->e_action = ACTION_CONNECT;
                glist->gl_editor->e_previousX = h;
                glist->gl_editor->e_previousY = d;
                
                sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags TEMPORARY\n",
                                glist,
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

static int canvas_performMouseLines (t_glist *glist, int positionX, int positionY, int clicked)
{
    t_glist *g = canvas_getView (glist);
    t_outconnect *connection = NULL;
    t_linetraverser t;
        
    canvas_traverseLinesStart (&t, g);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    t_float a = t.tr_lineStartX;
    t_float b = t.tr_lineStartY;
    t_float c = t.tr_lineEndX;
    t_float d = t.tr_lineEndY;

    if (positionX < PD_MIN (a, c)) { continue; }
    if (positionX > PD_MAX (a, c)) { continue; }
    if (positionY < PD_MIN (b, d)) { continue; }
    if (positionY > PD_MAX (b, d)) { continue; }
    
    /* Area of the triangle. */
    
    t_float area = a * (positionY - d) + positionX * (d - b) + c * (b - positionY);
    
    /* Tolerance proportional to the distance between the line extremities. */
    
    t_float k = PD_MAX (PD_ABS (c - a), PD_ABS (d - b));    
    
    if (PD_ABS (area) < (k * EDITOR_GRIP_SIZE)) {
        if (clicked) {
            canvas_selectLine (g, 
                connection, 
                canvas_getIndexOfObject (g, cast_gobj (t.tr_srcObject)), 
                t.tr_srcIndexOfOutlet,
                canvas_getIndexOfObject (g, cast_gobj (t.tr_destObject)), 
                t.tr_destIndexOfInlet);
        }
        
        return 1;
    }
    //
    }
    
    return 0;
}

static void canvas_performMouseLassoStart (t_glist *glist, int positionX, int positionY, int modifier)
{
    if (!(modifier & MODIFIER_SHIFT)) { canvas_deselectAll (glist); }
    
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -tags TEMPORARY\n",
                    glist,
                    positionX,
                    positionY,
                    positionX,
                    positionY);

    glist->gl_editor->e_action = ACTION_REGION;
    glist->gl_editor->e_previousX = positionX;
    glist->gl_editor->e_previousY = positionY;
}

static void canvas_performMouse (t_glist *glist, int a, int b, int modifier, int clicked)
{
    int hasShift     = (modifier & MODIFIER_SHIFT);
    int isRightClick = (modifier & MODIFIER_RIGHT);
    int isRunMode    = (modifier & MODIFIER_CTRL) || (!glist->gl_isEditMode);
    
    if (clicked) { canvas_performMouseResetGrabbed (glist); glist->gl_editor->e_action = ACTION_NONE; }

    if (glist->gl_editor->e_action == ACTION_NONE) {

        glist->gl_editor->e_previousX = a;
        glist->gl_editor->e_previousY = b;

        if (isRunMode && !isRightClick) { canvas_performMouseClick (glist, a, b, modifier, clicked); }
        else if (canvas_performMouseHit (glist, a, b, modifier, clicked)) { } 
        else {
        
            if (isRightClick)    { canvas_performMouseClickRight (glist, NULL, a, b); }
            else if (!isRunMode) {
                if (!hasShift && canvas_performMouseLines (glist, a, b, clicked)) {
                } else if (clicked) {
                    canvas_performMouseLassoStart (glist, a, b, modifier);
                }
            }
            
            canvas_setCursorType (glist, CURSOR_NOTHING);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_performCopy (t_glist *glist)
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
    
    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    int m = canvas_isObjectSelected (glist, cast_gobj (t.tr_srcObject));
    int n = canvas_isObjectSelected (glist, cast_gobj (t.tr_destObject));
    
    if (m &&n) {
        buffer_vAppend (b, "ssiiii;", 
            sym___hash__X, 
            sym_connect,
            canvas_getIndexOfObjectAmongSelected (glist, &t.tr_srcObject->te_g),
            t.tr_srcIndexOfOutlet,
            canvas_getIndexOfObjectAmongSelected (glist, &t.tr_destObject->te_g),
            t.tr_destIndexOfInlet);
    }
    //
    }
    
    buffer_free (editor_buffer); 
    editor_buffer = b;
    //
    }
}

static void canvas_performPaste (t_glist *glist)
{
    t_gobj *y = NULL;
    t_selection *s = NULL;
    int numberOfObjectsAlreadyThere = 0;
    int i = 0;
    int n = EDITOR_PASTE_OFFSET * ++editor_pasteCount;
    int state = dsp_suspend();
    
    t_pd *boundA = s__A.s_thing; 
    t_pd *boundX = s__X.s_thing;
    t_pd *boundN = s__N.s_thing;
    
    s__A.s_thing = NULL;
    s__X.s_thing = cast_pd (glist);
    s__N.s_thing = &pd_canvasMaker;

    canvas_deselectAll (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) { numberOfObjectsAlreadyThere++; }

    editor_canvasCurrentlyPastingOn = glist;
    editor_indexOffsetConnectingPastedObjects = numberOfObjectsAlreadyThere;
        
    buffer_eval (editor_buffer, NULL, 0, NULL);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (i >= numberOfObjectsAlreadyThere) { canvas_selectObject (glist, y); }
        i++;
    }
    
    editor_canvasCurrentlyPastingOn = NULL;
    
    dsp_resume (state);
    
    s__A.s_thing = boundA;
    s__X.s_thing = boundX;
    s__N.s_thing = boundN;

    for (s = glist->gl_editor->e_selectedObjects; s; s = s->sel_next) {
        gobj_displace (s->sel_what, glist, n, n);
    }
    
    for (s = glist->gl_editor->e_selectedObjects; s; s = s->sel_next) {
        if (pd_class (s->sel_what) == canvas_class) { canvas_loadbang (cast_glist (s->sel_what)); }
    }
    
    canvas_dirty (glist, 1);
    
    sys_vGui ("::ui_patch::updateScrollRegion .x%lx.c\n", glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_key (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    if (argc > 1) { 
    //
    int k, isDown = (atom_getFloat (argv + 0) != 0);
    
    t_symbol *s = sym__dummy;
    
    /* Assume key number is UTF-32. */
    
    UCS4_CODE_POINT n = IS_FLOAT (argv + 1) ? (UCS4_CODE_POINT)(GET_FLOAT (argv + 1)) : 0;
    
    /* Forbid following characters to avoid to mislead interpretation of scripts. */
    
    k |= (n == '{');
    k |= (n == '}');
    k |= (n == '\\');
    
    if (k) {
        post_error (PD_TRANSLATE ("key: keycode %d not allowed"), (int)n); return; 
    }

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
    
    if (sym__key->s_thing && isDown)    { if (n) { pd_float (sym__key->s_thing,   (t_float)n); } }
    if (sym__keyup->s_thing && !isDown) { if (n) { pd_float (sym__keyup->s_thing, (t_float)n); } }
    
    if (sym__keyname->s_thing) {
        t_atom a[2];
        SET_FLOAT (a + 0, isDown);
        SET_SYMBOL (a + 1, s);
        pd_list (sym__keyname->s_thing, 2, a);
    }
    
    /* Handle the event. */
    
    if (glist && glist->gl_editor && isDown) {
    //
    int isArrows = (s == sym_Up || s == sym_Down || s == sym_Left || s == sym_Right);

    if (glist->gl_editor->e_action == ACTION_MOVE) { glist->gl_editor->e_action = ACTION_NONE; }
    
    if (glist->gl_editor->e_grabbed && glist->gl_editor->e_fnKey) {
        if (n) { 
            (*glist->gl_editor->e_fnKey) (glist->gl_editor->e_grabbed, (t_float)n); 
        }
        
    } else if (glist->gl_editor->e_selectedText) {
        if (n || isArrows) {
            rtext_key (glist->gl_editor->e_selectedText, (int)n, s);
            if (glist->gl_editor->e_isTextDirty) { 
                canvas_dirty (glist, 1); 
            }
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

void canvas_click (t_glist *glist, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    canvas_visible (glist, 1);
}

void canvas_motion (t_glist *glist, t_float positionX, t_float positionY, t_float modifier)
{ 
    if (!glist->gl_editor) { return; }
    else {
    //
    int action = glist->gl_editor->e_action;
    
    int deltaX = positionX - glist->gl_editor->e_previousX;
    int deltaY = positionY - glist->gl_editor->e_previousY;
    
    canvas_setLastMotionCoordinates (glist, positionX, positionY);
    
    if (action == ACTION_MOVE) {
        clock_unset (glist->gl_editor->e_clock);
        clock_delay (glist->gl_editor->e_clock, 5.0);
        glist->gl_editor->e_newX = positionX;
        glist->gl_editor->e_newY = positionY;
    
    } else if (action == ACTION_CONNECT) {
        canvas_makeLineStart (glist, positionX, positionY);
        
    } else if (action == ACTION_REGION)  {
        canvas_selectingByLassoStart (glist, positionX, positionY);
        
    } else if (action == ACTION_PASS)    {
        PD_ASSERT (glist->gl_editor->e_fnMotion);
        (*glist->gl_editor->e_fnMotion) (cast_pd (glist->gl_editor->e_grabbed), deltaX, deltaY, modifier);
        glist->gl_editor->e_previousX = positionX;
        glist->gl_editor->e_previousY = positionY;
        
    } else if (action == ACTION_DRAG)    {
        t_boxtext *text = glist->gl_editor->e_selectedText;
        if (text) { boxtext_mouse (text, deltaX, deltaY, BOXTEXT_DRAG); }
                
    } else if (action == ACTION_RESIZE)  {
        canvas_motionResize (glist, positionX, positionY);

    } else {
        canvas_performMouse (glist, (int)positionX, (int)positionY, (int)modifier, 0);
    }
    //
    }
}

void canvas_mouse (t_glist *glist, t_float positionX, t_float positionY, t_float modifier)
{
    if (!glist->gl_editor) { return; } 
    else {
        canvas_performMouse (glist, (int)positionX, (int)positionY, (int)modifier, 1);
    }
}

void canvas_mouseUp (t_glist *glist, t_float positionX, t_float positionY)
{
    if (!glist->gl_editor) { return; }
    else {
    //
    int action = glist->gl_editor->e_action;
    
    if (action == ACTION_CONNECT) {
        canvas_makeLineEnd (glist, (int)positionX, (int)positionY);
        
    } else if (action == ACTION_REGION) {
        canvas_selectingByLassoEnd (glist, (int)positionX, (int)positionY);
        
    } else if (action == ACTION_MOVE) {
        if (canvas_getNumberOfSelectedObjects (glist) == 1) {
            gobj_activate (glist->gl_editor->e_selectedObjects->sel_what, glist, 1);
        }
    }

    glist->gl_editor->e_action = ACTION_NONE;
    //
    }
}

void canvas_setBounds (t_glist *glist, t_float a, t_float b, t_float c, t_float d)
{
    glist->gl_windowTopLeftX     = a;
    glist->gl_windowTopLeftY     = b;
    glist->gl_windowBottomRightX = c;
    glist->gl_windowBottomRightY = d;
}

/*
static void canvas_dosetbounds(t_glist *x, int x1, int y1, int x2, int y2)          // --
{
    int heightwas = y2 - y1;
    int heightchange = y2 - y1 - (x->gl_windowBottomRightY - x->gl_windowTopLef
    if (x->gl_windowTopLeftX == x1 && x->gl_windowTopLeftY == y1 &&                 // --
        x->gl_windowBottomRightX == x2 && x->gl_windowBottomRightY == y2)
            return;
    x->gl_windowTopLeftX = x1;
    x->gl_windowTopLeftY = y1;
    x->gl_windowBottomRightX = x2;
    x->gl_windowBottomRightY = y2;
    if (!canvas_isGraphOnParent(x) && (x->gl_valueDown < x->gl_valueUp))            // --
    {
        t_float diff = x->gl_valueUp - x->gl_valueDown;
        t_gobj *y;
        x->gl_valueUp = heightwas * diff;
        x->gl_valueDown = x->gl_valueUp - diff;
        for (y = x->gl_graphics; y; y = y->g_next)                                  // --
            if (canvas_castToObjectIfPatchable(&y->g_pd))                           // --
                gobj_displace(y, x, 0, heightchange);                               // --
        canvas_redraw(x);                                                           // --
    }
}
*/

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
    t_gobj *g = NULL;
    
    for (g = glist->gl_graphics; g; g = g->g_next) {
        t_object *o = NULL;
        if ((o = canvas_castToObjectIfPatchable (g)) && o->te_type == TYPE_TEXT) {
            t_boxtext *y = boxtext_fetch (glist, o);
            text_drawborder (o, glist, boxtext_getTag (y), boxtext_getWidth (y), boxtext_getHeight (y), 1);
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
        sys_vGui ("::ui_patch::setEditMode .x%lx %d\n", canvas_getView (glist), glist->gl_isEditMode);
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
        rtext_key (glist->gl_editor->e_selectedText, 127, &s_);
        canvas_dirty (glist, 1);
        
    } else if (glist->gl_editor->e_selectedObjects) {
        canvas_copy (glist);
        canvas_removeSelectedObjects (glist);
        sys_vGui ("::ui_patch::updateScrollRegion .x%lx.c\n", glist);
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
        sys_vGui ("clipboard append {%.*s}\n", s, t);   
        
    } else {
        canvas_performCopy (glist);
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
        canvas_performPaste (glist);
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
    canvas_performPaste (glist);
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
    char t[PD_STRING] = { 0 };
    
    t_editor *x = (t_editor *)PD_MEMORY_GET (sizeof (t_editor));
    
    string_sprintf (t, PD_STRING, ".x%lx", (t_int)owner);
    
    x->e_buffer     = buffer_new();
    x->e_clock      = clock_new (owner, (t_method)canvas_taskDisplace);
    x->e_guiconnect = guiconnect_new (cast_pd (owner), gensym (t));
    
    return x;
}

static void editor_free (t_editor *x)
{
    guiconnect_release (x->e_guiconnect, 1000.0);
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
        if (o = canvas_castToObjectIfPatchable (y)) { boxtext_new (glist, o); }
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
    while (text = glist->gl_editor->e_text) { boxtext_free (text); }
    
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
    editor_buffer = buffer_new();
}

void editor_release (void)
{
    if (editor_buffer) { buffer_free (editor_buffer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
