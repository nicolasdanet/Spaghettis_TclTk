
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* A root has no parent and an environment. */
/* An abstraction has a parent and an environment. */
/* A subpatch has a parent also but no environment. */
/* A top patch is either a root or an abstraction. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isRoot (t_glist *glist)
{
    return (!glist_hasParent (glist));
}

int glist_isTop (t_glist *glist)
{
    int k = (glist_isRoot (glist) || glist_isAbstraction (glist));
    
    return k;
}

int glist_isAbstraction (t_glist *glist)
{
    return (glist_hasParent (glist) && (glist->gl_environment != NULL));
}

int glist_isSubpatch (t_glist *glist)
{
    return (!glist_isTop (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Array is a GOP patch that contains only a scalar. */
/* This scalar has an array of numbers as unique field. */
/* Dirty bit is always owned by the top patch. */
/* For GOP the window to draw is owned higher in the tree. */
/* Note that a GOP can be opened in its own window on demand. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isArray (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

int glist_isDirty (t_glist *glist)
{
    return (glist_getTop (glist)->gl_isDirty != 0);
}

int glist_isOnScreen (t_glist *glist)
{
    return (!glist_isLoading (glist) && glist_getView (glist)->gl_isMapped);
}

int glist_isWindowable (t_glist *glist)
{
    return (!glist_isGraphOnParent (glist) || glist_hasWindow (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *glist_getTop (t_glist *glist)
{
    if (glist_isTop (glist)) { return glist; } else { return glist_getTop (glist_getParent (glist)); }
}

t_environment *glist_getEnvironment (t_glist *glist)
{
    return (glist_getTop (glist)->gl_environment);
}

t_glist *glist_getView (t_glist *glist)
{
    while (glist_hasParent (glist) && !glist_isWindowable (glist)) { glist = glist_getParent (glist); }
    
    return glist;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_addObjectProceed (t_glist *glist, t_gobj *first, t_gobj *next)
{
    next->g_next = NULL;
    
    if (first != NULL) { next->g_next = first->g_next; first->g_next = next; }
    else {
    //    
    if (!glist->gl_graphics) { glist->gl_graphics = next; }
    else {
        t_gobj *t = NULL; for (t = glist->gl_graphics; t->g_next; t = t->g_next) { } 
        t->g_next = next;
    }
    //
    }
}

void glist_addObjectNext (t_glist *glist, t_gobj *first, t_gobj *next)
{
    int needToPaintScalars = class_hasPainterWidgetBehavior (pd_class (next));
    
    if (needToPaintScalars) { paint_erase(); }
    
    glist_addObjectProceed (glist, first, next);
    
    if (cast_objectIfConnectable (next)) { editor_boxAdd (glist_getEditor (glist), cast_object (next)); }
    if (glist_isOnScreen (glist_getView (glist))) { gobj_visibilityChanged (next, glist, 1); }
    
    if (needToPaintScalars) { paint_draw(); }
}

void glist_addObject (t_glist *glist, t_gobj *y)
{
    glist_addObjectNext (glist, NULL, y);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_setName (t_glist *glist, t_symbol *name)
{
    canvas_unbind (glist);
    
    glist->gl_name = name;
    
    canvas_bind (glist);
    
    if (glist_hasWindow (glist)) { canvas_updateTitle (glist); }
}

void glist_setDirty (t_glist *glist, int n)
{
    int isDirty = (n != 0);
        
    t_glist *y = glist_getTop (glist);
        
    if (y->gl_isDirty != isDirty) {
    //
    y->gl_isDirty = isDirty; 
    
    if (glist_hasWindow (y)) { canvas_updateTitle (y); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
