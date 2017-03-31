
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

static void glist_deselectAllRecursive (t_gobj *y)
{
    if (pd_class (y) == canvas_class) { 
    //
    t_gobj *o = NULL;
    for (o = cast_glist (y)->gl_graphics; o; o = o->g_next) { glist_deselectAllRecursive (o); }
    canvas_deselectAll (cast_glist (y));
    //
    }
}

static void glist_deselectLine (t_glist *glist)
{
    glist_updateLineSelected (glist, 0);
    editor_selectedLineReset (glist_getEditor (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_selectingByLasso (t_glist *glist, int a, int b, int end)
{
    if (end) {
    
        t_rectangle r;
        
        rectangle_set (&r, 
            drag_getStartX (editor_getDrag (glist_getEditor (glist))), 
            drag_getStartY (editor_getDrag (glist_getEditor (glist))), 
            a, 
            b);
        
        canvas_selectObjectsInRectangle (glist, &r);
        
        editor_resetAction (glist_getEditor (glist));
        
        sys_vGui (".x%lx.c delete TEMPORARY\n", glist_getView (glist));
        
    } else {
        sys_vGui (".x%lx.c coords TEMPORARY %d %d %d %d\n",
                        glist_getView (glist),
                        drag_getStartX (editor_getDrag (glist_getEditor (glist))),
                        drag_getStartY (editor_getDrag (glist_getEditor (glist))),
                        a,
                        b);
    }
}

void canvas_selectingByLassoStart (t_glist *glist, int positionX, int positionY)
{
    canvas_selectingByLasso (glist, positionX, positionY, 0);
}

void canvas_selectingByLassoEnd (t_glist *glist, int positionX, int positionY)
{
    canvas_selectingByLasso (glist, positionX, positionY, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_selectObjectsInRectangle (t_glist *glist, t_rectangle *r)
{
    t_gobj *y;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_rectangle t;
    
    gobj_getRectangle (y, glist, &t);
    
    if (rectangle_overlap (r, &t) && !glist_objectIsSelected (glist, y)) {
        canvas_selectObject (glist, y);
    }
    //
    }
}

void canvas_selectObject (t_glist *glist, t_gobj *y)
{
    if (editor_hasSelectedLine (glist_getEditor (glist))) { glist_deselectLine (glist); }

    PD_ASSERT (!glist_objectIsSelected (glist, y));    /* Must NOT be already selected. */
    
    editor_selectionAdd (glist_getEditor (glist), y);
    
    gobj_selected (y, glist, 1);
}

void canvas_selectObjectIfNotSelected (t_glist *glist, t_gobj *y)
{
    if (!glist_objectIsSelected (glist, y)) {
        canvas_deselectAll (glist);
        canvas_selectObject (glist, y);
    }
}

void canvas_selectLine (t_glist *glist, t_outconnect *connection, int m, int i, int n, int j)
{
    canvas_deselectAll (glist);
    
    editor_selectedLineSet (glist_getEditor (glist), connection, m, i, n, j);
    
    sys_vGui (".x%lx.c itemconfigure %lxLINE -fill blue\n",
                    glist_getView (glist),
                    editor_getSelectedLineConnection (glist_getEditor (glist)));  
}

/* Split selected object from uneselected ones and move them to the end. */

static void canvas_putSelectedObjectsAtLast (t_glist *glist)
{
    t_gobj *selectedHead = NULL;
    t_gobj *selectedTail = NULL;
    t_gobj *unselectedHead = NULL;
    t_gobj *unselectedTail = NULL;
    t_gobj *yA = NULL;
    t_gobj *yB = NULL;

    for (yA = glist->gl_graphics; yA; yA = yB) {
    //
    yB = yA->g_next;
    
    if (glist_objectIsSelected (glist, yA)) {
        if (selectedTail) { selectedTail->g_next = yA; selectedTail = yA; yA->g_next = NULL; }
        else {
            selectedHead = selectedTail = yA; selectedTail->g_next = NULL;
        }
    } else {
        if (unselectedTail) { unselectedTail->g_next = yA; unselectedTail = yA; yA->g_next = NULL; }
        else {
            unselectedHead = unselectedTail = yA; unselectedTail->g_next = NULL;
        }
    }
    //
    }

    if (!unselectedHead) { glist->gl_graphics = selectedHead; }
    else {
        glist->gl_graphics = unselectedHead; unselectedTail->g_next = selectedHead; 
    }
}

/* Note that deselecting an object with its text active might recreate it. */
/* Return 1 if an object has been recreated. */

int canvas_deselectObject (t_glist *glist, t_gobj *y)
{
    int dspSuspended = 0;
    
    t_box *z = NULL;
    
    PD_ASSERT (glist_objectIsSelected (glist, y));         /* Must be already selected. */
    
    if (editor_hasSelectedBox (glist_getEditor (glist))) {
    
        t_box *text = box_fetch (glist, cast_object (y));
        
        if (editor_getSelectedBox (glist_getEditor (glist)) == text) {
            if (editor_hasSelectedBoxDirty (glist_getEditor (glist))) {
                z = text;
                canvas_putSelectedObjectsAtLast (glist);
                editor_selectionCacheLines (glist_getEditor (glist_getView (glist)));
                glist_deselectAllRecursive (y);
            }
            gobj_activated (y, glist, 0);
        }
        
        if (class_hasDSP (pd_class (y))) { dspSuspended = dsp_suspend(); }
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

int canvas_deselectObjectIfSelected (t_glist *glist, t_gobj *y)
{
    if (glist_objectIsSelected (glist, y)) { return canvas_deselectObject (glist, y); }
    
    return 0;
}

int canvas_deselectAll (t_glist *glist)
{
    int k = 0;
    
    while (editor_getSelection (glist_getEditor (glist))) {
    //
    k |= canvas_deselectObject (glist, selection_getObject (editor_getSelection (glist_getEditor (glist))));
    //
    }

    if (editor_hasSelectedLine (glist_getEditor (glist))) { glist_deselectLine (glist); }
    
    return k;   /* Return 1 if an object has been recreated. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
