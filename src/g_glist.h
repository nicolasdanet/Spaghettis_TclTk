
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_glist_h_
#define __g_glist_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _glist {  
    t_object            gl_obj;                         /* MUST be the first. */
    t_gobj              *gl_graphics;
    t_gmaster           *gl_holder;
    t_glist             *gl_parent;
    t_glist             *gl_next;
    t_environment       *gl_environment;
    t_symbol            *gl_name;
    t_editor            *gl_editor;
    t_clock             *gl_clock;
    t_unique            gl_uniqueIdentifier;
    t_bounds            gl_bounds;
    t_rectangle         gl_geometryGraph;
    t_rectangle         gl_geometryWindow;
    t_fontsize          gl_fontSize;
    int                 gl_hasWindow;
    int                 gl_isMapped;
    int                 gl_isDirty;
    int                 gl_isFrozen;
    int                 gl_isLoading;
    int                 gl_isDeleting;
    int                 gl_isEditing;
    int                 gl_isSelected;
    int                 gl_isGraphOnParent;
    int                 gl_isOpenedAtLoad;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist         *glist_newPatchPop          (t_symbol *name,
                                                t_bounds    *bounds, 
                                                t_rectangle *graph,
                                                t_rectangle *window, 
                                                int isOpened,
                                                int isEditMode,
                                                int isGOP,
                                                int fontSize);

t_glist         *glist_newPatch             (t_symbol *name,
                                                t_bounds    *bounds, 
                                                t_rectangle *graph,
                                                t_rectangle *window, 
                                                int isOpened,
                                                int isEditMode,
                                                int isGOP,
                                                int fontSize);
                                                            
