
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int clipboard_count;                 /* Static. */

static t_buffer *clipboard_buffer;          /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void clipboard_copy (t_glist *glist)
{
    if (editor_hasSelection (glist_getEditor (glist))) {
    //
    t_buffer *b = buffer_new();

    t_gobj *y = NULL;
    t_outconnect *connection = NULL;
    t_traverser t;
    
    clipboard_count = 0;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (glist_objectIsSelected (glist, y)) { gobj_save (y, b); }
    }
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    t_gobj *o = cast_gobj (traverser_getSource (&t));
    t_gobj *d = cast_gobj (traverser_getDestination (&t));
    int m = glist_objectIsSelected (glist, o);
    int n = glist_objectIsSelected (glist, d);
    
    if (m && n) {
    //
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_connect);
    buffer_appendFloat (b, glist_objectGetIndexAmongSelected (glist, o));
    buffer_appendFloat (b, traverser_getIndexOfOutlet (&t));
    buffer_appendFloat (b, glist_objectGetIndexAmongSelected (glist, d));
    buffer_appendFloat (b, traverser_getIndexOfInlet (&t));
    buffer_appendSemicolon (b);
    //
    }
    //
    }
    
    buffer_free (clipboard_buffer);
    
    clipboard_buffer = b;
    //
    }
}

void clipboard_paste (t_glist *glist)
{
    t_gobj *y = NULL;
    t_selection *s = NULL;
    int i = 0;
    int n = (++clipboard_count) * snap_getStep();
    int state = dsp_suspend();
    int alreadyThere = glist_objectGetNumberOf (glist);
    int isDirty = 0;
    
    glist_deselectAll (glist);
    
    snippet_addOffsetToLines (clipboard_buffer, alreadyThere);
    snippet_renameArrays (clipboard_buffer, glist);
    
        instance_loadSnippet (glist, clipboard_buffer);
    
    snippet_substractOffsetToLines (clipboard_buffer, alreadyThere);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (i >= alreadyThere) { glist_objectSelect (glist, y); isDirty = 1; }
        i++;
    }
    
    dsp_resume (state);
        
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
        y = selection_getObject (s); gobj_displaced (y, glist, n, n);
        if (gobj_isCanvas (y)) { glist_loadbang (cast_glist (y)); }
    }
    
    if (isDirty) { glist_setDirty (glist, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void clipboard_initialize (void)
{
    clipboard_count  = 0;
    clipboard_buffer = buffer_new();
}

void clipboard_release (void)
{
    if (clipboard_buffer) { buffer_free (clipboard_buffer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
