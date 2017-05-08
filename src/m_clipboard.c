
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define CLIPBOARD_PASTE_OFFSET      20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void clipboard_copy (t_clipboard *x, t_glist *glist)
{
    if (editor_hasSelection (glist_getEditor (glist))) {
    //
    t_buffer *b = buffer_new();

    t_gobj *y = NULL;
    t_outconnect *connection = NULL;
    t_traverser t;
    
    x->cb_count = 0;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (glist_objectIsSelected (glist, y)) { gobj_save (y, b); }
    }
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    int m = glist_objectIsSelected (glist, cast_gobj (traverser_getSource (&t)));
    int n = glist_objectIsSelected (glist, cast_gobj (traverser_getDestination (&t)));
    
    if (m && n) {
        buffer_vAppend (b, "ssiiii;", 
            sym___hash__X, 
            sym_connect,
            glist_objectGetIndexAmongSelected (glist, cast_gobj (traverser_getSource (&t))),
            traverser_getIndexOfOutlet (&t),
            glist_objectGetIndexAmongSelected (glist, cast_gobj (traverser_getDestination (&t))),
            traverser_getIndexOfInlet (&t));
    }
    //
    }
    
    buffer_free (x->cb_buffer); 
    x->cb_buffer = b;
    //
    }
}

void clipboard_paste (t_clipboard *x, t_glist *glist)
{
    t_gobj *y = NULL;
    t_selection *s = NULL;
    int i = 0;
    int n = (++x->cb_count) * CLIPBOARD_PASTE_OFFSET;
    int state = dsp_suspend();
    int alreadyThere = glist_objectGetNumberOf (glist);
    int isDirty = 0;
    
    glist_deselectAll (glist);
    
    snippet_addOffsetToLine (x->cb_buffer, alreadyThere);
    
        instance_loadSnippet (glist, x->cb_buffer);
    
    snippet_substractOffsetToLine (x->cb_buffer, alreadyThere);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (i >= alreadyThere) { glist_objectSelect (glist, y); isDirty = 1; }
        i++;
    }
    
    dsp_resume (state);
        
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
        y = selection_getObject (s); gobj_displaced (y, glist, n, n);
        if (pd_class (y) == canvas_class) { 
            glist_loadbang (cast_glist (y)); 
        }
    }
    
    if (isDirty) { glist_setDirty (glist, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void clipboard_init (t_clipboard *x)
{
    x->cb_count  = 0;
    x->cb_buffer = buffer_new();
}

void clipboard_destroy (t_clipboard *x)
{
    if (x->cb_buffer) { buffer_free (x->cb_buffer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
