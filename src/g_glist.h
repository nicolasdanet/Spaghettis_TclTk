
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
    char            gl_isMapped;
    char            gl_isDirty;
    char            gl_isLoading;
    char            gl_isDeleting;
    char            gl_isEditMode;
    char            gl_isSelected;
    char            gl_isGraphOnParent;
    char            gl_hasWindow;
    char            gl_openedAtLoad;
    char            gl_hideText;                            /* Unused but kept for compatibility. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist             *glist_getRoot                          (t_glist *glist);
t_environment       *glist_getEnvironment                   (t_glist *glist);
t_glist             *glist_getView                          (t_glist *glist);

void                glist_setName                           (t_glist *glist, t_symbol *name);
int                 glist_canHaveWindow                     (t_glist *glist);
int                 glist_isMapped                          (t_glist *glist);
int                 glist_isRoot                            (t_glist *glist);
int                 glist_isAbstraction                     (t_glist *glist);
int                 glist_isSubpatch                        (t_glist *glist);
int                 glist_isDirty                           (t_glist *glist);
int                 glist_isGraph                           (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_glist *glist_getParent (t_glist *glist)
{
    return glist->gl_parent;
}

static inline t_symbol *glist_getName (t_glist *glist)
{
    return glist->gl_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_glist_h_
