
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
static int                  editor_mouseUpX;                /* Shared. */
static int                  editor_mouseUpY;                /* Shared. */
static double               editor_mouseUpClickTime;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define EDITOR_MODIFIER_NONE            0
#define EDITOR_MODIFIER_SHIFT           1
#define EDITOR_MODIFIER_CTRL            2
#define EDITOR_MODIFIER_ALT             4
#define EDITOR_MODIFIER_RIGHT           8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define EDITOR_DBLCLICK_INTERVAL        0.25

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
    
    if (sym__key->s_thing && isDown)    { pd_float (sym__key->s_thing,   (t_float)n); }
    if (sym__keyup->s_thing && !isDown) { pd_float (sym__keyup->s_thing, (t_float)n); }
    
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

    if (glist->gl_editor->e_onMotion == ACTION_MOVE) { glist->gl_editor->e_onMotion = ACTION_NONE; }
    
    if (glist->gl_editor->e_grabbed && glist->gl_editor->e_fnKey) {
        (*glist->gl_editor->e_fnKey) (glist->gl_editor->e_grabbed, (t_float)n);
        
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
    int action = glist->gl_editor->e_onMotion;
    
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
        canvas_doclick (glist, (int)positionX, (int)positionY, 0, (int)modifier, 0);
    }
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

    /* right-clicking on a canvas object pops up a menu. */
static void canvas_rightclick(t_glist *x, int xpos, int ypos, t_gobj *y)
{
    int canprop, canopen;
    canprop = (!y || (y && class_hasPropertiesFunction (pd_class(&y->g_pd))));
    canopen = (y && class_hasMethod (pd_class (&y->g_pd), sym_open));
    sys_vGui("::ui_menu::showPopup .x%lx %d %d %d %d\n",
        x, xpos, ypos, canprop, canopen);
}

    /* mouse click */
