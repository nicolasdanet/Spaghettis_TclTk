
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
#pragma mark -

t_glist *canvas_getRoot (t_glist *glist)
{
    if (canvas_isRoot (glist)) { return glist; }
    else {
        return canvas_getRoot (glist->gl_parent);
    }
}

t_environment *canvas_getEnvironment (t_glist *glist)
{
    PD_ASSERT (glist);
    
    while (!glist->gl_environment) { glist = glist->gl_parent; PD_ASSERT (glist); }
    
    PD_ASSERT (canvas_isRoot (glist));
    
    return glist->gl_environment;
}

t_glist *canvas_getView (t_glist *glist)
{
    while (glist->gl_parent && !canvas_canHaveWindow (glist)) { glist = glist->gl_parent; }
    
    return glist;
}

t_symbol *canvas_getName (t_glist *glist)
{
    return glist->gl_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_isMapped (t_glist *glist)
{
    return (!glist->gl_isLoading && canvas_getView (glist)->gl_isMapped);
}

int canvas_isRoot (t_glist *glist)
{
    int k = (!glist->gl_parent || canvas_isAbstraction (glist));
    
    if (k) { PD_ASSERT (glist->gl_environment != NULL); }
    
    return k;
}

int canvas_isAbstraction (t_glist *glist)
{
    return (glist->gl_parent && glist->gl_environment != NULL);
}

int canvas_isSubpatch (t_glist *glist)
{
    return !canvas_isRoot (glist);
}

int canvas_isDirty (t_glist *glist)
{
    return (canvas_getRoot (glist)->gl_isDirty != 0);
}

/* A graph-on-parent that contains an array of numbers. */

int canvas_isGraph (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

/* Either a top window a subpacth or a graph-on-parent forced. */

int canvas_canHaveWindow (t_glist *glist)
{
    return (glist->gl_hasWindow || !glist->gl_isGraphOnParent);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
