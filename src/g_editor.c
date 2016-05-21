
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

t_glist                     *editor_pasteCanvas;            /* Shared. */
int                         editor_pasteOnset;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_buffer             *editor_buffer;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EDITOR_HANDLE_SIZE      4

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
            if (positionX > (c - EDITOR_HANDLE_SIZE) && positionY < (d - EDITOR_HANDLE_SIZE)) {
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
    
    if (numberOfOutlets && (positionY >= d - EDITOR_HANDLE_SIZE)) {
    //
    int closest = INLETS_NEXTTO (positionX, a, c, numberOfOutlets);
    int hotspot = a + INLETS_MIDDLE ((c - a), closest, numberOfOutlets);

    PD_ASSERT (closest >= 0 && closest < numberOfOutlets);
    
    if ((positionX > (hotspot - EDITOR_HANDLE_SIZE)) && (positionX < (hotspot + EDITOR_HANDLE_SIZE))) {
        *h = hotspot; return closest;
    }
    //
    }
    //
    }
    
    return -1;
}

static int canvas_performMouseHitLines (t_glist *glist, int positionX, int positionY, int clicked)
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
    
    /*
    t_float area    = (c - a) * (positionY - b) - (d - b) * (positionX - a);
    t_float dsquare = (c - a) * (c - a) + (d - b) * (d - b);
    if (area * area >= 50 * dsquare) continue;
    if ((c - a) * (positionX - a) + (d - b) * (positionY - b) < 0) continue;
    if ((c - a) * (c - positionX) + (d - b) * (d - positionY) < 0) continue;
    */

    if (positionX < PD_MIN (a, c)) { continue; }
    if (positionX > PD_MAX (a, c)) { continue; }
    if (positionY < PD_MIN (b, d)) { continue; }
    if (positionY > PD_MAX (b, d)) { continue; }
    
    /* Area of the triangle. */
    
    t_float area = a * (positionY - d) + positionX * (d - b) + c * (b - positionY);
    
    /* Tolerance proportional to distance between line extremities. */
    
    t_float k = PD_MAX (PD_ABS (c - a), PD_ABS (d - b));    
    
    if (PD_ABS (area) < (k * EDITOR_HANDLE_SIZE)) {
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

static int canvas_performMouseHit (t_glist *glist, int positionX, int positionY, int modifier, int clicked)
{
    int n, h, a, b, c, d;
    
    t_gobj *y = canvas_getHitObject (glist, positionX, positionY, &a, &b, &c, &d);
        
    if (!y) { return 0; }
    else {
    //
    t_object *object = canvas_castToObjectIfPatchable (y);

    if (modifier & MODIFIER_RIGHT) {
        canvas_performMouseClickRight (glist, y, positionX, positionY);
    
    } else if (modifier & MODIFIER_SHIFT) {
    
        if (clicked) {
        //
        t_boxtext *text = glist->gl_editor->e_selectedText;
        
        if (object && text && (text == glist_findrtext (glist, object))) {
            rtext_mouse (text, positionX - a, positionY - b, BOX_TEXT_SHIFT);
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
        
            if (!clicked) { canvas_setCursorType (glist, CURSOR_EDIT_RESIZE); }
            else {
                canvas_selectObjectIfNotAlreadySelected (glist, y);
                glist->gl_editor->e_action = ACTION_RESIZE;
                glist->gl_editor->e_previousX = a;
                glist->gl_editor->e_previousY = b;
                glist->gl_editor->e_newX = positionX;
                glist->gl_editor->e_newY = positionY;
            }  
                                             
        } else if ((n = canvas_performMouseHitOutlets (object, positionX, positionY, a, c, d, &h)) != -1) {
            
            if (!clicked) { canvas_setCursorType (glist, CURSOR_EDIT_CONNECT); }
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
            
            if (object && text && (text == glist_findrtext (glist, object))) {
                int flag = (modifier & MODIFIER_DOUBLE) ? BOX_TEXT_DOUBLE : BOX_TEXT_DOWN;
                rtext_mouse (text, positionX - a, positionY - b, flag);
                glist->gl_editor->e_action = ACTION_DRAG;
                glist->gl_editor->e_previousX = a;
                glist->gl_editor->e_previousY = b;
                
            } else {
                canvas_selectObjectIfNotAlreadySelected (glist, y);
                glist->gl_editor->e_action = ACTION_MOVE;
            }
            
        } else { 
            canvas_setCursorType (glist, CURSOR_EDIT_NOTHING);
        }
    }
    //
    }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_performMouse (t_glist *glist, int positionX, int positionY, int modifier, int clicked)
{
    int hasShift        = (modifier & MODIFIER_SHIFT);
    int hasCtrl         = (modifier & MODIFIER_CTRL);
    int isRightClick    = (modifier & MODIFIER_RIGHT);
    
    int isRunMode       = hasCtrl || (!glist->gl_isEditMode);
    
    if (!glist->gl_editor) { PD_BUG; return; }
    
    if (clicked) { canvas_performMouseResetGrabbed (glist); glist->gl_editor->e_action = ACTION_NONE; }

    if (glist->gl_editor->e_action == ACTION_NONE) {

        glist->gl_editor->e_previousX = positionX;
        glist->gl_editor->e_previousY = positionY;

        if (isRunMode && !isRightClick) {
            canvas_performMouseClick (glist, positionX, positionY, modifier, clicked);
            return;
        }

        if (canvas_performMouseHit (glist, positionX, positionY, modifier, clicked)) { 
            return; 
        }

        if (isRightClick)    { canvas_performMouseClickRight (glist, NULL, positionX, positionY); }
        else if (!isRunMode) {
        
            if (!hasShift && canvas_performMouseHitLines (glist, positionX, positionY, clicked)) {
                canvas_setCursorType (glist, CURSOR_EDIT_DISCONNECT);
                return;
            }
            
            if (clicked) {
                if (!hasShift) canvas_deselectAll(glist);
                sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags TEMPORARY\n",
                      glist, positionX, positionY, positionX, positionY);
                glist->gl_editor->e_previousX = positionX;
                glist->gl_editor->e_previousY = positionY;
                glist->gl_editor->e_action = ACTION_REGION;
            }
        }
        
        canvas_setCursorType (glist, CURSOR_EDIT_NOTHING);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_key (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    if (argc > 1) { 
    //
    int isDown = (atom_getFloat (argv + 0) != 0);
    
    t_symbol *s = sym__dummy;
    
    /* Assume key number is UTF-32. */
    
    UCS4_CODE_POINT n = IS_FLOAT (argv + 1) ? (UCS4_CODE_POINT)(GET_FLOAT (argv + 1)) : 0;
    
    /* Forbid following characters to avoid to mislead interpretation of scripts. */
    
    if (n == '{' || n == '}' || n == '\\') { 
        post_error (PD_TRANSLATE ("key: keycode %d not allowed"), (int)n);
        return; 
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
        if (n) { (*glist->gl_editor->e_fnKey) (glist->gl_editor->e_grabbed, (t_float)n); }
        
    } else if (glist->gl_editor->e_selectedText) {
        if (n || isArrows) {
            rtext_key (glist->gl_editor->e_selectedText, (int)n, s);
            if (glist->gl_editor->e_isTextDirty) { canvas_dirty (glist, 1); }
        }
        
    } else if (s == sym_Delete || s == sym_BackSpace) {
        if (glist->gl_editor->e_isSelectedline)       { canvas_removeSelectedLine (glist);    }
        else if (glist->gl_editor->e_selectedObjects) { canvas_removeSelectedObjects (glist); }
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
    if (!glist->gl_editor) { PD_BUG; return; }
    else {
    //
    int action = glist->gl_editor->e_action;
    
    int a, b, c, d;
    int deltaX = positionX - glist->gl_editor->e_previousX;
    int deltaY = positionY - glist->gl_editor->e_previousY;
    
    canvas_setLastMotionCoordinates (glist, positionX, positionY);
    
    if (action == ACTION_MOVE) {
        clock_unset (glist->gl_editor->e_clock);
        clock_delay (glist->gl_editor->e_clock, 5.0);
        glist->gl_editor->e_newX = positionX;
        glist->gl_editor->e_newY = positionY;
    
    } else if (action == ACTION_CONNECT) {
        canvas_makingLine (glist, positionX, positionY, 0);
        
    } else if (action == ACTION_REGION)  {
        canvas_selectingByLasso (glist, positionX, positionY, 0);
        
    } else if (action == ACTION_PASS)    {
        PD_ASSERT (glist->gl_editor->e_fnMotion);
        (*glist->gl_editor->e_fnMotion) (cast_pd (glist->gl_editor->e_grabbed), deltaX, deltaY, modifier);
        glist->gl_editor->e_previousX = positionX;
        glist->gl_editor->e_previousY = positionY;
        
    } else if (action == ACTION_DRAG)    {
        t_boxtext *text = glist->gl_editor->e_selectedText;
        if (text) { rtext_mouse (text, deltaX, deltaY, BOX_TEXT_DRAG); }
                
    } else if (action == ACTION_RESIZE)  {

        t_gobj *y = canvas_getHitObject (glist, 
                        glist->gl_editor->e_previousX, 
                        glist->gl_editor->e_previousY,
                        &a, &b, &c, &d);
        
        if (y) {
            t_object *object = canvas_castToObjectIfPatchable (y);
            
            if (object) {
            //
            if (canvas_objectIsBox (object)) {
                int w = (positionX - a) / font_getHostFontWidth (canvas_getFontSize (glist));
                object->te_width = PD_MAX (1, w);
                gobj_visibilityChanged (y, glist, 0);
                canvas_updateLinesByObject (glist, object);
                gobj_visibilityChanged (y, glist, 1);
                
            } else if (pd_class (object) == canvas_class) {
                gobj_visibilityChanged (y, glist, 0);
                cast_glist (object)->gl_width  += positionX - glist->gl_editor->e_newX;
                cast_glist (object)->gl_height += positionY - glist->gl_editor->e_newY;
                glist->gl_editor->e_newX = positionX;
                glist->gl_editor->e_newY = positionY;
                canvas_updateLinesByObject (glist, object);
                gobj_visibilityChanged (y, glist, 1);
            }
            //
            }
        }

    } else {
        canvas_performMouse (glist, (int)positionX, (int)positionY, (int)modifier, 0);
    }
    //
    }
}

void canvas_mouse (t_glist *glist, t_float positionX, t_float positionY, t_float dummy, t_float modifier)
{
    canvas_performMouse (glist, (int)positionX, (int)positionY, (int)modifier, 1);
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
    
        if (canvas_isMapped (glist) && canvas_canHaveWindow (glist)) {

            t_gobj *g = NULL;
            canvas_setCursorType (glist, CURSOR_EDIT_NOTHING);
            
            for (g = glist->gl_graphics; g; g = g->g_next) {
                t_object *o = NULL;
                if ((o = canvas_castToObjectIfPatchable (g)) && o->te_type == TYPE_TEXT) {
                    t_boxtext *y = glist_findrtext (glist, o);
                    text_drawborder (o, glist, rtext_gettag (y), rtext_width (y), rtext_height (y), 1);
                }
            }
        }
        
    } else {
    
        canvas_deselectAll (glist);
        
        if (canvas_isMapped (glist) && canvas_canHaveWindow (glist)) {
            canvas_setCursorType (glist, CURSOR_NOTHING);
            sys_vGui (".x%lx.c delete COMMENTBAR\n", canvas_getView (glist));
        }
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

void canvas_mouseup(t_glist *x, t_float fxpos, t_float fypos, t_float fwhich)
{
    int xpos = fxpos, ypos = fypos, which = fwhich;
    /* post("mouseup %d %d %d", xpos, ypos, which); */
    if (!x->gl_editor)
    {
        PD_BUG;
        return;
    }

    if (x->gl_editor->e_action == ACTION_CONNECT)
        canvas_makingLine (x, xpos, ypos, 1);
    else if (x->gl_editor->e_action == ACTION_REGION)
        canvas_selectingByLasso(x, xpos, ypos, 1);
    else if (x->gl_editor->e_action == ACTION_MOVE ||
        x->gl_editor->e_action == ACTION_RESIZE)
    {
            /* after motion or resizing, if there's only one text item
                selected, activate the text */
        if (x->gl_editor->e_selectedObjects &&
            !(x->gl_editor->e_selectedObjects->sel_next))
        {
            t_gobj *g = x->gl_editor->e_selectedObjects->sel_what;
            t_glist *gl2;
                /* first though, check we aren't an abstraction with a
                dirty sub-patch that would be discarded if we edit this. */
            /*
            if (pd_class(&g->g_pd) == canvas_class &&
                canvas_isAbstraction((t_glist *)g) &&
                    (gl2 = canvas_findDirty((t_glist *)g)))
            {
                pd_vMessage(&gl2->gl_obj.te_g.g_pd, sym_open, "");
                x->gl_editor->e_action = ACTION_NONE;
                sys_vGui(
"::ui_confirm::checkAction .x%lx { Discard changes to %s? } { ::ui_interface::pdsend .x%lx dirty 0 } { no }\n",
                    canvas_getRoot(gl2),
                        canvas_getRoot(gl2)->gl_name->s_name, gl2);
                return;
            }*/
                /* OK, activate it */
            gobj_activate(x->gl_editor->e_selectedObjects->sel_what, x, 1);
        }
    }

    x->gl_editor->e_action = ACTION_NONE;
}

static t_buffer *canvas_docopy(t_glist *x)
{
    t_gobj *y;
    t_linetraverser t;
    t_outconnect *oc;
    t_buffer *b = buffer_new();
    for (y = x->gl_graphics; y; y = y->g_next)
    {
        if (canvas_isObjectSelected(x, y))
            gobj_save(y, b);
    }
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if (canvas_isObjectSelected(x, &t.tr_srcObject->te_g)
            && canvas_isObjectSelected(x, &t.tr_destObject->te_g))
        {
            buffer_vAppend(b, "ssiiii;", sym___hash__X, sym_connect,
                canvas_getIndexOfObjectAmongSelected(x, &t.tr_srcObject->te_g), t.tr_srcIndexOfOutlet,
                canvas_getIndexOfObjectAmongSelected(x, &t.tr_destObject->te_g), t.tr_destIndexOfInlet);
        }
    }
    return (b);
}

void canvas_copy (t_glist *x)
{
    if (!x->gl_editor || !x->gl_editor->e_selectedObjects)
        return;
    buffer_free(editor_buffer);
    editor_buffer = canvas_docopy(x);
    if (x->gl_editor->e_selectedText)
    {
        char *buf;
        int bufsize;
        rtext_getseltext(x->gl_editor->e_selectedText, &buf, &bufsize);
        sys_gui("clipboard clear\n");
        sys_vGui("clipboard append {%.*s}\n", bufsize, buf);
    }
}

void canvas_cut (t_glist *x)
{
    if (!x->gl_editor)  /* ignore if invisible */ 
        return;
    if (x->gl_editor && x->gl_editor->e_isSelectedline)   /* delete line */
        canvas_removeSelectedLine(x);
    else if (x->gl_editor->e_selectedText) /* delete selected text in a box */
    {
        char *buf;
        int bufsize;
        rtext_getseltext(x->gl_editor->e_selectedText, &buf, &bufsize);
        if (!bufsize && x->gl_editor->e_selectedObjects &&
            !x->gl_editor->e_selectedObjects->sel_next)
        {
                /* if the text is already empty, delete the box.  We
                first clear 'textedfor' so that canvas_removeSelected later will
                think the whole box was selected, not the text */
            x->gl_editor->e_selectedText = 0;
            goto deleteobj;
        }
        canvas_copy(x);
        rtext_key(x->gl_editor->e_selectedText, 127, &s_);
        canvas_dirty(x, 1);
    }
    else if (x->gl_editor && x->gl_editor->e_selectedObjects)
    {
    deleteobj:      /* delete one or more objects */
        // canvas_setundo(x, canvas_undo_cut, canvas_undo_set_cut(x, UCUT_CUT), "cut");
        canvas_copy(x);
        canvas_removeSelectedObjects(x);
        sys_vGui("::ui_patch::updateScrollRegion .x%lx.c\n", x);
    }
}

static void glist_donewloadbangs(t_glist *x)
{
    if (x->gl_editor)
    {
        t_selection *sel;
        for (sel = x->gl_editor->e_selectedObjects; sel; sel = sel->sel_next)
            if (pd_class(&sel->sel_what->g_pd) == canvas_class)
                canvas_loadbang((t_glist *)(&sel->sel_what->g_pd));
    }
}

static void canvas_dopaste(t_glist *x, t_buffer *b)
{
    t_gobj *newgobj, *last, *g2;
    int dspstate = dsp_suspend(), nbox, count;
    t_symbol *asym = sym___hash__A;
        /* save and clear bindings to symbols #a, $N, $X; restore when done */
    t_pd *boundx = s__X.s_thing, *bounda = asym->s_thing, 
        *boundn = s__N.s_thing;
    asym->s_thing = 0;
    s__X.s_thing = &x->gl_obj.te_g.g_pd;
    s__N.s_thing = &pd_canvasMaker;

    canvas_editmode(x, 1.);
    canvas_deselectAll(x);
    for (g2 = x->gl_graphics, nbox = 0; g2; g2 = g2->g_next) nbox++;

    editor_pasteOnset = nbox;
    editor_pasteCanvas = x;

    buffer_eval(b, 0, 0, 0);
    for (g2 = x->gl_graphics, count = 0; g2; g2 = g2->g_next, count++)
        if (count >= nbox)
            canvas_selectObject(x, g2);
    editor_pasteCanvas = 0;
    dsp_resume(dspstate);
    canvas_dirty(x, 1);
    sys_vGui("::ui_patch::updateScrollRegion .x%lx.c\n", x);
    glist_donewloadbangs(x);
    asym->s_thing = bounda;
    s__X.s_thing = boundx;
    s__N.s_thing = boundn;
}

void canvas_paste(t_glist *x)
{
    if (!x->gl_editor)
        return;
    if (x->gl_editor->e_selectedText)
    {
        /* simulate keystrokes as if the copy buffer were typed in. */
        // sys_vGui("::ui_object::pasteText .x%lx\n", x);
    }
    else
    {
        // canvas_setundo(x, canvas_undo_paste, canvas_undo_set_paste(x), "paste");
        canvas_dopaste(x, editor_buffer);
    }
}

void canvas_duplicate(t_glist *x)
{
    if (!x->gl_editor)
        return;
    if (x->gl_editor->e_action == ACTION_NONE && x->gl_editor->e_selectedObjects)
    {
        t_selection *y;
        canvas_copy(x);
        // canvas_setundo(x, canvas_undo_paste, canvas_undo_set_paste(x), "duplicate");
        canvas_dopaste(x, editor_buffer);
        for (y = x->gl_editor->e_selectedObjects; y; y = y->sel_next)
            gobj_displace(y->sel_what, x,
                10, 10);
        canvas_dirty(x, 1);
    }
}

void canvas_selectall(t_glist *x)
{
    t_gobj *y;
    if (!x->gl_editor)
        return;
    if (!x->gl_isEditMode)
        canvas_editmode(x, 1);
            /* if everyone is already selected deselect everyone */
    if (!canvas_getNumberOfUnselectedObjects (x))
        canvas_deselectAll(x);
    else for (y = x->gl_graphics; y; y = y->g_next)
    {
        if (!canvas_isObjectSelected(x, y))
            canvas_selectObject(x, y);
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
        if (o = canvas_castToObjectIfPatchable (y)) { rtext_new (glist, o); }
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
    while (text = glist->gl_editor->e_text) { rtext_free (text); }
    
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
