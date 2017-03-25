
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

/* A patch without a parent is root. */
/* Only abstractions and roots have an environment. */
/* A subpatch have a parent and no environment. */
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

int glist_isMapped (t_glist *glist)
{
    return (!glist_isLoading (glist) && glist_getView (glist)->gl_isMapped);
}

int glist_isWindowable (t_glist *glist)
{
    return (glist_hasWindow (glist) || !glist_isGraphOnParent (glist));
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

void glist_setName (t_glist *glist, t_symbol *name)
{
    canvas_unbind (glist);
    glist->gl_name = name;
    canvas_bind (glist);
    
    if (glist_hasWindow (glist)) { canvas_updateTitle (glist); }
}

void glist_setMapped (t_glist *glist, int n)
{
    glist->gl_isMapped = (n != 0);
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

void glist_setEditMode (t_glist *glist, int n)
{
    glist->gl_isEditing = (n != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
