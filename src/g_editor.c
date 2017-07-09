
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_box *editor_boxFetch (t_editor *x, t_object *object)
{
    t_box *box = NULL;
    
    for (box = x->e_boxes; box && box->box_object != object; box = box->box_next) { }
    
    PD_ASSERT (box);
    
    return box;
}

void editor_boxAdd (t_editor *x, t_object *object)
{
    t_box *box = (t_box *)PD_MEMORY_GET (sizeof (t_box));

    box->box_next   = x->e_boxes;
    box->box_object = object;
    box->box_owner  = x->e_owner;
    
    buffer_toStringUnzeroed (object_getBuffer (object), &box->box_string, &box->box_stringSizeInBytes);
    
    {
    //
    t_glist *view = glist_getView (x->e_owner);
    t_error err = string_sprintf (box->box_tag, BOX_TAG_SIZE, "%s.%lxBOX", glist_getTagAsString (view), box);
    PD_UNUSED (err); PD_ASSERT (!err);
    //
    }
    
    x->e_boxes = box;
}

void editor_boxRemove (t_editor *x, t_box *box)
{
    editor_boxUnselect (x, box);
    
    if (x->e_boxes == box) { x->e_boxes = box->box_next; }
    else {
        t_box *t = NULL;
        for (t = x->e_boxes; t; t = t->box_next) {
            if (t->box_next == box) { t->box_next = box->box_next; break; }
        }
    }

    PD_MEMORY_FREE (box->box_string);
    PD_MEMORY_FREE (box);
}

void editor_boxSelect (t_editor *x, t_box *box)
{
    x->e_selectedBox = box; x->e_isSelectedBoxDirty = 0;
}

void editor_boxUnselect (t_editor *x, t_box *box)
{
    if (x->e_selectedBox == box) { editor_boxSelect (x, NULL); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editor_selectionAdd (t_editor *x, t_gobj *y)
{
    t_selection *s = (t_selection *)PD_MEMORY_GET (sizeof (t_selection));
    
    s->sel_next = x->e_selectedObjects;
    s->sel_what = y;
    
    x->e_selectedObjects = s;
}

int editor_selectionRemove (t_editor *x, t_gobj *y)
{
    t_selection *s1 = NULL;
    t_selection *s2 = NULL;
    
    s1 = x->e_selectedObjects;
    
    if (selection_getObject (s1) == y) {
        x->e_selectedObjects = selection_getNext (x->e_selectedObjects);
        PD_MEMORY_FREE (s1);
        return 1;
        
    } else {
        for (; (s2 = selection_getNext (s1)); (s1 = s2)) {
            if (selection_getObject (s2) == y) {
                s1->sel_next = selection_getNext (s2);
                PD_MEMORY_FREE (s2);
                return 1;
            }
        }
    }
    
    return 0;
}

void editor_selectionDeplace (t_editor *x)
{
    int deltaX = drag_getMoveX (editor_getDrag (x));
    int deltaY = drag_getMoveY (editor_getDrag (x));
    
    if (snap_hasSnapToGrid()) {
        if (drag_hasMovedOnce (editor_getDrag (x))) { glist_objectSnapSelected (x->e_owner); }
    }
    
    if (deltaX || deltaY) { glist_objectDisplaceSelected (x->e_owner, deltaX, deltaY); }
        
    drag_close (editor_getDrag (x));
}

void editor_selectionCacheLines (t_editor *x)
{
    t_outconnect *connection = NULL;
    t_traverser t;
    
    buffer_clear (x->e_cachedLines);
    
    traverser_start (&t, x->e_owner);
    
    while ((connection = traverser_next (&t))) {
    //
    int s1 = glist_objectIsSelected (x->e_owner, cast_gobj (traverser_getSource (&t)));
    int s2 = glist_objectIsSelected (x->e_owner, cast_gobj (traverser_getDestination (&t)));
    
    if (s1 != s2) {
    //
    buffer_vAppend (x->e_cachedLines, "ssiiii;",
        sym___hash__X, 
        sym_connect,
        glist_objectGetIndexOf (x->e_owner, cast_gobj (traverser_getSource (&t))),
        traverser_getIndexOfOutlet (&t),
        glist_objectGetIndexOf (x->e_owner, cast_gobj (traverser_getDestination (&t))),
        traverser_getIndexOfInlet (&t));
    //
    }
    //
    }
}

void editor_selectionRestoreLines (t_editor *x)
{
    instance_loadSnippet (x->e_owner, x->e_cachedLines);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editor_selectedLineSet (t_editor *x, t_outconnect *connection, int m, int i, int n, int j)
{
    x->e_selectedLineConnection = connection;
    
    x->e_selectedLine[0] = m;
    x->e_selectedLine[1] = i;
    x->e_selectedLine[2] = n;
    x->e_selectedLine[3] = j;
}

void editor_selectedLineReset (t_editor *x)
{
    editor_selectedLineSet (x, NULL, 0, 0, 0, 0);
}

void editor_selectedLineDisconnect (t_editor *x)
{
    int m = x->e_selectedLine[0];
    int i = x->e_selectedLine[1];
    int n = x->e_selectedLine[2];
    int j = x->e_selectedLine[3];
    
    glist_lineDisconnect (x->e_owner, m, i, n, j);
    
    editor_selectedLineReset (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editor_motionSet (t_editor *x, t_gobj *y, t_motionfn callback, int a, int b)
{
    PD_ASSERT (callback);
    
    editor_startAction (x, ACTION_PASS, a, b);
    
    x->e_grabbed  = y;
    x->e_fnMotion = callback;
}

void editor_motionReset (t_editor *x)
{
    drag_close (editor_getDrag (x));
    
    x->e_grabbed  = NULL;
    x->e_fnMotion = NULL;
    
    editor_resetAction (x);
}

void editor_motionUnset (t_editor *x, t_gobj *y)
{
    if (!x->e_grabbed || x->e_grabbed == y) { editor_motionReset (x); }
}

void editor_motionProceed (t_editor *x, int deltaX, int deltaY, int m)
{
    if (x->e_fnMotion) { (*x->e_fnMotion) (cast_pd (x->e_grabbed), deltaX, deltaY, m); }
    else { 
        PD_BUG;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_editor *editor_new (t_glist *owner)
{
    t_editor *x = (t_editor *)PD_MEMORY_GET (sizeof (t_editor));
    
    t_gobj *y = NULL;
    
    x->e_owner       = owner;
    x->e_proxy       = proxy_new (cast_pd (owner));
    x->e_cachedLines = buffer_new();
    
    for (y = owner->gl_graphics; y; y = y->g_next) {
        if (cast_objectIfConnectable (y)) { editor_boxAdd (x, cast_object (y)); }
    }
    
    return x;
}

void editor_free (t_editor *x)
{
    t_box *box = NULL;
    
    while ((box = x->e_boxes)) { editor_boxRemove (x, box); }
    
    buffer_free (x->e_cachedLines);
    proxy_release (x->e_proxy);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
