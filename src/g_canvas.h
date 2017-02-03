
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

#define TYPE_COMMENT                    0
#define TYPE_OBJECT                     1
#define TYPE_MESSAGE                    2
#define TYPE_ATOM                       3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define ACTION_NONE                     0
#define ACTION_MOVE                     1
#define ACTION_CONNECT                  2
#define ACTION_REGION                   3
#define ACTION_PASS                     4
#define ACTION_DRAG                     5
#define ACTION_RESIZE                   6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CURSOR_NOTHING                  0                   /* Must NOT be changed. */
#define CURSOR_CLICK                    1                   /* Must NOT be changed. */
#define CURSOR_THICKEN                  2
#define CURSOR_ADD                      3
#define CURSOR_CONNECT                  4
#define CURSOR_RESIZE                   5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MODIFIER_NONE                   0
#define MODIFIER_SHIFT                  1
#define MODIFIER_CTRL                   2                   /* Command key on Mac OS X. */
#define MODIFIER_ALT                    4
#define MODIFIER_RIGHT                  8
#define MODIFIER_DOUBLE                 16

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BOXTEXT_DOWN                    1
#define BOXTEXT_DRAG                    2
#define BOXTEXT_DOUBLE                  3
#define BOXTEXT_SHIFT                   4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INLET_WIDTH                     7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_APPLE
    #define INLET_HEIGHT                2
#else
    #define INLET_HEIGHT                1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define INLET_OFFSET(width, i, n)       ((((width) - INLET_WIDTH) * (i)) / (((n) == 1) ? 1 : ((n) - 1)))
#define INLET_MIDDLE(width, i, n)       INLET_OFFSET (width, i, n) + ((INLET_WIDTH - 1) / 2)
#define INLET_NEXTTO(y, xA, xB, n)      (((y - xA) * (n - 1) + ((xB - xA) / 2)) / (xB - xA))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define WINDOW_WIDTH                    450
#define WINDOW_HEIGHT                   300

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_APPLE
    #define WINDOW_HEADER               22
#else
    #define WINDOW_HEADER               50
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_GUISTUB                      ".guistub"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GRAPH_DEFAULT_X                 40
#define GRAPH_DEFAULT_Y                 40
#define GRAPH_DEFAULT_WIDTH             200
#define GRAPH_DEFAULT_HEIGHT            140

#define GRAPH_DEFAULT_START             0.0
#define GRAPH_DEFAULT_UP                1.0
#define GRAPH_DEFAULT_END               100.0
#define GRAPH_DEFAULT_DOWN             -1.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _environment {
    int                 ce_dollarZeroValue;
    int                 ce_argc;
    t_atom              *ce_argv;
    t_symbol            *ce_directory;
    t_symbol            *ce_fileName;
};

typedef struct _linetraverser {
    t_glist             *tr_owner;
    t_outconnect        *tr_connectionCached;
    t_object            *tr_srcObject;
    t_outlet            *tr_srcOutlet;
    t_object            *tr_destObject;
    t_inlet             *tr_destInlet;
    int                 tr_srcIndexOfOutlet;
    int                 tr_srcIndexOfNextOutlet;
    int                 tr_srcNumberOfOutlets;
    int                 tr_destIndexOfInlet;
    int                 tr_destNumberOfInlets;
    int                 tr_srcTopLeftX;
    int                 tr_srcTopLeftY;
    int                 tr_srcBottomRightX;
    int                 tr_srcBottomRightY;
    int                 tr_destTopLeftX;
    int                 tr_destTopLeftY;
    int                 tr_destBottomRightX;
    int                 tr_destBottomRightY;
    int                 tr_lineStartX;
    int                 tr_lineStartY;
    int                 tr_lineEndX;
    int                 tr_lineEndY;
    } t_linetraverser;
    
typedef struct _selection {
    t_gobj              *sel_what;
    struct _selection   *sel_next;
    } t_selection;
    
