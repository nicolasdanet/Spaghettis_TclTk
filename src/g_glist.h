
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_glist_h_
#define __g_glist_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _glist {  
    t_object        gl_obj;                                 /* MUST be the first. */
    t_gobj          *gl_graphics;
    t_gmaster       *gl_holder;
    t_glist         *gl_parent;
    t_glist         *gl_next;
    t_environment   *gl_environment;
    t_symbol        *gl_name;
    t_editor        *gl_editor;
    t_unique        gl_uniqueIdentifier;
    t_bounds        gl_bounds;
    t_rectangle     gl_geometryGraph;
    t_rectangle     gl_geometryWindow;
    t_fontsize      gl_fontSize;
    int             gl_isMapped;
    int             gl_isDirty;
    int             gl_isLoading;
    int             gl_isDeleting;
    int             gl_isEditMode;
    int             gl_isSelected;
    int             gl_isGraphOnParent;
    int             gl_hasWindow;
    int             gl_openedAtLoad;
    int             gl_hideText;                            /* Unused but kept for compatibility. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist             *glist_getRoot                          (t_glist *glist);
t_environment       *glist_getEnvironment                   (t_glist *glist);
t_glist             *glist_getView                          (t_glist *glist);

int                 glist_isMapped                          (t_glist *glist);
int                 glist_isRoot                            (t_glist *glist);
int                 glist_isAbstraction                     (t_glist *glist);
int                 glist_isSubpatch                        (t_glist *glist);
int                 glist_isDirty                           (t_glist *glist);
int                 glist_isGraph                           (t_glist *glist);

int                 glist_canHaveWindow                     (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void                glist_setName                           (t_glist *glist, t_symbol *name);
void                glist_setMapped                         (t_glist *glist, int n);
void                glist_setDirty                          (t_glist *glist, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_glist *glist_getParent (t_glist *glist)
{
    return glist->gl_parent;
}

static inline t_glist *glist_getNext (t_glist *glist)
{
    return glist->gl_next;
}

static inline t_symbol *glist_getName (t_glist *glist)
{
    return glist->gl_name;
}

static inline t_editor *glist_getEditor (t_glist *glist)
{
    return glist->gl_editor;
}

static inline t_bounds *glist_getBounds (t_glist *glist)
{
    return &glist->gl_bounds;
}

static inline t_rectangle *glist_getGraphGeometry (t_glist *glist)
{
    return &glist->gl_geometryGraph;
}

static inline t_rectangle *glist_getWindowGeometry (t_glist *glist)
{
    return &glist->gl_geometryWindow;
}

static inline t_gmaster *glist_getMaster (t_glist *glist)
{
    return glist->gl_holder;
}

static inline t_unique glist_getIdentifier (t_glist *glist)
{
    return glist->gl_uniqueIdentifier;
}

static inline t_fontsize glist_getFontSize (t_glist *glist)
{
    return glist->gl_fontSize;
}

static inline int glist_getMapped (t_glist *glist)
{
    return glist->gl_isMapped;
}

static inline int glist_getDirty (t_glist *glist)
{
    return glist->gl_isDirty;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void glist_setNext (t_glist *glist, t_glist *next)
{
    glist->gl_next = next;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int glist_hasParent (t_glist *glist)
{
    return (glist->gl_parent != NULL);
}

static inline int glist_hasParentMapped (t_glist *glist)
{
    return (glist_hasParent (glist) && glist_isMapped (glist_getParent (glist)));
}

static inline int glist_hasEditor (t_glist *glist)
{
    return (glist->gl_editor != NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_glist_h_
