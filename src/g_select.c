
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int select_getIndexOfObject (t_glist *glist, t_gobj *y, int selected)
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

static void select_deselectAllRecursive (t_gobj *g)
{
    if (pd_class (g) == canvas_class) { 
    //
    t_gobj *o = NULL;
    for (o = cast_glist (g)->gl_graphics; o; o = o->g_next) { select_deselectAllRecursive (o); }
    canvas_deselectAll (cast_glist (g));
    //
    }
}

static void select_deselectLine (t_glist *glist)
{
    PD_ASSERT (glist->gl_editor);
    
    glist->gl_editor->e_isSelectedline = 0;
    
    sys_vGui (".x%lx.c itemconfigure %lxLINE -fill black\n",
                glist,
                glist->gl_editor->e_selectedLineConnection);   
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_removeSelectedLine (t_glist *glist)
{
    if (glist->gl_editor->e_isSelectedline) {
    //
    canvas_disconnect (glist, 
        glist->gl_editor->e_selectedLineIndexOfObjectOut,
        glist->gl_editor->e_selectedLineIndexOfOutlet,
        glist->gl_editor->e_selectedLineIndexOfObjectIn,
        glist->gl_editor->e_selectedLineIndexOfInlet);
             
    canvas_dirty (glist, 1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_isObjectSelected (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor) {
    //
    t_selection *selection = NULL;
    for (selection = glist->gl_editor->e_selectedObjects; selection; selection = selection->sel_next) {
        if (selection->sel_what == y) { return 1; }
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_selectObjectsInRectangle (t_glist *glist, int a, int b, int c, int d)
{
    t_gobj *y;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    int x1, y1, x2, y2;
    
    gobj_getRectangle (y, glist, &x1, &y1, &x2, &y2);
    
    if (c >= x1 && a <= x2 && d >= y1 && b <= y2 && !canvas_isObjectSelected (glist, y)) {
        canvas_selectObject (glist, y);
    }
    //
    }
}

void canvas_selectObject (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor) {
    //
    t_selection *selection = (t_selection *)PD_MEMORY_GET (sizeof (t_selection));
    
    if (glist->gl_editor->e_isSelectedline) { select_deselectLine (glist); }

    PD_ASSERT (!canvas_isObjectSelected (glist, y));    /* Must NOT be already selected. */
    
    selection->sel_next = glist->gl_editor->e_selectedObjects;
    selection->sel_what = y;
    
    glist->gl_editor->e_selectedObjects = selection;
    
    gobj_select (y, glist, 1);
    //
    }
}

void canvas_selectLine (t_glist *glist,
    t_outconnect *connection,
    int indexOfObjectOut,
    int indexOfOutlet,
    int indexOfObjectIn,
    int indexOfInlet)
{
    if (glist->gl_editor) {
    //
    canvas_deselectAll (glist);
        
    glist->gl_editor->e_isSelectedline                  = 1;
    glist->gl_editor->e_selectedLineIndexOfObjectOut    = indexOfObjectOut;
    glist->gl_editor->e_selectedLineIndexOfOutlet       = indexOfOutlet;
    glist->gl_editor->e_selectedLineIndexOfObjectIn     = indexOfObjectIn;
    glist->gl_editor->e_selectedLineIndexOfInlet        = indexOfInlet;
    glist->gl_editor->e_selectedLineConnection          = connection;
    
    sys_vGui (".x%lx.c itemconfigure %lxLINE -fill blue\n",
                glist,
                glist->gl_editor->e_selectedLineConnection);
    //
    }    
}

void canvas_deselectObject (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor) {
    //
    int dspSuspended = 0;
    
    t_boxtext *z = NULL;
    
    t_selection *selection1 = NULL;
    t_selection *selection2 = NULL;

    PD_ASSERT (canvas_isObjectSelected (glist, y));     /* Must be already selected. */
    
    if (glist->gl_editor->e_selectedText) {
    
        t_boxtext *text = glist_findrtext (glist, cast_object (y));
        
        if (glist->gl_editor->e_selectedText == text) {
        
            if (glist->gl_editor->e_isTextDirty) {
                z = text;
                canvas_stowconnections (canvas_getView (glist));
                select_deselectAllRecursive (y);
            }
            
            gobj_activate (y, glist, 0);
        }
        
        if (class_hasMethod (pd_class (y), sym_dsp)) { dspSuspended = dsp_suspend(); }
    }
    
    selection1 = glist->gl_editor->e_selectedObjects;
    
    if (selection1->sel_what == y) {
        glist->gl_editor->e_selectedObjects = glist->gl_editor->e_selectedObjects->sel_next;
        gobj_select (y, glist, 0);
        PD_MEMORY_FREE (selection1);
        
    } else {
        for (; selection2 = selection1->sel_next; selection1 = selection2) {
            if (selection2->sel_what == y) {
                selection1->sel_next = selection2->sel_next;
                gobj_select (y, glist, 0);
                PD_MEMORY_FREE (selection2);
                break;
            }
        }
    }
    
    if (z) {
        char *t = NULL;
        int size;
        rtext_gettext (z, &t, &size);
        text_setto (cast_object (y), glist, t, size);
        canvas_updateLinesByObject (glist, cast_object (y));
        glist->gl_editor->e_selectedText = 0;
    }
    
    if (dspSuspended) { dsp_resume (dspSuspended); }
    //
    }
}

void canvas_deselectAll (t_glist *glist)
{
    if (glist->gl_editor) {
    //
    while (glist->gl_editor->e_selectedObjects) {
        canvas_deselectObject (glist, glist->gl_editor->e_selectedObjects->sel_what);
    }

    if (glist->gl_editor->e_isSelectedline) { select_deselectLine (glist); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_getNumberOfSelectedObjects (t_glist *glist)
{
    return canvas_getIndexOfObjectAmongSelected (glist, NULL);
}

int canvas_getNumberOfUnselectedObjects (t_glist *glist)
{
    return canvas_getIndexOfObjectAmongUnselected (glist, NULL);
}

int canvas_getIndexOfObjectAmongSelected (t_glist *glist, t_gobj *y)
{
    return select_getIndexOfObject (glist, y, 1);
}

int canvas_getIndexOfObjectAmongUnselected (t_glist *glist, t_gobj *y)
{
    return select_getIndexOfObject (glist, y, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