typedef struct _editor {
    t_guiconnect        *e_guiconnect;
    t_boxtext           *e_boxtexts;
    t_boxtext           *e_selectedText;
    t_selection         *e_selectedObjects;
    t_gobj              *e_grabbed;
    t_buffer            *e_buffer;
    t_clock             *e_clock;
    t_outconnect        *e_selectedLineConnection;
    int                 e_selectedLineIndexOfObjectOut;
    int                 e_selectedLineIndexOfOutlet;
    int                 e_selectedLineIndexOfObjectIn;
    int                 e_selectedLineIndexOfInlet;
    t_motionfn          e_fnMotion;
    int                 e_previousX;
    int                 e_previousY;
    int                 e_newX;
    int                 e_newY;
    int                 e_action;
    int                 e_isTextDirty;
    int                 e_isSelectedline;
    } t_editor;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _glist {  
    t_object            gl_obj;                     /* MUST be the first. */
    t_gobj              *gl_graphics;
    t_gmaster           *gl_master;
    t_glist             *gl_parent;
    t_glist             *gl_next;
    t_environment       *gl_environment;
    t_symbol            *gl_name;
    t_editor            *gl_editor;
    t_unique            gl_uniqueIdentifier;
    int                 gl_graphWidth;
    int                 gl_graphHeight;
    int                 gl_graphMarginLeft;
    int                 gl_graphMarginTop;
    t_float             gl_valueLeft;
    t_float             gl_valueRight;
    t_float             gl_valueTop;
    t_float             gl_valueBottom;
    int                 gl_windowTopLeftX;
    int                 gl_windowTopLeftY;
    int                 gl_windowBottomRightX;
    int                 gl_windowBottomRightY;
    t_fontsize          gl_fontSize;
    char                gl_isMapped;
    char                gl_isDirty;
    char                gl_isLoading;
    char                gl_isDeleting;
    char                gl_isEditMode;
    char                gl_isSelected;
    char                gl_isGraphOnParent;
    char                gl_hasWindow;
    char                gl_hideText;                /* Unused but kept for compatibility. */
    char                gl_saveScalar;
    char                gl_openedAtLoad;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_guiconnect    *guiconnect_new                         (t_pd *owner);
char            *guiconnect_getBoundAsString            (t_guiconnect *x);

void            guiconnect_release                      (t_guiconnect *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol        *environment_getActiveFilename          (void);
t_environment   *environment_fetchActiveIfAny           (void);

void            environment_free                        (t_environment *environment);
void            environment_setActiveFile               (t_symbol *name, t_symbol *directory);
void            environment_setActiveArguments          (int argc, t_atom *argv);
void            environment_resetActiveArguments        (void);

int             environment_getDollarZero               (t_environment *environment);
int             environment_getNumberOfArguments        (t_environment *environment);

t_atom          *environment_getArguments               (t_environment *environment);
t_symbol        *environment_getDirectory               (t_environment *environment);
t_symbol        *environment_getFileName                (t_environment *environment);
char            *environment_getDirectoryAsString       (t_environment *environment);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_newPatch                         (void *dummy, t_symbol *name, t_symbol *directory);

t_symbol        *canvas_expandDollar                    (t_glist *glist, t_symbol *s);

t_glist         *canvas_getCurrent                      (void);
t_glist         *canvas_getRoot                         (t_glist *glist);
t_environment   *canvas_getEnvironment                  (t_glist *glist);
t_glist         *canvas_getView                         (t_glist *glist);
t_symbol        *canvas_getName                         (t_glist *glist);

void            canvas_setName                          (t_glist *glist, t_symbol *name);

void            canvas_makeTextObject                   (t_glist *glist, 
                                                            int positionX, 
                                                            int positionY, 
                                                            int width, 
                                                            int isSelected, 
                                                            t_buffer *b);

void            canvas_addScalarNext                    (t_glist *glist, t_scalar *first, t_scalar *next);
void            canvas_addObject                        (t_glist *glist, t_gobj *y);
void            canvas_removeObject                     (t_glist *glist, t_gobj *y);
void            canvas_clear                            (t_glist *glist);

void            canvas_setAsGraphOnParent               (t_glist *glist, int flags);
int             canvas_canHaveWindow                    (t_glist *glist);

int             canvas_isMapped                         (t_glist *glist);
int             canvas_isRoot                           (t_glist *glist);
int             canvas_isAbstraction                    (t_glist *glist);
int             canvas_isSubpatch                       (t_glist *glist);
int             canvas_isDirty                          (t_glist *glist);
int             canvas_isGraph                          (t_glist *glist);

int             canvas_openFileExist                    (t_glist *glist,
                                                            const char *name,
                                                            const char *extension);
                                                            
int             canvas_openFile                         (t_glist *glist,
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

t_gobj          *canvas_getHitObject                    (t_glist *glist, 
                                                            int positionX, 
                                                            int positionY,
                                                            int *a, 
                                                            int *b, 
                                                            int *c, 
                                                            int *d);

int             canvas_hasLine                          (t_glist *glist,
                                                            t_object *objectOut,
                                                            int m,
                                                            t_object *objectIn,
                                                            int n);
                                                            
void            canvas_setLastMotionCoordinates         (t_glist *glist, int a, int b);
void            canvas_getLastMotionCoordinates         (t_glist *glist, int *a, int *b);

int             canvas_getIndexOfObject                 (t_glist *glist, t_gobj *y);
t_gobj          *canvas_getObjectAtIndex                (t_glist *glist, int n);

void            canvas_traverseLinesStart               (t_linetraverser *t, t_glist *glist);
t_outconnect    *canvas_traverseLinesNext               (t_linetraverser *t);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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

void            canvas_selectObjectsInRectangle         (t_glist *glist, int a, int b, int c, int d);
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

void            canvas_createEditorIfNone               (t_glist *glist);
void            canvas_destroyEditorIfAny               (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_makeGraphWithArray               (t_glist *glist,
                                                            t_symbol *name,
                                                            t_float size,
                                                            t_float flags);
                                                            
void            canvas_makeArray                        (t_glist *glist,
                                                            t_symbol *s,
                                                            t_symbol *type,
                                                            t_float size,
                                                            t_float flags);
                                                            
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

t_inlet         *canvas_addInlet                        (t_glist *glist, t_pd *owner, t_symbol *s);
t_outlet        *canvas_addOutlet                       (t_glist *glist, t_pd *owner, t_symbol *s);

void            canvas_removeInlet                      (t_glist *glist, t_inlet *inlet);
void            canvas_removeOutlet                     (t_glist *glist, t_outlet *outlet);

void            canvas_resortInlets                     (t_glist *glist);
void            canvas_resortOutlets                    (t_glist *glist);

void            canvas_bounds                           (t_glist *glist,
                                                            t_float a,
                                                            t_float b,
                                                            t_float c,
                                                            t_float d);
                                                            
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

t_glist         *canvas_newGraphOnParent                (t_glist *glist,
                                                            t_float valueStart,
                                                            t_float valueUp,
                                                            t_float valueEnd,
                                                            t_float valueDown,
                                                            t_float topLeftX,
                                                            t_float topLeftY,
                                                            t_float bottomRightX,
                                                            t_float bottomRightY);
                                                            
t_glist         *canvas_new                             (void *dummy, t_symbol *s, int argc, t_atom *argv);

void            canvas_free                             (t_glist *glist);

void            canvas_click                            (t_glist *glist,
                                                            t_float a,
                                                            t_float b,
                                                            t_float shift,
                                                            t_float ctrl,
                                                            t_float alt);

void            canvas_motion                           (t_glist *glist,
                                                            t_float positionX,
                                                            t_float positionY,
                                                            t_float modifier);

void            canvas_mouse                            (t_glist *glist,
                                                            t_float a,
                                                            t_float b,
                                                            t_float modifier);

void            canvas_mouseUp                          (t_glist *glist,
                                                            t_float a,
                                                            t_float b);

void            canvas_window                           (t_glist *glist,
                                                            t_float a,
                                                            t_float b,
                                                            t_float c,
                                                            t_float d);
                                                            
void            canvas_connect                          (t_glist *glist,
                                                            t_float indexOfObjectOut,
                                                            t_float indexOfOutlet,
                                                            t_float indexOfObjectIn,
                                                            t_float indexOfInlet);

void            canvas_disconnect                       (t_glist *glist,
                                                            t_float indexOfObjectOut,
                                                            t_float indexOfOutlet,
                                                            t_float indexOfObjectIn,
                                                            t_float indexOfInlet);
                                                            
void            canvas_key                              (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_restore                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
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
void            canvas_saveToFile                       (t_glist *glist, 
                                                            t_symbol *name,
                                                            t_symbol *directory, 
                                                            float destroy);

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
#pragma mark -

void            gobj_getRectangle                       (t_gobj *x,
                                                            t_glist *owner,
                                                            int *a,
                                                            int *b,
                                                            int *c,
                                                            int *d);
                                                            
void            gobj_displaced                          (t_gobj *x, t_glist *owner, int deltaX, int deltaY);
void            gobj_selected                           (t_gobj *x, t_glist *owner, int isSelected);
void            gobj_activated                          (t_gobj *x, t_glist *owner, int isActivated);
void            gobj_deleted                            (t_gobj *x, t_glist *owner);
void            gobj_visibilityChanged                  (t_gobj *x, t_glist *owner, int isVisible);
int             gobj_mouse                              (t_gobj *x,
                                                            t_glist *owner,
                                                            int a,
                                                            int b,
                                                            int shift,
                                                            int ctrl,
                                                            int alt,
                                                            int dbl,
                                                            int clicked);
                                                        
void            gobj_save                               (t_gobj *x, t_buffer *buffer);
int             gobj_hit                                (t_gobj *x,
                                                            t_glist *owner,
                                                            int positionX,
                                                            int positionY,
                                                            int *a,
                                                            int *b,
                                                            int *c,
                                                            int *d);
                                                            
int             gobj_isVisible                          (t_gobj *x, t_glist *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int             object_isBox                            (t_object *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_boxtext       *boxtext_new                            (t_glist *glist, t_object *object);
t_boxtext       *boxtext_fetch                          (t_glist *glist, t_object *object);
char            *boxtext_getTag                         (t_boxtext *x);

void            boxtext_retext                          (t_glist *glist, t_object *object);

void            boxtext_free                            (t_boxtext *x);
void            boxtext_restore                         (t_boxtext *x);
int             boxtext_getWidth                        (t_boxtext *x);
int             boxtext_getHeight                       (t_boxtext *x);
void            boxtext_getText                         (t_boxtext *x, char **p, int *size);
void            boxtext_getSelection                    (t_boxtext *x, char **p, int *size);
void            boxtext_draw                            (t_boxtext *x);
void            boxtext_update                          (t_boxtext *x);
void            boxtext_erase                           (t_boxtext *x);
void            boxtext_displace                        (t_boxtext *x, int deltaX, int deltaY);
void            boxtext_select                          (t_boxtext *x, int isSelected);
void            boxtext_activate                        (t_boxtext *x, int state);
void            boxtext_mouse                           (t_boxtext *x, int a, int b, int flag);
void            boxtext_key                             (t_boxtext *x, t_keycode n, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            message_makeObject                      (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

void            message_click                           (t_message *x, 
                                                            t_float a,
                                                            t_float b,
                                                            t_float shift,
                                                            t_float ctrl,
                                                            t_float alt);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            text_behaviorGetRectangle               (t_gobj *x,
                                                            t_glist *glist,
                                                            int *a,
                                                            int *b,
                                                            int *c,
                                                            int *d);
                                                            
void            text_behaviorDisplaced                  (t_gobj *x, t_glist *glist, int deltaX, int deltaY);
void            text_behaviorSelected                   (t_gobj *x, t_glist *glist, int isSelected);
void            text_behaviorActivated                  (t_gobj *x, t_glist *glist, int isActivated);
void            text_behaviorDeleted                    (t_gobj *x, t_glist *glist);
void            text_behaviorVisibilityChanged          (t_gobj *x, t_glist *glist, int isVisible);
int             text_behaviorMouse                      (t_gobj *x,
                                                            t_glist *glist,
                                                            int a,
                                                            int b,
                                                            int shift,
                                                            int ctrl,
                                                            int alt,
                                                            int dbl,
                                                            int clicked);

void            text_functionSave                       (t_gobj *x, t_buffer *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void            text_set                                (t_object *x, t_glist *glist, char *s, int size);
int             text_getPixelX                          (t_object *x, t_glist *glist);
int             text_getPixelY                          (t_object *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            *table_makeObject                       (t_symbol *name,
                                                            t_float down, 
                                                            t_float up,
                                                            t_float size,
                                                            t_float flags);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            gatom_makeObject                        (t_glist *glist, 
                                                            t_atomtype type,
                                                            t_symbol *s,
                                                            int argc,
                                                            t_atom *argv);

void            gatom_click                             (t_gatom *x,
                                                            t_float a,
                                                            t_float b,
                                                            t_float shift,
                                                            t_float ctrl,
                                                            t_float alt);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_garray        *garray_makeObject                      (t_glist *glist,
                                                            t_symbol *name,
                                                            t_float size,
                                                            t_float flags);
                                                            
t_array         *garray_getArray                        (t_garray *x);
t_glist         *garray_getView                         (t_garray *x);
t_scalar        *garray_getScalar                       (t_garray *x);
t_symbol        *garray_getName                         (t_garray *x);

int             garray_isSingle                         (t_glist *glist);

int             garray_getSize                          (t_garray *x);         
int             garray_getData                          (t_garray *x, int *size, t_word **w);
void            garray_setDataAtIndex                   (t_garray *x, int i, t_float f);
t_float         garray_getDataAtIndex                   (t_garray *x, int i);
void            garray_setDataFromIndex                 (t_garray *x, int i, t_float f);
t_float         garray_getAmplitude                     (t_garray *x);
void            garray_setAsUsedInDSP                   (t_garray *x);
void            garray_setSaveWithParent                (t_garray *x, int savedWithParent);
void            garray_redraw                           (t_garray *x);
void            garray_resizeWithInteger                (t_garray *x, int n);
void            garray_saveContentsToBuffer             (t_garray *x, t_buffer *b);
void            garray_functionProperties               (t_garray *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet         *vinlet_getInlet                        (t_pd *x);
t_outlet        *voutlet_getOutlet                      (t_pd *x);

int             vinlet_isSignal                         (t_vinlet *x);
int             voutlet_isSignal                        (t_voutlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_canvas_h_
