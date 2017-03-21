
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

static void clipboard_addOffsetToLineConnections (t_clipboard *x, int i)
{
    t_iterator *iter = iterator_new (buffer_size (x->cb_buffer), buffer_atoms (x->cb_buffer));
    t_atom *atoms = NULL;
    int count;
    
    int k = 0;
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    if (count >= 2) {
    //
    t_symbol *s = atom_getSymbolAtIndex (1, count, atoms);
    
    if (s == sym_canvas)  { k++; }      /* Connections in subpatches must not be changed. */
    if (s == sym_restore) { k--; }
    if (s == sym_connect) {
    //
    if (!k && count == 6) {
    //
    t_float m = atom_getFloat (atoms + 2);
    t_float n = atom_getFloat (atoms + 4);
    SET_FLOAT (atoms + 2, m + i);
    SET_FLOAT (atoms + 4, n + i);
    //
    }
    //
    }
    //
    }
    //
    }
    
    iterator_free (iter);
}

static void clipboard_substractOffsetToLineConnections (t_clipboard *x, int i)
{
    clipboard_addOffsetToLineConnections (x, -i);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void clipboard_copy (t_clipboard *x, t_glist *glist)
{
    if (glist->gl_editor->e_selectedObjects) {
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
    int alreadyThere = 0;
    int i = 0;
    int n = (++x->cb_count) * CLIPBOARD_PASTE_OFFSET;
    int state = dsp_suspend();
     
    canvas_deselectAll (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) { alreadyThere++; }
    
    clipboard_addOffsetToLineConnections (x, alreadyThere);
    
        instance_stackEval (glist, x->cb_buffer);
    
    clipboard_substractOffsetToLineConnections (x, alreadyThere);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (i >= alreadyThere) { canvas_selectObject (glist, y); }
        i++;
    }
    
    dsp_resume (state);
        
    for (s = glist->gl_editor->e_selectedObjects; s; s = selection_getNext (s)) {
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
