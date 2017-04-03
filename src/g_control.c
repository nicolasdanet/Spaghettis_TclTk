
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

void canvas_cut (t_glist *glist)
{
    if (!glist_hasEditMode (glist)) { return; }
    else {
    //
    if (editor_hasSelectedLine (glist_getEditor (glist))) { glist_lineDeleteSelected (glist); }
    else if (editor_hasSelectedBox (glist_getEditor (glist))) {
        canvas_copy (glist);
        box_key (editor_getSelectedBox (glist_getEditor (glist)), (t_keycode)127, sym_Delete);
        glist_setDirty (glist, 1);
        
    } else if (editor_hasSelection (glist_getEditor (glist))) {
        canvas_copy (glist);
        glist_objectRemoveSelected (glist);
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
        if (!glist_objectIsSelected (glist, y)) { glist_objectSelect (glist, y); }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
