
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_makeLineEnd      (t_glist *, int, int);
void glist_makeLineBegin    (t_glist *, int, int);
void glist_motionResize     (t_glist *, int, int);
void glist_mouse            (t_glist *, int, int, int, int);

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
        if (editor_hasSelectedLine (glist_getEditor (glist)))   { glist_lineDeleteSelected (glist); }
        else if (editor_hasSelection (glist_getEditor (glist))) { 
            glist_objectRemoveSelected (glist); 
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
        glist_makeLineBegin (glist, a, b);
        
    } else if (action == ACTION_REGION)  {
        glist_selectLassoBegin (glist, a, b);
        
    } else if (action == ACTION_PASS)    {
        editor_motionProceed (glist_getEditor (glist), deltaX, deltaY, m);
        drag_setStart (editor_getDrag (glist_getEditor (glist)), a, b);
        
    } else if (action == ACTION_DRAG)    {
        t_box *text = editor_getSelectedBox (glist_getEditor (glist));
        if (text) { box_mouse (text, deltaX, deltaY, BOX_DRAG); }
                
    } else if (action == ACTION_RESIZE)  {
        glist_motionResize (glist, a, b);

    } else {
        glist_mouse (glist, a, b, m, 0);
    }
}

void canvas_mouse (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int m = (int)atom_getFloatAtIndex (2, argc, argv);

    glist_mouse (glist, a, b, m, 1);
}

void canvas_mouseUp (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int action = editor_getAction (glist_getEditor (glist));
    
    if (action == ACTION_CONNECT)     { glist_makeLineEnd (glist, a, b); }
    else if (action == ACTION_REGION) { glist_selectLassoEnd (glist, a, b); }
    else if (action == ACTION_MOVE)   {
    //
    if (glist_objectGetNumberOfSelected (glist) == 1) {
        gobj_activated (selection_getObject (editor_getSelection (glist_getEditor (glist))), glist, 1);
    }
    //
    }

    editor_resetAction (glist_getEditor (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
