
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

static void select_deselectAllRecursive (t_gobj *g)
{
    if (pd_class (g) == canvas_class) { 
    //
    t_gobj *o = NULL;
    for (o = cast_glist (g)->gl_graphics; o; o = o->g_next) { select_deselectAllRecursive (o); }
    glist_noselect (cast_glist (g));
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

void select_selectLine (t_glist *glist,
    t_outconnect *connection,
    int indexOfObjectOut,
    int indexOfOutlet,
    int indexOfObjectIn,
    int indexOfInlet)
{
    if (glist->gl_editor) {
    //
    glist_noselect (glist);
        
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int select_isObjectSelected (t_glist *glist, t_gobj *y)
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

void select_selectObject (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor) {
    //
    t_selection *selection = (t_selection *)PD_MEMORY_GET (sizeof (t_selection));
    
    if (glist->gl_editor->e_isSelectedline) { select_deselectLine (glist); }

    PD_ASSERT (!select_isObjectSelected (glist, y));    /* Must NOT be already selected. */
    
    selection->sel_next = glist->gl_editor->e_selectedObjects;
    selection->sel_what = y;
    
    glist->gl_editor->e_selectedObjects = selection;
    
    gobj_select (y, glist, 1);
    //
    }
}

void select_deselectObject (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor) {
    //
    int dspSuspended = 0;
    
    t_boxtext *z = NULL;
    
    t_selection *selection1 = NULL;
    t_selection *selection2 = NULL;

    PD_ASSERT (select_isObjectSelected (glist, y));     /* Must be already selected. */
    
    if (glist->gl_editor->e_selectedText) {
    
        t_boxtext *text = glist_findrtext (glist, cast_object (y));
        
        if (glist->gl_editor->e_selectedText == text) {
        
            if (glist->gl_editor->e_isTextDirty) {
                z = text;
                canvas_stowconnections (glist_getcanvas (glist));
                select_deselectAllRecursive (y);
            }
            
            gobj_activate (y, glist, 0);
        }
        
        if (class_hasMethod (pd_class (y), gensym ("dsp"))) { dspSuspended = dsp_suspend(); }
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_noselect (t_glist *x)
{
    if (x->gl_editor)
    {
        while (x->gl_editor->e_selectedObjects)
            select_deselectObject(x, x->gl_editor->e_selectedObjects->sel_what);
        if (x->gl_editor->e_isSelectedline)
            select_deselectLine(x);
    }
}

    /* get the index of a gobj in a glist.  If y is zero, return the
    total number of objects. */
int glist_getindex(t_glist *x, t_gobj *y)
{
    t_gobj *y2;
    int indx;

    for (y2 = x->gl_graphics, indx = 0; y2 && y2 != y; y2 = y2->g_next)
        indx++;
    return (indx);
}

    /* get the index of the object, among selected items, if "selected"
    is set; otherwise, among unselected ones.  If y is zero, just
    counts the selected or unselected objects. */
int glist_selectionindex(t_glist *x, t_gobj *y, int selected)
{
    t_gobj *y2;
    int indx;

    for (y2 = x->gl_graphics, indx = 0; y2 && y2 != y; y2 = y2->g_next)
        if (selected == select_isObjectSelected(x, y2))
            indx++;
    return (indx);
}

t_gobj *glist_nth (t_glist *x, int n)
{
    t_gobj *y;
    int indx;
    for (y = x->gl_graphics, indx = 0; y; y = y->g_next, indx++)
        if (indx == n)
            return (y);
    return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
