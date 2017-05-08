
/* Copyright (c) 1997-2017 Miller Puckette and others. */

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

void canvas_copy (t_glist *glist)
{
    if (glist_hasEditMode (glist)) {
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
        sys_vGui ("::ui_bind::pasteText %s\n", glist_getTagAsString (glist));
    } else {
        clipboard_paste (instance_getClipboard(), glist);
    }
    //
    }
}

void canvas_duplicate (t_glist *glist)
{
    if (glist_hasEditMode (glist)) {
    //
    canvas_copy (glist);
    clipboard_paste (instance_getClipboard(), glist);
    //
    }
}

void canvas_selectAll (t_glist *glist)
{
    if (glist_hasEditMode (glist)) {
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
