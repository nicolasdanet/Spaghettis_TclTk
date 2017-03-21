
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

extern t_class  *canvas_class;
extern t_class  *vinlet_class;
extern t_class  *voutlet_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int canvas_getIndexOfObjectAmong (t_glist *glist, t_gobj *y, int selected)
{
    t_gobj *t = NULL;
    int n = 0;

    for (t = glist->gl_graphics; t && t != y; t = t->g_next) {
        if (selected == canvas_isObjectSelected (glist, t)) { 
            n++; 
        }
    }
    
    return n;
}

static void canvas_deselectAllRecursive (t_gobj *y)
{
    if (pd_class (y) == canvas_class) { 
    //
    t_gobj *o = NULL;
    for (o = cast_glist (y)->gl_graphics; o; o = o->g_next) { canvas_deselectAllRecursive (o); }
    canvas_deselectAll (cast_glist (y));
    //
    }
}

static void canvas_deselectLine (t_glist *glist)
{
    glist->gl_editor->e_hasSelectedline = 0;
    
    sys_vGui (".x%lx.c itemconfigure %lxLINE -fill black\n",
                    canvas_getView (glist),
                    glist->gl_editor->e_selectedLineConnection);   
}

static void canvas_cacheLines (t_glist *glist)
{
    t_gobj *selectedHead = NULL;
    t_gobj *selectedTail = NULL;
    t_gobj *unselectedHead = NULL;
    t_gobj *unselectedTail = NULL;
    t_gobj *yA = NULL;
    t_gobj *yB = NULL;
    t_traverser t;
    t_outconnect *connection;
    
    /* Split selected object from uneselected ones and move it to the end. */
    
    for (yA = glist->gl_graphics; yA; yA = yB) {
    //
    yB = yA->g_next;
    
    if (canvas_isObjectSelected (glist, yA)) {
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

    buffer_reset (glist->gl_editor->e_cachedLines);
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    int s1 = canvas_isObjectSelected (glist, cast_gobj (traverser_getSource (&t)));
    int s2 = canvas_isObjectSelected (glist, cast_gobj (traverser_getDestination (&t)));
    
    if (s1 != s2) {
    //
    buffer_vAppend (glist->gl_editor->e_cachedLines, "ssiiii;",
        sym___hash__X, 
        sym_connect,
        canvas_getIndexOfObject (glist, cast_gobj (traverser_getSource (&t))),
        traverser_getIndexOfOutlet (&t),
        canvas_getIndexOfObject (glist, cast_gobj (traverser_getDestination (&t))),
        traverser_getIndexOfInlet (&t));
    //
    }
    //
    }
}

void canvas_restoreCachedLines (t_glist *glist)
{
    instance_stackEval (glist, glist->gl_editor->e_cachedLines);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_removeSelectedObjects (t_glist *glist)
{
    t_gobj *yA = NULL;
    t_gobj *yB = NULL;
        
    int state = dsp_suspend();
    
    /* If box text is selected, deselecting it might recreate the object. */ 
    /* Workaround by deselecting it first and looking for a "new" object next. */
        
    if (editor_hasSelectedBox (glist->gl_editor)) {
        instance_setNewestObject (NULL);
        canvas_deselectAll (glist);
        if (instance_getNewestObject()) {
            for (yA = glist->gl_graphics; yA; yA = yA->g_next) {
                if (cast_pd (yA) == instance_getNewestObject()) { canvas_selectObject (glist, yA); }
            }
        }
    }
    
    for (yA = glist->gl_graphics; yA; yA = yB) {
        yB = yA->g_next;
        if (canvas_isObjectSelected (glist, yA)) { canvas_removeObject (glist, yA); }
    }

    dsp_resume (state);
    
    canvas_dirty (glist, 1);
}

void canvas_removeSelectedLine (t_glist *glist)
{
    if (glist->gl_editor->e_hasSelectedline) {
        canvas_disconnect (glist, NULL, 4, glist->gl_editor->e_selectedLine);
        canvas_dirty (glist, 1);
    }
}

void canvas_displaceSelectedObjects (t_glist *glist, int deltaX, int deltaY)
{
    t_selection *y = NULL;
    
    int needToResortInlets  = 0;
    int needToResortOutlets = 0;
    int isDirty = 0;
    
    for (y = glist->gl_editor->e_selectedObjects; y; y = selection_getNext (y)) {
        gobj_displaced (selection_getObject (y), glist, deltaX, deltaY);
        if (pd_class (selection_getObject (y)) == vinlet_class)  { needToResortInlets  = 1; }
        if (pd_class (selection_getObject (y)) == voutlet_class) { needToResortOutlets = 1; }
        isDirty = 1;
    }
    
    if (needToResortInlets)  { canvas_resortInlets (glist); }
    if (needToResortOutlets) { canvas_resortOutlets (glist); }
        
    if (isDirty) { canvas_dirty (glist, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_isObjectSelected (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor) {
    //
    t_selection *s = NULL;
    for (s = glist->gl_editor->e_selectedObjects; s; s = selection_getNext (s)) {
        if (selection_getObject (s) == y) { return 1; }
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_selectingByLasso (t_glist *glist, int a, int b, int end)
{
    if (end) {
    
        t_rectangle r;
        
        rectangle_set (&r, glist->gl_editor->e_previousX, glist->gl_editor->e_previousY, a, b);
        
        canvas_selectObjectsInRectangle (glist, &r);
        
        glist->gl_editor->e_action = ACTION_NONE;
        
        sys_vGui (".x%lx.c delete TEMPORARY\n", canvas_getView (glist));
        
    } else {
        sys_vGui (".x%lx.c coords TEMPORARY %d %d %d %d\n",
                        canvas_getView (glist),
                        glist->gl_editor->e_previousX,
                        glist->gl_editor->e_previousY,
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
    
    if (rectangle_overlap (r, &t) && !canvas_isObjectSelected (glist, y)) {
        canvas_selectObject (glist, y);
    }
    //
    }
}

void canvas_selectObject (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor->e_hasSelectedline) { canvas_deselectLine (glist); }

    PD_ASSERT (!canvas_isObjectSelected (glist, y));    /* Must NOT be already selected. */
    
    editor_selectionAdd (glist->gl_editor, y);
    
    gobj_selected (y, glist, 1);
}

void canvas_selectObjectIfNotSelected (t_glist *glist, t_gobj *y)
{
    if (!canvas_isObjectSelected (glist, y)) {
        canvas_deselectAll (glist);
        canvas_selectObject (glist, y);
    }
}

void canvas_selectLine (t_glist *glist,
    t_outconnect *connection,
    int indexOfObjectOut,
    int indexOfOutlet,
    int indexOfObjectIn,
    int indexOfInlet)
{
    canvas_deselectAll (glist);
        
    glist->gl_editor->e_hasSelectedline = 1;
    glist->gl_editor->e_selectedLineConnection = connection;
    
    SET_FLOAT (glist->gl_editor->e_selectedLine + 0, indexOfObjectOut);
    SET_FLOAT (glist->gl_editor->e_selectedLine + 1, indexOfOutlet);
    SET_FLOAT (glist->gl_editor->e_selectedLine + 2, indexOfObjectIn);
    SET_FLOAT (glist->gl_editor->e_selectedLine + 3, indexOfInlet);
    
    sys_vGui (".x%lx.c itemconfigure %lxLINE -fill blue\n",
                    canvas_getView (glist),
                    glist->gl_editor->e_selectedLineConnection);  
}

/* Note that deselecting an object with its text active might recreate it. */
/* Return 1 if an object has been recreated. */

int canvas_deselectObject (t_glist *glist, t_gobj *y)
{
    int dspSuspended = 0;
    
    t_box *z = NULL;
    
    PD_ASSERT (canvas_isObjectSelected (glist, y));         /* Must be already selected. */
    
    if (editor_hasSelectedBox (glist->gl_editor)) {
    
        t_box *text = box_fetch (glist, cast_object (y));
        
        if (editor_getSelectedBox (glist->gl_editor) == text) {
            if (glist->gl_editor->e_isTextDirty) {
                z = text;
                canvas_cacheLines (canvas_getView (glist));
                canvas_deselectAllRecursive (y);
            }
            gobj_activated (y, glist, 0);
        }
        
        if (class_hasDSP (pd_class (y))) { dspSuspended = dsp_suspend(); }
    }
    
    if (editor_selectionRemove (glist->gl_editor, y)) { gobj_selected (y, glist, 0); }
    
    if (z) {
        object_setFromEntry (cast_object (y), glist, z);
        canvas_updateLinesByObject (glist, cast_object (y));
        editor_selectBox (glist->gl_editor, NULL);
    }
    
    if (dspSuspended) { dsp_resume (dspSuspended); }
    
    if (z) { return 1; } else { return 0; }
}

int canvas_deselectObjectIfSelected (t_glist *glist, t_gobj *y)
{
    if (canvas_isObjectSelected (glist, y)) { return canvas_deselectObject (glist, y); }
    
    return 0;
}

int canvas_deselectAll (t_glist *glist)
{
    int k = 0;
    
    if (glist->gl_editor) {         /* Required (for newly recreated subpatch for instance). */
    //
    while (glist->gl_editor->e_selectedObjects) {
        k |= canvas_deselectObject (glist, selection_getObject (glist->gl_editor->e_selectedObjects));
    }

    if (glist->gl_editor->e_hasSelectedline) { canvas_deselectLine (glist); }
    //
    }
    
    return k;   /* Return 1 if an object has been recreated. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_getNumberOfObjects (t_glist *glist)
{
    t_gobj *y = NULL; 
    int n = 0;
    for (y = glist->gl_graphics; y; y = y->g_next) { n++; }
    return n;
}

int canvas_getNumberOfSelectedObjects (t_glist *glist)
{
    return canvas_getIndexOfObjectAmongSelected (glist, NULL);
}

int canvas_getNumberOfUnselectedObjects (t_glist *glist)
{
    return canvas_getIndexOfObjectAmongUnselected (glist, NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_getIndexOfObjectAmongSelected (t_glist *glist, t_gobj *y)
{
    return canvas_getIndexOfObjectAmong (glist, y, 1);
}

int canvas_getIndexOfObjectAmongUnselected (t_glist *glist, t_gobj *y)
{
    return canvas_getIndexOfObjectAmong (glist, y, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setMotionFunction (t_glist *glist, t_gobj *y, t_motionfn callback, int a, int b)
{
    t_glist *canvas = canvas_getView (glist);
    
    if (callback) { canvas->gl_editor->e_action = ACTION_PASS; }
    else { 
        canvas->gl_editor->e_action = ACTION_NONE;
    }
    
    canvas->gl_editor->e_grabbed    = y;
    canvas->gl_editor->e_fnMotion   = callback;
    canvas->gl_editor->e_previousX  = a;
    canvas->gl_editor->e_previousY  = b;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
