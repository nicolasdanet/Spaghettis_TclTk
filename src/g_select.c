
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Get selected objects from uneselected ones and move them to the end. */

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
#pragma mark -

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
#pragma mark -

static void glist_deselectAllRecursive (t_gobj *y)
{
    if (pd_class (y) == canvas_class) { 
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
        k |= glist_objectDeselect (glist, selection_getObject (s));
    }

    if (editor_hasSelectedLine (glist_getEditor (glist))) { 
        glist_lineDeselect (glist);
    }
    
    return k;   /* Return 1 if an object has been recreated. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

int glist_objectDeselect (t_glist *glist, t_gobj *y)
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
            editor_selectionCacheLines (glist_getEditor (glist_getView (glist)));
            glist_deselectAllRecursive (y);
        }
        gobj_activated (y, glist, 0);
    }
    
    if (class_hasDSP (pd_class (y))) { dspSuspended = dsp_suspend(); }
    //
    }
    
    if (editor_selectionRemove (glist_getEditor (glist), y)) { gobj_selected (y, glist, 0); }
    
    if (z) {
        object_setFromEntry (cast_object (y), glist, z);
        glist_updateLines (glist, cast_object (y));
        editor_boxSelect (glist_getEditor (glist), NULL);
    }
    
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
    if (glist_objectIsSelected (glist, y)) { return glist_objectDeselect (glist, y); }
    
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

void glist_objectRemoveSelected (t_glist *glist)
{
    /* If box text is selected, deselecting it might recreate the object. */ 
    /* Just forbid that. */
    
    if (editor_hasSelectedBox (glist_getEditor (glist))) { PD_BUG; glist_deselectAll (glist); }
    else {
    //
    t_gobj *t1 = NULL;
    t_gobj *t2 = NULL;
    
    int dspState = 0;
    int dspSuspended = 0;
    
    for (t1 = glist->gl_graphics; t1; t1 = t2) {
    //
    t2 = t1->g_next;
    
    if (glist_objectIsSelected (glist, t1)) {
        if (!dspSuspended) { 
            if (class_hasDSP (pd_class (t1))) { dspState = dsp_suspend(); dspSuspended = 1; }
        } 
        glist_objectRemove (glist, t1); 
    }
    //
    }

    glist_setDirty (glist, 1);
    
    if (dspSuspended) { dsp_resume (dspState); }
    //
    }
}

void glist_objectDisplaceSelected (t_glist *glist, int deltaX, int deltaY)
{
    t_selection *y = NULL;
    
    int resortInlets  = 0;
    int resortOutlets = 0;
    int isDirty = 0;
    
    for (y = editor_getSelection (glist_getEditor (glist)); y; y = selection_getNext (y)) {
        gobj_displaced (selection_getObject (y), glist, deltaX, deltaY);
        resortInlets  |= (pd_class (selection_getObject (y)) == vinlet_class);
        resortOutlets |= (pd_class (selection_getObject (y)) == voutlet_class);
        isDirty = 1;
    }
    
    if (resortInlets)  { canvas_resortInlets (glist);  }
    if (resortOutlets) { canvas_resortOutlets (glist); }
    if (isDirty)       { glist_setDirty (glist, 1);    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
