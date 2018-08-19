
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Separate selected objects from uneselected ones and move them to the end. */

static void glist_sortSelected (t_glist *glist)
{
    t_gobj *sHead = NULL;
    t_gobj *sTail = NULL;
    t_gobj *uHead = NULL;
    t_gobj *uTail = NULL;
    t_gobj *t1 = NULL;
    t_gobj *t2 = NULL;

    for (t1 = glist->gl_graphics; t1; t1 = t2) {
    //
    t2 = t1->g_next;
    
    if (glist_objectIsSelected (glist, t1)) {
    
        if (sTail) { sTail->g_next = t1; sTail = t1; t1->g_next = NULL; }
        else {
            sHead = sTail = t1; sTail->g_next = NULL;
        }
        
    } else {
    
        if (uTail) { uTail->g_next = t1; uTail = t1; t1->g_next = NULL; }
        else {
            uHead = uTail = t1; uTail->g_next = NULL;
        }
    }
    //
    }

    if (!uHead) { glist->gl_graphics = sHead; }
    else {
        glist->gl_graphics = uHead; uTail->g_next = sHead; 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_selectLassoOverlap (t_glist *glist, t_rectangle *r)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {

        t_rectangle t;
        
        gobj_getRectangle (y, glist, &t);
        
        if (rectangle_overlap (r, &t) && !glist_objectIsSelected (glist, y)) {
            glist_objectSelect (glist, y);
        }
    }
}

static void glist_selectLassoProceed (t_glist *glist, int a, int b, int end)
{
    if (!end) { glist_updateLasso (glist, a, b); }
    else {
    //
    t_rectangle r;
    
    rectangle_set (&r, 
        drag_getStartX (editor_getDrag (glist_getEditor (glist))), 
        drag_getStartY (editor_getDrag (glist_getEditor (glist))), 
        a, 
        b);
    
    glist_selectLassoOverlap (glist, &r);
    editor_resetAction (glist_getEditor (glist));
    glist_eraseLasso (glist);
    //
    }
}

void glist_selectLassoBegin (t_glist *glist, int a, int b)
{
    glist_selectLassoProceed (glist, a, b, 0);
}

void glist_selectLassoEnd (t_glist *glist, int a, int b)
{
    glist_selectLassoProceed (glist, a, b, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_deselectAllRecursive (t_gobj *y)
{
    if (gobj_isCanvas (y)) { 
    //
    t_gobj *o = NULL;
    for (o = cast_glist (y)->gl_graphics; o; o = o->g_next) { glist_deselectAllRecursive (o); }
    glist_deselectAll (cast_glist (y));
    //
    }
}

int glist_deselectAll (t_glist *glist)
{
    int k = 0;
    t_selection *s = NULL;
    
    while ((s = editor_getSelection (glist_getEditor (glist)))) {
        k |= glist_objectDeselect (glist, selection_getObject (s), 1);
    }

    if (editor_hasSelectedLine (glist_getEditor (glist))) { 
        glist_lineDeselect (glist);
    }
    
    return k;   /* Return 1 if an object has been recreated. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_objectSelect (t_glist *glist, t_gobj *y)
{
    PD_ASSERT (!glist_objectIsSelected (glist, y));                     /* Must NOT be already selected. */
    
    if (editor_hasSelectedLine (glist_getEditor (glist))) { glist_lineDeselect (glist); }
    
    editor_selectionAdd (glist_getEditor (glist), y);
    
    gobj_selected (y, glist, 1);
}

/* Note that deselecting an object with its text active might recreate it. */
/* Return 1 if an object has been recreated. */
/* Lines are cached to reconnect automatically the new object. */

int glist_objectDeselect (t_glist *glist, t_gobj *y, int withUndo)
{
    int dspSuspended = 0;
    
    t_box *z = NULL;
    
    PD_ASSERT (glist_objectIsSelected (glist, y));                      /* Must be already selected. */
    
    if (editor_hasSelectedBox (glist_getEditor (glist))) {
    //
    t_box *box = box_fetch (glist, cast_object (y));
        
    if (editor_getSelectedBox (glist_getEditor (glist)) == box) {
    
        if (editor_hasSelectedBoxDirty (glist_getEditor (glist))) {
            z = box;
            glist_sortSelected (glist);
            editor_selectionCacheLines (glist_getEditor (glist));
            glist_deselectAllRecursive (y);
        }
        gobj_activated (y, glist, 0);
    }
    
    if (class_hasDSP (pd_class (y))) { dspSuspended = dsp_suspend(); }
    //
    }
    
    if (editor_selectionRemove (glist_getEditor (glist), y)) { gobj_selected (y, glist, 0); }
    
    if (z) {
        t_object *t = object_setFromEntry (cast_object (y), glist, z);
        if (t) { glist_lineCheck (glist, t); glist_updateLinesForObject (glist, t); }
        editor_boxSelect (glist_getEditor (glist), NULL);
        glist_redrawRequired (glist);
        glist_setDirty (glist, 1);
    }
    
    if (withUndo && glist_undoIsOk (glist)) { glist_undoAppendSeparator (glist); }
    
    if (dspSuspended) { dsp_resume (dspSuspended); }
    
    if (z) { return 1; } else { return 0; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_objectSelectIfNotSelected (t_glist *glist, t_gobj *y)
{
    if (!glist_objectIsSelected (glist, y)) { glist_deselectAll (glist); glist_objectSelect (glist, y); }
}

int glist_objectDeselectIfSelected (t_glist *glist, t_gobj *y)
{
    if (glist_objectIsSelected (glist, y)) { return glist_objectDeselect (glist, y, 1); }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_objectIsSelected (t_glist *glist, t_gobj *y)
{
    t_selection *s = NULL;
    
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
        if (selection_getObject (s) == y) { 
            return 1; 
        }
    }
    
    return 0;
}

void glist_objectSwapSelected (t_glist *glist, t_gobj *y)
{
    if (glist_objectIsSelected (glist, y)) { glist_objectDeselect (glist, y, 1); }
    else { 
        glist_objectSelect (glist, y);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int glist_objectGetNumberOfSelected (t_glist *glist)
{
    return glist_objectGetIndexAmongSelected (glist, NULL);
}

static int glist_objectGetIndexAmong (t_glist *glist, t_gobj *y, int selected)
{
    t_gobj *t = NULL;
    int n = 0;

    for (t = glist->gl_graphics; t && t != y; t = t->g_next) {
        if (selected == glist_objectIsSelected (glist, t)) { 
            n++; 
        }
    }
    
    return n;
}

int glist_objectGetIndexAmongSelected (t_glist *glist, t_gobj *y)
{
    return glist_objectGetIndexAmong (glist, y, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_objectRemoveSelected (t_glist *glist)
{
    t_gobj *t1 = NULL;
    t_gobj *t2 = NULL;
    
    int dspState     = 0;
    int dspSuspended = 0;
    int onlyScalars  = 1;
    
    for (t1 = glist->gl_graphics; t1; t1 = t2) {
    //
    t2 = t1->g_next;
    
    if (glist_objectIsSelected (glist, t1)) {
        if (!dspSuspended) { 
            if (class_hasDSP (pd_class (t1))) { dspState = dsp_suspend(); dspSuspended = 1; }
        }
        if (!gobj_isScalar (t1)) {
            onlyScalars = 0;
        }
        glist_objectRemove (glist, t1); 
    }
    //
    }

    if (!onlyScalars) { glist_setDirty (glist, 1); }
    
    if (dspSuspended) { dsp_resume (dspState); }
    
    if (glist_undoIsOk (glist)) { glist_undoAppendSeparator (glist); }
}

void glist_objectDisplaceProceed (t_glist *glist, t_gobj *object, int deltaX, int deltaY)
{
    gobj_displaced (object, glist, deltaX, deltaY);
    
    if (pd_class (object) == vinlet_class)  { glist_inletSort (glist);   }
    if (pd_class (object) == voutlet_class) { glist_outletSort (glist);  }
    if (!gobj_isScalar (object))            { glist_setDirty (glist, 1); }
    
    if (glist_undoIsOk (glist)) { glist_undoAppend (glist, undomotion_new (object, deltaX, deltaY)); }
}

void glist_objectSnapSelected (t_glist *glist, int withSnapUndo)
{
    t_selection *y = NULL;
    
    int hasSnapUndo = !withSnapUndo;
    int undoable = glist_undoIsOk (glist);
    
    for (y = editor_getSelection (glist_getEditor (glist)); y; y = selection_getNext (y)) {
    //
    t_gobj *t = selection_getObject (y);
    
    int deltaX = 0;
    int deltaY = 0;
    
    if (gobj_isScalar (t)) {
    
        t_scalar *x = cast_scalar (t);
        
        if (scalar_fieldIsFloat (x, sym_x)) {
            deltaX = snap_getOffset ((int)(scalar_getFloat (x, sym_x) / glist_getValueForOnePixelX (glist)));
        }
 
        if (scalar_fieldIsFloat (x, sym_y)) {
            deltaY = snap_getOffset ((int)(scalar_getFloat (x, sym_y) / glist_getValueForOnePixelY (glist)));
        }
    
    } else {
        deltaX = snap_getOffset (object_getX (cast_object (t)));
        deltaY = snap_getOffset (object_getY (cast_object (t)));
    }
    
    if (deltaX || deltaY) {
        if (undoable && !hasSnapUndo) { glist_undoAppend (glist, undosnap_new()); }
        glist_objectDisplaceProceed (glist, t, deltaX, deltaY);
        hasSnapUndo = 1;
    }
    //
    }
}

void glist_objectDisplaceSelected (t_glist *glist, int deltaX, int deltaY)
{
    if (deltaX || deltaY) {
    //
    t_selection *y = NULL;
    
    for (y = editor_getSelection (glist_getEditor (glist)); y; y = selection_getNext (y)) {
    //
    glist_objectDisplaceProceed (glist, selection_getObject (y), deltaX, deltaY);
    //
    }
    //
    }
}

t_error glist_objectDisplaceByUnique (t_id u, int deltaX, int deltaY)
{
    if (deltaX || deltaY) {
    //
    t_gobj *object = instance_registerGetObject (u);
    t_glist *glist = instance_registerGetOwner (u);

    if (object && glist) {
    //
    glist_objectDisplaceProceed (glist, object, deltaX, deltaY); return PD_ERROR_NONE;
    //
    }
    //
    }
    
    return PD_ERROR;
}

void glist_objectMoveSelected (t_glist *glist, int backward)
{
    t_selection *y = NULL;
    
    int isDirty     = 0;
    int onlyScalars = 1;
    int undoable    = glist_undoIsOk (glist);
    
    for (y = editor_getSelection (glist_getEditor (glist)); y; y = selection_getNext (y)) {
    //
    t_gobj *t = selection_getObject (y);

    if (undoable) {
        glist_undoAppend (glist, backward
            ? undoback_new (t, undosnippet_new (t, glist))
            : undofront_new (t, undosnippet_new (t, glist)));
    }

    if (backward) { glist_objectMoveAtFirst (glist, t); }
    else {
        glist_objectMoveAtLast (glist, t);
    }
    
    if (!gobj_isScalar (t)) { onlyScalars = 0; }
    
    isDirty = 1;
    //
    }
    
    if (isDirty) { glist_redrawRequired (glist); }
    
    if (isDirty && !onlyScalars) { glist_setDirty (glist, 1); }

    if (isDirty && glist_undoIsOk (glist)) { glist_undoAppendSeparator (glist); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
