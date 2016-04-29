
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

void select_selectObject (t_glist *glist, t_gobj *y)
{
    if (glist->gl_editor) {
    //
    t_selection *selection = (t_selection *)PD_MEMORY_GET (sizeof (t_selection));
    
    if (glist->gl_editor->e_isSelectedline) { select_deselectLine (glist); }

    PD_ASSERT (!select_isObjectSelected (glist, y));
    
    selection->sel_next = glist->gl_editor->e_selectedObjects;
    selection->sel_what = y;
    
    glist->gl_editor->e_selectedObjects = selection;
    
    gobj_select (y, glist, 1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* recursively deselect everything in a gobj "g", if it happens to be
    a glist, in preparation for deselecting g itself in glist_dselect() */
static void glist_checkanddeselectall(t_glist *gl, t_gobj *g)
{
    t_glist *gl2;
    t_gobj *g2;
    if (pd_class(&g->g_pd) != canvas_class)
        return;
    gl2 = (t_glist *)g;
    for (g2 = gl2->gl_graphics; g2; g2 = g2->g_next)
        glist_checkanddeselectall(gl2, g2);
    glist_noselect(gl2);
}

    /* call this for selected objects only */
void glist_deselect(t_glist *x, t_gobj *y)
{
    int fixdsp = 0;
    if (x->gl_editor)
    {
        t_selection *sel, *sel2;
        t_boxtext *z = 0;
        if (!select_isObjectSelected(x, y)) { PD_BUG; }
        if (x->gl_editor->e_selectedText)
        {
            t_boxtext *fuddy = glist_findrtext(x, (t_object *)y);
            if (x->gl_editor->e_selectedText == fuddy)
            {
                if (x->gl_editor->e_isTextDirty)
                {
                    z = fuddy;
                    canvas_stowconnections(glist_getcanvas(x));
                    glist_checkanddeselectall(x, y);
                }
                gobj_activate(y, x, 0);
            }
            if (class_hasMethod (pd_class (&y->g_pd), gensym ("dsp")))
                fixdsp = dsp_suspend();
        }
        if ((sel = x->gl_editor->e_selectedObjects)->sel_what == y)
        {
            x->gl_editor->e_selectedObjects = x->gl_editor->e_selectedObjects->sel_next;
            gobj_select(sel->sel_what, x, 0);
            PD_MEMORY_FREE(sel);
        }
        else
        {
            for (sel = x->gl_editor->e_selectedObjects; sel2 = sel->sel_next;
                sel = sel2)
            {
                if (sel2->sel_what == y)
                {
                    sel->sel_next = sel2->sel_next;
                    gobj_select(sel2->sel_what, x, 0);
                    PD_MEMORY_FREE(sel2);
                    break;
                }
            }
        }
        if (z)
        {
            char *buf;
            int bufsize;

            rtext_gettext(z, &buf, &bufsize);
            text_setto((t_object *)y, x, buf, bufsize);
            canvas_updateLinesByObject(x, (t_object *)y);
            x->gl_editor->e_selectedText = 0;
        }
        if (fixdsp)
            dsp_resume(1);
    }
}

void glist_noselect(t_glist *x)
{
    if (x->gl_editor)
    {
        while (x->gl_editor->e_selectedObjects)
            glist_deselect(x, x->gl_editor->e_selectedObjects->sel_what);
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
