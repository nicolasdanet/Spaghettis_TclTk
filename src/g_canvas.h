
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_canvas_h_
#define __g_canvas_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_motionfn)  (void *z, t_float deltaX, t_float deltaY, t_float modifier);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _selection {
    t_gobj              *sel_what;
    struct _selection   *sel_next;
    } t_selection;
    
typedef struct _editor {
    t_proxy             *e_proxy;
    t_box               *e_boxes;
    t_box               *e_selectedText;
    t_selection         *e_selectedObjects;
    t_gobj              *e_grabbed;
    t_clock             *e_clock;
    t_buffer            *e_cachedLines;
    t_outconnect        *e_selectedLineConnection;
    t_atom              e_selectedLine[4];
    t_motionfn          e_fnMotion;
    int                 e_previousX;
    int                 e_previousY;
    int                 e_newX;
    int                 e_newY;
    int                 e_action;
    int                 e_isTextDirty;
    int                 e_hasSelectedline;
    } t_editor;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _glist {  
    t_object            gl_obj;                         /* MUST be the first. */
    t_gobj              *gl_graphics;
    t_gmaster           *gl_holder;
    t_glist             *gl_parent;
    t_glist             *gl_next;
    t_environment       *gl_environment;
    t_symbol            *gl_name;
    t_editor            *gl_editor;
    t_unique            gl_uniqueIdentifier;
    t_bounds            gl_bounds;
    t_rectangle         gl_geometryGraph;
    t_rectangle         gl_geometryWindow;
    t_fontsize          gl_fontSize;
    char                gl_isMapped;
    char                gl_isDirty;
    char                gl_isLoading;
    char                gl_isDeleting;
    char                gl_isEditMode;
    char                gl_isSelected;
    char                gl_isGraphOnParent;
    char                gl_hasWindow;
    char                gl_hideText;                    /* Unused but kept for compatibility. */
    char                gl_saveScalar;
    char                gl_openedAtLoad;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_newPatch                         (void *dummy, t_symbol *name, t_symbol *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol        *canvas_expandDollar                    (t_glist *glist, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist         *canvas_getCurrent                      (void);
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
#pragma mark -

void            canvas_makeTextObject                   (t_glist *glist, 
                                                            int positionX, 
                                                            int positionY, 
                                                            int width, 
                                                            int isSelected, 
                                                            t_buffer *b);

void            canvas_addScalarNext                    (t_glist *glist, t_scalar *first, t_scalar *next);
void            canvas_addObject                        (t_glist *glist, t_gobj *y);
void            canvas_removeObject                     (t_glist *glist, t_gobj *y);
void            canvas_removeScalarsRecursive           (t_glist *glist, t_template *tmpl);
void            canvas_clear                            (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int             canvas_fileExist                        (t_glist *glist,
                                                            const char *name,
                                                            const char *extension);

int             canvas_fileFind                         (t_glist *glist,
                                                            const char *name,
                                                            const char *extension,
                                                            char *directoryResult,
                                                            char **nameResult,
                                                            size_t size);
                                                            
int             canvas_fileOpen                         (t_glist *glist,
                                                            const char *name,
                                                            const char *extension,
                                                            char *directoryResult,
                                                            char **nameResult,
                                                            size_t size);

void            canvas_bind                             (t_glist *glist);
void            canvas_unbind                           (t_glist *glist);
t_error         canvas_makeFilePath                     (t_glist *glist, char *name, char *dest, size_t size);
void            canvas_updateTitle                      (t_glist *glist);
t_fontsize      canvas_getFontSize                      (t_glist *glist);
void            canvas_setCursorType                    (t_glist *glist, int type);

t_gobj          *canvas_getHitObject                    (t_glist *glist, int a, int b, t_rectangle *r);

int             canvas_hasLine                          (t_glist *glist,
                                                            t_object *objectOut,
                                                            int m,
                                                            t_object *objectIn,
                                                            int n);
                                                            
void            canvas_setLastMotionCoordinates         (t_glist *glist, int a, int b);
void            canvas_getLastMotionCoordinates         (t_glist *glist, int *a, int *b);

int             canvas_getIndexOfObject                 (t_glist *glist, t_gobj *y);
t_gobj          *canvas_getObjectAtIndex                (t_glist *glist, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_redraw                           (t_glist *glist);
void            canvas_redrawGraphOnParent              (t_glist *glist);
void            canvas_drawLines                        (t_glist *glist);
void            canvas_updateLinesByObject              (t_glist *glist, t_object *o);
void            canvas_deleteLinesByObject              (t_glist *glist, t_object *o);
void            canvas_deleteLinesByInlets              (t_glist *glist,
                                                            t_object *o,
                                                            t_inlet  *inlet,
                                                            t_outlet *outlet);

void            canvas_drawBox                          (t_glist *glist, t_object *o, char *tag, int create);
void            canvas_eraseBox                         (t_glist *glist, t_object *o, char *tag);

void            canvas_drawGraphOnParentRectangle       (t_glist *glist);
void            canvas_updateGraphOnParentRectangle     (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_restoreCachedLines               (t_glist *glist);

void            canvas_removeSelectedObjects            (t_glist *glist);
void            canvas_removeSelectedLine               (t_glist *glist);
void            canvas_displaceSelectedObjects          (t_glist *glist, int deltaX, int deltaY);

int             canvas_isObjectSelected                 (t_glist *glist, t_gobj *y);
void            canvas_selectingByLassoStart            (t_glist *glist, int positionX, int positionY);
void            canvas_selectingByLassoEnd              (t_glist *glist, int positionX, int positionY);

void            canvas_selectObjectsInRectangle         (t_glist *glist, t_rectangle *r);
void            canvas_selectObject                     (t_glist *glist, t_gobj *y);
void            canvas_selectObjectIfNotSelected        (t_glist *glist, t_gobj *y);
void            canvas_selectLine                       (t_glist *glist, 
                                                            t_outconnect *connection,
                                                            int indexOfObjectOut,
                                                            int indexOfOutlet,
                                                            int indexOfObjectIn,
                                                            int indexOfInlet);
                                                            
int             canvas_deselectObject                   (t_glist *glist, t_gobj *y);
int             canvas_deselectObjectIfSelected         (t_glist *glist, t_gobj *y);
int             canvas_deselectAll                      (t_glist *glist);
int             canvas_getNumberOfSelectedObjects       (t_glist *glist);
int             canvas_getNumberOfUnselectedObjects     (t_glist *glist);
int             canvas_getIndexOfObjectAmongSelected    (t_glist *glist, t_gobj *y);
int             canvas_getIndexOfObjectAmongUnselected  (t_glist *glist, t_gobj *y);

void            canvas_setMotionFunction                (t_glist *glist,
                                                            t_gobj *y,
                                                            t_motionfn callback,
                                                            int a,
                                                            int b);
                                    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_createEditorIfNone               (t_glist *glist);
void            canvas_destroyEditorIfAny               (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_fromArrayDialog                  (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeArray                        (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeObject                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeMessage                      (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeFloatAtom                    (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeSymbolAtom                   (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeComment                      (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeScalar                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeBang                         (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeToggle                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeVerticalSlider               (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeHorizontalSlider             (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeHorizontalRadio              (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeVerticalRadio                (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeVu                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makePanel                        (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeDial                         (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet         *canvas_addInlet                        (t_glist *glist, t_pd *receiver, t_symbol *s);
t_outlet        *canvas_addOutlet                       (t_glist *glist, t_symbol *s);

void            canvas_removeInlet                      (t_glist *glist, t_inlet *inlet);
void            canvas_removeOutlet                     (t_glist *glist, t_outlet *outlet);

void            canvas_resortInlets                     (t_glist *glist);
void            canvas_resortOutlets                    (t_glist *glist);

void            canvas_bounds                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
                                                            
t_float         canvas_valueToPixelX                    (t_glist *glist, t_float f);
t_float         canvas_valueToPixelY                    (t_glist *glist, t_float f);
t_float         canvas_pixelToValueX                    (t_glist *glist, t_float f);
t_float         canvas_pixelToValueY                    (t_glist *glist, t_float f);

t_float         canvas_valueForDeltaInPixelX            (t_glist *glist, t_float f);
t_float         canvas_valueForDeltaInPixelY            (t_glist *glist, t_float f);
t_float         canvas_valueForOnePixelX                (t_glist *glist);
t_float         canvas_valueForOnePixelY                (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist         *canvas_new                             (void *dummy, t_symbol *s, int argc, t_atom *argv);

void            canvas_free                             (t_glist *glist);

void            canvas_click                            (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_motion                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_mouse                            (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_mouseUp                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_window                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_connect                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_disconnect                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_key                              (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_restore                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_rename                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_loadbang                         (t_glist *glist);
void            canvas_editmode                         (t_glist *glist, t_float f);
void            canvas_close                            (t_glist *glist, t_float f);
void            canvas_dirty                            (t_glist *glist, t_float f);
void            canvas_visible                          (t_glist *glist, t_float f);
void            canvas_map                              (t_glist *glist, t_float f);
void            canvas_pop                              (t_glist *glist, t_float f);

void            canvas_cut                              (t_glist *glist);
void            canvas_copy                             (t_glist *glist);
void            canvas_paste                            (t_glist *glist);
void            canvas_duplicate                        (t_glist *glist);
void            canvas_selectAll                        (t_glist *glist);

void            canvas_save                             (t_glist *glist, float destroy);
void            canvas_saveAs                           (t_glist *glist, float destroy);
void            canvas_saveToFile                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

void            canvas_serialize                        (t_glist *glist, t_buffer *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_dsp                              (t_glist *glist, t_signal **sp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_serializeTemplates               (t_glist *glist, t_buffer *b);
t_error         canvas_deserializeScalar                (t_glist *glist, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_canvas_h_
