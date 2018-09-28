
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_undo (t_glist *glist)
{
    if (glist_undoIsOk (glist)) { undomanager_undo (glist_getUndoManager (glist)); glist_updateUndo (glist); }
}

void canvas_redo (t_glist *glist)
{
    if (glist_undoIsOk (glist)) { undomanager_redo (glist_getUndoManager (glist)); glist_updateUndo (glist); }
}

void canvas_update (t_glist *glist)
{
    glist_updateUndo (glist); glist_updateEncapsulate (glist);
}

void canvas_copy (t_glist *glist)
{
    if (glist_hasEditMode (glist)) {
    //
    if (editor_hasSelectedBox (glist_getEditor (glist))) {
        char *t = NULL;
        int s = 0;
        box_getSelection (editor_getSelectedBox (glist_getEditor (glist)), &t, &s);
        gui_add ("clipboard clear\n");
        gui_vAdd ("clipboard append {%.*s}\n", s, t);       /* < http://stackoverflow.com/a/13289324 > */
        
    } else {
        clipboard_copy (glist);
    }
    //
    }
}

void canvas_cut (t_glist *glist)
{
    if (glist_hasEditMode (glist)) {
    //
    if (editor_hasSelectedLine (glist_getEditor (glist))) { glist_lineDeleteSelected (glist); }
    else {
    //
    canvas_copy (glist);
    
    if (editor_hasSelectedBox (glist_getEditor (glist)))  {
        box_key (editor_getSelectedBox (glist_getEditor (glist)), (t_keycode)127, sym_Delete);
    
    } else {
        if (glist_undoIsOk (glist)) { glist_undoAppend (glist, undocut_new()); }
        glist_objectRemoveSelected (glist);
    }
    //
    }
    //
    }
}

void canvas_paste (t_glist *glist)
{
    if (glist_hasEditMode (glist)) { 
    //
    if (editor_hasSelectedBox (glist_getEditor (glist))) {
        gui_vAdd ("::ui_bind::pasteText %s\n", glist_getTagAsString (glist));
    } else {
        clipboard_paste (glist, 0);
    }
    //
    }
}

void canvas_duplicate (t_glist *glist)
{
    if (glist_hasEditMode (glist)) {
    //
    glist_cancelEditingBox (glist);
    canvas_copy (glist);
    clipboard_paste (glist, 1);
    //
    }
}

void canvas_encapsulate (t_glist *glist)
{
    if (glist_hasEditMode (glist)) { encapsulate_encapsulate (glist); }
}

void canvas_deencapsulate (t_glist *glist)
{
    if (glist_hasEditMode (glist) && (glist_objectGetNumberOfSelected (glist) == 1)) {
    //
    t_gobj *y = selection_getObject (editor_getSelection (glist_getEditor (glist)));
    
    if (gobj_isCanvas (y) && !glist_isArray (cast_glist (y))) { encapsulate_deencapsulate (cast_glist (y)); }
    //
    }
}

void canvas_selectAll (t_glist *glist)
{
    if (glist_hasEditMode (glist)) {
    //
    t_gobj *y = NULL;

    glist_cancelEditingBox (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (!glist_objectIsSelected (glist, y)) { glist_objectSelect (glist, y); }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_snap (t_glist *glist)
{
    glist_cancelEditingBox (glist);
    glist_objectSnapSelected (glist, 1);
}

void canvas_bringToFront (t_glist *glist)
{
    glist_cancelEditingBox (glist);
    glist_objectMoveSelected (glist, 0);
}

void canvas_sendToBack (t_glist *glist)
{
    glist_cancelEditingBox (glist);
    glist_objectMoveSelected (glist, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