void canvas_doclick(t_glist *x, int xpos, int ypos, int which, int mod, int doit)
{
    t_gobj *y;
    int shiftmod, runmode, altmod, doublemod = 0, rightclick;
    int x1=0, y1=0, x2=0, y2=0, clickreturned = 0;
    
    if (!x->gl_editor)
    {
        PD_BUG;
        return;
    }
    
    shiftmod = (mod & EDITOR_MODIFIER_SHIFT);
    runmode = ((mod & EDITOR_MODIFIER_CTRL) || (!x->gl_isEditMode));
    altmod = (mod & EDITOR_MODIFIER_ALT);
    rightclick = (mod & EDITOR_MODIFIER_RIGHT);

    // canvas_undo_already_set_move = 0;

            /* if keyboard was grabbed, notify grabber and cancel the grab */
    if (doit && x->gl_editor->e_grabbed && x->gl_editor->e_fnKey)
    {
        (* x->gl_editor->e_fnKey) (x->gl_editor->e_grabbed, 0);
        glist_grab(x, 0, 0, 0, 0, 0);
    }

    if (doit && !runmode && xpos == editor_mouseUpX && ypos == editor_mouseUpY &&
        sys_getRealTimeInSeconds() - editor_mouseUpClickTime < EDITOR_DBLCLICK_INTERVAL)
            doublemod = 1;
    //x->gl_editor->e_lastmoved = 0;
    if (doit)
    {
        x->gl_editor->e_grabbed = 0;
        x->gl_editor->e_onMotion = ACTION_NONE;
    }
    /* post("click %d %d %d %d", xpos, ypos, which, mod); */
    
    if (x->gl_editor->e_onMotion != ACTION_NONE)
        return;

    x->gl_editor->e_previousX = xpos;
    x->gl_editor->e_previousY = ypos;

    if (runmode && !rightclick)
    {
        for (y = x->gl_graphics; y; y = y->g_next)
        {
                /* check if the object wants to be clicked */
            if (gobj_hit(y, x, xpos, ypos, &x1, &y1, &x2, &y2)
                && (clickreturned = gobj_click(y, x, xpos, ypos,
                    shiftmod, ((mod & EDITOR_MODIFIER_CTRL) && (!x->gl_isEditMode)) || altmod,
                        0, doit)))
                            break;
        }
        if (!doit)
        {
            if (y)
                canvas_setCursorType(x, clickreturned);
            else canvas_setCursorType(x, CURSOR_NOTHING);
        }
        return;
    }
        /* if not a runmode left click, fall here. */
    if (y = canvas_getHitObject(x, xpos, ypos, &x1, &y1, &x2, &y2))
    {
        t_object *ob = canvas_castToObjectIfPatchable(&y->g_pd);
            /* check you're in the rectangle */
        ob = canvas_castToObjectIfPatchable(&y->g_pd);
        if (rightclick)
            canvas_rightclick(x, xpos, ypos, y);
        else if (shiftmod)
        {
            if (doit)
            {
                t_boxtext *rt;
                if (ob && (rt = x->gl_editor->e_selectedText) &&
                    rt == glist_findrtext(x, ob))
                {
                    rtext_mouse(rt, xpos - x1, ypos - y1, BOX_TEXT_SHIFT);
                    x->gl_editor->e_onMotion = ACTION_DRAG;
                    x->gl_editor->e_previousX = x1;
                    x->gl_editor->e_previousY = y1;
                }
                else
                {
                    if (canvas_isObjectSelected(x, y))
                        canvas_deselectObject(x, y);
                    else canvas_selectObject(x, y);
                }
            }
        }
        else
        {
            int noutlet;
                /* resize?  only for "true" text boxes or canvases*/
            if (ob && !x->gl_editor->e_selectedObjects &&
                (ob->te_g.g_pd->c_behavior == &text_widgetBehavior ||
                    canvas_castToGlistChecked(&ob->te_g.g_pd)) &&
                        xpos >= x2-4 && ypos < y2-4)
            {
                if (doit)
                {
                    if (!canvas_isObjectSelected(x, y))
                    {
                        canvas_deselectAll(x);
                        canvas_selectObject(x, y);
                    }
                    x->gl_editor->e_onMotion = ACTION_RESIZE;
                    x->gl_editor->e_previousX = x1;
                    x->gl_editor->e_previousY = y1;
                    x->gl_editor->e_newX = xpos;
                    x->gl_editor->e_newY = ypos;
                }                                   
                else canvas_setCursorType(x, CURSOR_EDIT_RESIZE);
            }
                /* look for an outlet */
            else if (ob && (noutlet = object_numberOfOutlets(ob)) && ypos >= y2-4)
            {
                int width = x2 - x1;
                int nout1 = (noutlet > 1 ? noutlet - 1 : 1);
                int closest = ((xpos-x1) * (nout1) + width/2)/width;
                int hotspot = x1 +
                    (width - INLETS_WIDTH) * closest / (nout1);
                if (closest < noutlet &&
                    xpos >= (hotspot-1) && xpos <= hotspot + (INLETS_WIDTH+1))
                {
                    if (doit)
                    {
                        int issignal = object_isSignalOutlet(ob, closest);
                        x->gl_editor->e_onMotion = ACTION_CONNECT;
                        x->gl_editor->e_previousX = xpos;
                        x->gl_editor->e_previousY = ypos;
                        sys_vGui(
                          ".x%lx.c create line %d %d %d %d -width %d -tags TEMPORARY\n",
                                x, xpos, ypos, xpos, ypos,
                                    (issignal ? 2 : 1));
                    }                                   
                    else canvas_setCursorType(x, CURSOR_EDIT_CONNECT);
                }
                else if (doit)
                    goto nooutletafterall;
                else canvas_setCursorType(x, CURSOR_EDIT_NOTHING);
            }
                /* not in an outlet; select and move */
            else if (doit)
            {
                t_boxtext *rt;
                    /* check if the box is being text edited */
            nooutletafterall:
                if (ob && (rt = x->gl_editor->e_selectedText) &&
                    rt == glist_findrtext(x, ob))
                {
                    rtext_mouse(rt, xpos - x1, ypos - y1,
                        (doublemod ? BOX_TEXT_DOUBLE : BOX_TEXT_DOWN));
                    x->gl_editor->e_onMotion = ACTION_DRAG;
                    x->gl_editor->e_previousX = x1;
                    x->gl_editor->e_previousY = y1;
                }
                else
                {
                        /* otherwise select and drag to displace */
                    if (!canvas_isObjectSelected(x, y))
                    {
                        canvas_deselectAll(x);
                        canvas_selectObject(x, y);
                    }
                    x->gl_editor->e_onMotion = ACTION_MOVE;
                }
            }
            else canvas_setCursorType(x, CURSOR_EDIT_NOTHING);
        }
        return;
    }
        /* if right click doesn't hit any boxes, call rightclick
            routine anyway */
    if (rightclick)
        canvas_rightclick(x, xpos, ypos, 0);

        /* if not an editing action, and if we didn't hit a
        box, set cursor and return */
    if (runmode || rightclick)
    {
        canvas_setCursorType(x, CURSOR_NOTHING);
        return;
    }
        /* having failed to find a box, we try lines now. */
    if (!runmode && !altmod && !shiftmod)
    {
        t_linetraverser t;
        t_outconnect *oc;
        t_float fx = xpos, fy = ypos;
        t_glist *glist2 = canvas_getView(x);
        canvas_traverseLinesStart(&t, glist2);
        while (oc = canvas_traverseLinesNext(&t))
        {
            t_float lx1 = t.tr_lineStartX, ly1 = t.tr_lineStartY,
                lx2 = t.tr_lineEndX, ly2 = t.tr_lineEndY;
            t_float area = (lx2 - lx1) * (fy - ly1) -
                (ly2 - ly1) * (fx - lx1);
            t_float dsquare = (lx2-lx1) * (lx2-lx1) + (ly2-ly1) * (ly2-ly1);
            if (area * area >= 50 * dsquare) continue;
            if ((lx2-lx1) * (fx-lx1) + (ly2-ly1) * (fy-ly1) < 0) continue;
            if ((lx2-lx1) * (lx2-fx) + (ly2-ly1) * (ly2-fy) < 0) continue;
            if (doit)
            {
                canvas_selectLine(glist2, oc, 
                    canvas_getIndexOfObject(glist2, &t.tr_srcObject->te_g), t.tr_srcIndexOfOutlet,
                    canvas_getIndexOfObject(glist2, &t.tr_destObject->te_g), t.tr_destIndexOfInlet);
            }
            canvas_setCursorType(x, CURSOR_EDIT_DISCONNECT);
            return;
        }
    }
    canvas_setCursorType(x, CURSOR_EDIT_NOTHING);
    if (doit)
    {
        if (!shiftmod) canvas_deselectAll(x);
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags TEMPORARY\n",
              x, xpos, ypos, xpos, ypos);
        x->gl_editor->e_previousX = xpos;
        x->gl_editor->e_previousY = ypos;
        x->gl_editor->e_onMotion = ACTION_REGION;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_mouse(t_glist *x, t_float xpos, t_float ypos, t_float which, t_float mod)
{
    canvas_doclick (x, (int)xpos, (int)ypos, (int)which, (int)mod, 1);
}

void canvas_mouseup(t_glist *x,
    t_float fxpos, t_float fypos, t_float fwhich)
{
    int xpos = fxpos, ypos = fypos, which = fwhich;
    /* post("mouseup %d %d %d", xpos, ypos, which); */
    if (!x->gl_editor)
    {
        PD_BUG;
        return;
    }

    editor_mouseUpClickTime = sys_getRealTimeInSeconds();
    editor_mouseUpX = xpos;
    editor_mouseUpY = ypos;

    if (x->gl_editor->e_onMotion == ACTION_CONNECT)
        canvas_makingLine (x, xpos, ypos, 1);
    else if (x->gl_editor->e_onMotion == ACTION_REGION)
        canvas_selectingByLasso(x, xpos, ypos, 1);
    else if (x->gl_editor->e_onMotion == ACTION_MOVE ||
        x->gl_editor->e_onMotion == ACTION_RESIZE)
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
                x->gl_editor->e_onMotion = ACTION_NONE;
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

    x->gl_editor->e_onMotion = ACTION_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    if (x->gl_editor->e_onMotion == ACTION_NONE && x->gl_editor->e_selectedObjects)
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