void            glist_free                  (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist         *glist_getTop               (t_glist *g);
t_environment   *glist_getEnvironment       (t_glist *g);
t_glist         *glist_getView              (t_glist *g);
t_garray        *glist_getArray             (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     glist_isRoot                        (t_glist *g);
int     glist_isTop                         (t_glist *g);
int     glist_isAbstraction                 (t_glist *g);
int     glist_isSubpatch                    (t_glist *g);
int     glist_isArray                       (t_glist *g);
int     glist_isDirty                       (t_glist *g);
int     glist_isFrozen                      (t_glist *g);
int     glist_isOnScreen                    (t_glist *g);
int     glist_isParentOnScreen              (t_glist *g);
int     glist_isWindowable                  (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_bind                          (t_glist *g);
void    glist_unbind                        (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void    glist_loadbang                      (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void    glist_serialize                     (t_glist *g, t_buffer *b);
void    glist_rename                        (t_glist *g, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int     glist_fileExist                     (t_glist *g, char *name, char *extension, t_fileproperties *p);
int     glist_fileOpen                      (t_glist *g, char *name, char *extension, t_fileproperties *p);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_setName                       (t_glist *g, t_symbol *name);
void    glist_setDirty                      (t_glist *g, int n);
void    glist_setFrozen                     (t_glist *g, int n);
void    glist_setFontSize                   (t_glist *g, int n);
void    glist_setMotion                     (t_glist *g, t_gobj *y, t_motionfn fn, int a, int b);
void    glist_setBounds                     (t_glist *g, t_bounds *bounds);
void    glist_setGraphGeometry              (t_glist *g, t_rectangle *r, t_bounds *bounds, int isGOP);
void    glist_setWindowGeometry             (t_glist *g, t_rectangle *r);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_cancelEditingBox              (t_glist *glist);

void    glist_key                           (t_glist *g, t_keycode n, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_objectMake                    (t_glist *g, int a, int b, int w, int selected, t_buffer *t);
void    glist_objectMakeScalar              (t_glist *g, int argc, t_atom *argv);
void    glist_objectSetWidthOfLast          (t_glist *g, int w);
void    glist_objectAddNext                 (t_glist *g, t_gobj *y, t_gobj *first);
void    glist_objectAdd                     (t_glist *g, t_gobj *y);
void    glist_objectRemove                  (t_glist *g, t_gobj *y);
void    glist_objectRemoveAllByTemplate     (t_glist *g, t_template *tmpl);
void    glist_objectRemoveAllScalars        (t_glist *g);
void    glist_objectRemoveAll               (t_glist *g);

void    glist_objectSelect                  (t_glist *g, t_gobj *y);
void    glist_objectSelectIfNotSelected     (t_glist *g, t_gobj *y);
int     glist_objectDeselect                (t_glist *g, t_gobj *y);
int     glist_objectDeselectIfSelected      (t_glist *g, t_gobj *y);
int     glist_objectIsSelected              (t_glist *g, t_gobj *y);
void    glist_objectSwapSelected            (t_glist *g, t_gobj *y);
void    glist_objectMoveAtFirst             (t_glist *g, t_gobj *y);
void    glist_objectMoveAtLast              (t_glist *g, t_gobj *y);
int     glist_objectGetIndexOf              (t_glist *g, t_gobj *y);
int     glist_objectGetIndexAmongSelected   (t_glist *g, t_gobj *y);
int     glist_objectGetNumberOf             (t_glist *g);
int     glist_objectGetNumberOfSelected     (t_glist *g);
void    glist_objectRemoveSelected          (t_glist *g);
void    glist_objectSnapSelected            (t_glist *g);
void    glist_objectMoveSelected            (t_glist *g, int backward);
void    glist_objectDisplaceSelected        (t_glist *g, int deltaX, int deltaY);
void    glist_objectDeleteLines             (t_glist *g, t_object *o);
void    glist_objectDeleteLinesByInlet      (t_glist *g, t_object *o, t_inlet *inlet);
void    glist_objectDeleteLinesByOutlet     (t_glist *g, t_object *o, t_outlet *outlet);

t_gobj  *glist_objectGetAt                  (t_glist *g, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_inlet     *glist_inletAdd                 (t_glist *g, t_pd *receiver, int isSignal);

void        glist_inletRemove               (t_glist *g, t_inlet *inlet);
int         glist_inletNumberOf             (t_glist *g);
void        glist_inletSort                 (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_outlet    *glist_outletAdd                (t_glist *g, t_symbol *s);

void        glist_outletRemove              (t_glist *g, t_outlet *outlet);
int         glist_outletNumberOf            (t_glist *g);
void        glist_outletSort                (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_lineSelect                    (t_glist *g, t_traverser *t);
void    glist_lineDeselect                  (t_glist *g);
void    glist_lineDeleteSelected            (t_glist *g);
int     glist_lineExist                     (t_glist *g, t_object *o, int m, t_object *i, int n);
void    glist_lineCheck                     (t_glist *g, t_object *o);
t_error glist_lineConnect                   (t_glist *g, int m, int i, int n, int j);
t_error glist_lineDisconnect                (t_glist *g, int m, int i, int n, int j);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_selectLassoBegin              (t_glist *g, int a, int b);
void    glist_selectLassoEnd                (t_glist *g, int a, int b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     glist_deselectAll                   (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_updateTitle                   (t_glist *g);
void    glist_updateWindow                  (t_glist *g);
void    glist_updateLinesForObject          (t_glist *g, t_object *o);
void    glist_updateLineSelected            (t_glist *g, int isSelected);
void    glist_updateRectangleOnParent       (t_glist *g);
void    glist_updateRectangle               (t_glist *g);
void    glist_updateLasso                   (t_glist *g, int a, int b);
void    glist_updateTemporary               (t_glist *g, int a, int b);
void    glist_updateLine                    (t_glist *g, t_cord *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_drawAllLines                  (t_glist *g);
void    glist_drawRectangleOnParent         (t_glist *g);
void    glist_drawRectangle                 (t_glist *g);
void    glist_drawLasso                     (t_glist *g, int a, int b);
void    glist_drawTemporary                 (t_glist *g, int a, int b);
void    glist_drawLine                      (t_glist *g, t_cord *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_eraseRectangleOnParent        (t_glist *g);
void    glist_eraseLasso                    (t_glist *g);
void    glist_eraseTemporary                (t_glist *g);
void    glist_eraseLine                     (t_glist *g, t_cord *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_windowEdit                    (t_glist *g, int isEditMode);
void    glist_windowMapped                  (t_glist *g, int isMapped);
void    glist_windowOpen                    (t_glist *g);
void    glist_windowClose                   (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    glist_redrawRequired                (t_glist *g);
void    glist_redraw                        (t_glist *g);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float glist_pixelToValueX                 (t_glist *g, t_float f);
t_float glist_pixelToValueY                 (t_glist *g, t_float f);
t_float glist_valueToPixelX                 (t_glist *g, t_float f);
t_float glist_valueToPixelY                 (t_glist *g, t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     glist_getPixelX                     (t_glist *g, t_object *x);
int     glist_getPixelY                     (t_glist *g, t_object *x);

t_float glist_getValueForOnePixelX          (t_glist *g);
t_float glist_getValueForOnePixelY          (t_glist *g);

void    glist_getRectangleOnParent          (t_glist *g, t_rectangle *r);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void glist_setMapped (t_glist *g, int n)
{
    g->gl_isMapped = (n != 0);
}

static inline void glist_setWindow (t_glist *g, int n)
{
    g->gl_hasWindow = (n != 0);
}

static inline void glist_setEditMode (t_glist *g, int n)
{
    g->gl_isEditing = (n != 0);
}

static inline void glist_setNext (t_glist *g, t_glist *next)
{
    g->gl_next = next;
}

static inline void glist_setSelected (t_glist *g, int n)
{
    g->gl_isSelected = n;
}

static inline void glist_setGraphOnParent (t_glist *g, int n)
{
    g->gl_isGraphOnParent = n;
}

static inline void glist_setOpenedAtLoad (t_glist *g, int n)
{
    g->gl_isOpenedAtLoad = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void glist_loadBegin (t_glist *g)
{
    g->gl_isLoading = 1;
}

static inline void glist_loadEnd (t_glist *g)
{
    g->gl_isLoading = 0;
}

static inline void glist_deleteBegin (t_glist *g)
{
    g->gl_isDeleting = 1;
}

static inline void glist_deleteEnd (t_glist *g)
{
    g->gl_isDeleting = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_glist *glist_getParent (t_glist *g)
{
    return g->gl_parent;
}

static inline t_glist *glist_getNext (t_glist *g)
{
    return g->gl_next;
}

static inline t_symbol *glist_getName (t_glist *g)
{
    return g->gl_name;
}

static inline t_editor *glist_getEditor (t_glist *g)
{
    return g->gl_editor;
}

static inline t_bounds *glist_getBounds (t_glist *g)
{
    return &g->gl_bounds;
}

static inline t_rectangle *glist_getGraphGeometry (t_glist *g)
{
    return &g->gl_geometryGraph;
}

static inline t_rectangle *glist_getWindowGeometry (t_glist *g)
{
    return &g->gl_geometryWindow;
}

static inline t_gmaster *glist_getMaster (t_glist *g)
{
    return g->gl_holder;
}

static inline t_unique glist_getIdentifier (t_glist *g)
{
    return g->gl_uniqueIdentifier;
}

static inline t_fontsize glist_getFontSize (t_glist *g)
{
    return g->gl_fontSize;
}

static inline int glist_getMapped (t_glist *g)
{
    return g->gl_isMapped;
}

static inline int glist_getDirty (t_glist *g)
{
    return g->gl_isDirty;
}

static inline char *glist_getTagAsString (t_glist *g)
{
    return editor_getTagAsString (glist_getEditor (g));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int glist_hasView (t_glist *g)
{
    return (g == glist_getView (g));
}

static inline int glist_hasParent (t_glist *g)
{
    return (g->gl_parent != NULL);
}

static inline int glist_hasWindow (t_glist *g)
{
    return g->gl_hasWindow;
}

static inline int glist_hasEditMode (t_glist *g)
{
    return g->gl_isEditing;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int glist_isEditable (t_glist *g)
{
    return (!glist_isAbstraction (g) && !glist_isArray (g));
}

static inline int glist_isLoading (t_glist *g)
{
    return g->gl_isLoading;
}

static inline int glist_isDeleting (t_glist *g)
{
    return g->gl_isDeleting;
}

static inline int glist_isSelected (t_glist *g)
{
    return g->gl_isSelected;
}

static inline int glist_isGraphOnParent (t_glist *g)
{
    return g->gl_isGraphOnParent;
}

static inline int glist_isOpenedAtLoad (t_glist *g)
{
    return g->gl_isOpenedAtLoad;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_glist_h_
