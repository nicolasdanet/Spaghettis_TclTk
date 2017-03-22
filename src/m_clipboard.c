
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
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define CLIPBOARD_PASTE_OFFSET      20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void clipboard_copy (t_clipboard *x, t_glist *glist)
{
    if (editor_hasSelection (glist->gl_editor)) {
    //
    t_buffer *b = buffer_new();

    t_gobj *y = NULL;
    t_outconnect *connection = NULL;
    t_traverser t;
    
    x->cb_count = 0;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (canvas_isObjectSelected (glist, y)) { gobj_save (y, b); }
    }
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    int m = canvas_isObjectSelected (glist, cast_gobj (traverser_getSource (&t)));
    int n = canvas_isObjectSelected (glist, cast_gobj (traverser_getDestination (&t)));
    
    if (m && n) {
        buffer_vAppend (b, "ssiiii;", 
            sym___hash__X, 
            sym_connect,
            canvas_getIndexOfObjectAmongSelected (glist, cast_gobj (traverser_getSource (&t))),
            traverser_getIndexOfOutlet (&t),
            canvas_getIndexOfObjectAmongSelected (glist, cast_gobj (traverser_getDestination (&t))),
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
    int alreadyThere = canvas_getNumberOfObjects (glist);
    
    canvas_deselectAll (glist);
    
    snippet_addOffsetToLine (x->cb_buffer, alreadyThere);
    
        instance_stackEval (glist, x->cb_buffer);
    
    snippet_substractOffsetToLine (x->cb_buffer, alreadyThere);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (i >= alreadyThere) { canvas_selectObject (glist, y); }
        i++;
    }
    
    dsp_resume (state);
        
    for (s = editor_getSelection (glist->gl_editor); s; s = selection_getNext (s)) {
        y = selection_getObject (s); gobj_displaced (y, glist, n, n);
        if (pd_class (y) == canvas_class) { 
            canvas_loadbang (cast_glist (y)); 
        }
    }
    
    canvas_dirty (glist, 1);
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
