
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
    int             gl_hasWindow;
    int             gl_isMapped;
    int             gl_isDirty;
    int             gl_isLoading;
    int             gl_isDeleting;
    int             gl_isEditing;
    int             gl_isSelected;
    int             gl_isGraphOnParent;
    int             gl_isOpenedAtLoad;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist         *glist_getTop           (t_glist *glist);
t_environment   *glist_getEnvironment   (t_glist *glist);
t_glist         *glist_getView          (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int     glist_isRoot                    (t_glist *glist);
int     glist_isTop                     (t_glist *glist);
int     glist_isAbstraction             (t_glist *glist);
int     glist_isSubpatch                (t_glist *glist);
int     glist_isArray                   (t_glist *glist);
int     glist_isDirty                   (t_glist *glist);
int     glist_isOnScreen                (t_glist *glist);
int     glist_isWindowable              (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    glist_objectMake                (t_glist *glist, int a, int b, int w, int selected, t_buffer *t);
void    glist_objectAddNext             (t_glist *glist, t_gobj *first, t_gobj *next);
void    glist_objectAdd                 (t_glist *glist, t_gobj *y);
void    glist_objectRemove              (t_glist *glist, t_gobj *y);
void    glist_objectRemoveByTemplate    (t_glist *glist, t_template *tmpl);
void    glist_objectRemoveAll           (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int     glist_fileExist                 (t_glist *glist, char *name, char *extension, t_fileproperties *p);
int     glist_fileOpen                  (t_glist *glist, char *name, char *extension, t_fileproperties *p);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    glist_setName                   (t_glist *glist, t_symbol *name);
void    glist_setDirty                  (t_glist *glist, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void glist_setMapped (t_glist *glist, int n)
{
    glist->gl_isMapped = (n != 0);
}

static inline void glist_setWindow (t_glist *glist, int n)
{
    glist->gl_hasWindow = (n != 0);
}

static inline void glist_setEditMode (t_glist *glist, int n)
{
    glist->gl_isEditing = (n != 0);
}

static inline void glist_setNext (t_glist *glist, t_glist *next)
{
    glist->gl_next = next;
}

static inline void glist_setSelected (t_glist *glist, int n)
{
    glist->gl_isSelected = n;
}

static inline void glist_setGraphOnParent (t_glist *glist, int n)
{
    glist->gl_isGraphOnParent = n;
}

static inline void glist_setOpenedAtLoad (t_glist *glist, int n)
{
    glist->gl_isOpenedAtLoad = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void glist_loadBegin (t_glist *glist)
{
    glist->gl_isLoading = 1;
}

static inline void glist_loadEnd (t_glist *glist)
{
    glist->gl_isLoading = 0;
}

static inline void glist_deleteBegin (t_glist *glist)
{
    glist->gl_isDeleting++;
}

static inline void glist_deleteEnd (t_glist *glist)
{
    glist->gl_isDeleting--;
}

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

static inline char *glist_getTagAsString (t_glist *glist)
{
    return editor_getTagAsString (glist_getEditor (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int glist_hasParent (t_glist *glist)
{
    return (glist->gl_parent != NULL);
}

static inline int glist_hasParentOnScreen (t_glist *glist)
{
    return (glist_hasParent (glist) && glist_isOnScreen (glist_getParent (glist)));
}

static inline int glist_hasWindow (t_glist *glist)
{
    return glist->gl_hasWindow;
}

static inline int glist_hasEditMode (t_glist *glist)
{
    return glist->gl_isEditing;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int glist_isLoading (t_glist *glist)
{
    return glist->gl_isLoading;
}

static inline int glist_isDeleting (t_glist *glist)
{
    return (glist->gl_isDeleting > 0);
}

static inline int glist_isSelected (t_glist *glist)
{
    return glist->gl_isSelected;
}

static inline int glist_isGraphOnParent (t_glist *glist)
{
    return glist->gl_isGraphOnParent;
}

static inline int glist_isOpenedAtLoad (t_glist *glist)
{
    return glist->gl_isOpenedAtLoad;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_glist_h_
