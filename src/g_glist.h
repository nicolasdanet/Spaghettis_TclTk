
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
    t_object        gl_obj;                             /* MUST be the first. */
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
    char            gl_hideText;                        /* Unused but kept for compatibility. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist         *canvas_getRoot                         (t_glist *glist);
t_environment   *canvas_getEnvironment                  (t_glist *glist);
t_glist         *canvas_getView                         (t_glist *glist);
t_symbol        *canvas_getName                         (t_glist *glist);

void            canvas_setName                          (t_glist *glist, t_symbol *name);
int             canvas_canHaveWindow                    (t_glist *glist);
int             canvas_isMapped                         (t_glist *glist);
int             canvas_isRoot                           (t_glist *glist);
int             canvas_isAbstraction                    (t_glist *glist);
int             canvas_isSubpatch                       (t_glist *glist);
int             canvas_isDirty                          (t_glist *glist);
int             canvas_isGraph                          (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_glist_h_
