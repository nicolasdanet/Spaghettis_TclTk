
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

#define DATA_FLOAT                      0
#define DATA_SYMBOL                     1
#define DATA_TEXT                       2
#define DATA_ARRAY                      3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define POINTER_NONE                    0
#define POINTER_GLIST                   1
#define POINTER_ARRAY                   2

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

#define PLOT_POINTS                     0
#define PLOT_POLYGONS                   1
#define PLOT_CURVES                     2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CURSOR_NOTHING                  0
#define CURSOR_CLICK                    1
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

#define SCALAR_REDRAW                   0
#define SCALAR_DRAW                     1
#define SCALAR_ERASE                    2

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
#define INLET_NEXTTO(y, x1, x2, n)      (((y - x1) * (n - 1) + ((x2 - x1) / 2)) / (x2 - x1))

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

#define COLOR_MASKED                    0xdddddd        /* Grey. */
#define COLOR_NORMAL                    0x000000        /* Black. */
#define COLOR_SELECTED                  0x0000ff        /* Blue. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _environment {
    int                 ce_dollarZeroValue;
    int                 ce_argc;
    t_atom              *ce_argv;
    t_symbol            *ce_directory;
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
    t_object            gl_obj;                 /* MUST be the first. */
    t_gobj              *gl_graphics;
    t_gstub             *gl_stub;
    t_glist             *gl_parent;
    t_glist             *gl_next;
    t_environment       *gl_environment;
    t_symbol            *gl_name;
    t_editor            *gl_editor;
    int                 gl_magic;
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
    int                 gl_fontSize;
    char                gl_isMapped;
    char                gl_isDirty;
    char                gl_isLoading;
    char                gl_isDeleting;
    char                gl_isEditMode;
    char                gl_isSelected;
    char                gl_isGraphOnParent;
    char                gl_hasWindow;
    char                gl_hideText;
    char                gl_saveScalar;
    char                gl_willBeVisible;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _array {
    int                 a_size;
    int                 a_elementSize;
    char                *a_vector;
    t_symbol            *a_template;
    int                 a_valid;
    t_gpointer          a_gpointer;
    t_gstub             *a_stub;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _dataslot {
    int                 ds_type;
    t_symbol            *ds_name;
    t_symbol            *ds_template;
    } t_dataslot;

struct _template {
    t_pd                tpl_pd;            /* MUST be the first. */
    t_gtemplate         *tpl_list;  
    t_symbol            *tpl_symbol;    
    int                 tpl_size;    
    t_dataslot          *tpl_vector;  
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define canvas_castToObjectIfPatchable(x)   (pd_class (x)->c_isBox ? (t_object *)(x) : NULL)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist         *canvas_castToGlistChecked              (t_pd *x);
int             canvas_objectIsBox                      (t_object *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_guiconnect    *guiconnect_new                         (t_pd *owner, t_symbol *bindTo);
void            guiconnect_release                      (t_guiconnect *x, double timeOut);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_newPatch                         (void *dummy, t_symbol *name, t_symbol *directory);
void            canvas_setActiveFileNameAndDirectory    (t_symbol *name, t_symbol *directory);
void            canvas_setActiveArguments               (int argc, t_atom *argv);

t_glist         *canvas_getCurrent                      (void);
t_glist         *canvas_getRoot                         (t_glist *glist);
t_environment   *canvas_getEnvironment                  (t_glist *glist);
t_glist         *canvas_getView                         (t_glist *glist);

t_symbol        *canvas_expandDollar                    (t_glist *glist, t_symbol *s);
t_symbol        *canvas_makeBindSymbol                  (t_symbol *s);

t_glist         *canvas_addGraph                        (t_glist *glist,
                                                            t_symbol *name,
                                                            t_float valueStart,
                                                            t_float valueUp,
                                                            t_float valueEnd,
                                                            t_float valueDown,
                                                            t_float topLeftX,
                                                            t_float topLeftY,
                                                            t_float bottomRightX,
                                                            t_float bottomRightY);

void            canvas_makeTextObject                   (t_glist *glist, 
                                                            int positionX, 
                                                            int positionY, 
                                                            int width, 
                                                            int isSelected, 
                                                            t_buffer *b);
                                                            
void            canvas_addObject                        (t_glist *glist, t_gobj *y);
void            canvas_removeObject                     (t_glist *glist, t_gobj *y);
void            canvas_clear                            (t_glist *glist);

void            canvas_setAsGraphOnParent               (t_glist *glist, int flags);

int             canvas_isMapped                         (t_glist *glist);
int             canvas_isRoot                           (t_glist *glist);
int             canvas_isAbstraction                    (t_glist *glist);
int             canvas_isSubpatch                       (t_glist *glist);
int             canvas_isDirty                          (t_glist *glist);
int             canvas_isDrawnOnParent                  (t_glist *glist);
int             canvas_canHaveWindow                    (t_glist *glist);
int             canvas_hasEnvironment                   (t_glist *glist);

int             canvas_openFile                         (t_glist *glist,
                                                            const char *name,
                                                            const char *extension,
                                                            char *directoryResult,
                                                            char **nameResult,
                                                            size_t size);

void            canvas_bind                             (t_glist *glist);
void            canvas_unbind                           (t_glist *glist);
t_error         canvas_makeFilePath                     (t_glist *glist, char *name, char *dest, size_t size);
void            canvas_setName                          (t_glist *glist, t_symbol *name, t_symbol *directory);
void            canvas_updateTitle                      (t_glist *glist);
int             canvas_hasGraphOnParentTitle            (t_glist *glist);
int             canvas_getFontSize                      (t_glist *glist);
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
void            canvas_deleteGraphOnParentRectangle     (t_glist *glist);

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
                                                            
void            canvas_deselectObject                   (t_glist *glist, t_gobj *y);
void            canvas_deselectAll                      (t_glist *glist);
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
                                                            t_symbol *templateName,
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
                                                            
t_float         canvas_valueToPositionX                 (t_glist *glist, t_float f);
t_float         canvas_valueToPositionY                 (t_glist *glist, t_float f);
t_float         canvas_positionToValueX                 (t_glist *glist, t_float f);
t_float         canvas_positionToValueY                 (t_glist *glist, t_float f);

t_float         canvas_deltaPositionToValueX            (t_glist *glist, t_float f);
t_float         canvas_deltaPositionToValueY            (t_glist *glist, t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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

void            dsp_state                               (void *dummy, t_symbol *s, int argc, t_atom *argv);
void            dsp_update                              (void);
int             dsp_suspend                             (void);
void            dsp_resume                              (int oldState);

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
int             gobj_clicked                            (t_gobj *x,
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
int             text_behaviorClicked                    (t_gobj *x,
                                                            t_glist *glist,
                                                            int a,
                                                            int b,
                                                            int shift,
                                                            int ctrl,
                                                            int alt,
                                                            int dbl,
                                                            int clicked);

void            text_functionSave                       (t_gobj *x, t_buffer *b);

void            text_set                                (t_object *x, t_glist *glist, char *s, int size);
int             text_getPositionX                       (t_object *x, t_glist *glist);
int             text_getPositionY                       (t_object *x, t_glist *glist);

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

void            garray_initialize                       (void);

t_garray        *garray_makeObject                      (t_glist *glist,
                                                            t_symbol *s,
                                                            t_symbol *templateName,
                                                            t_float size,
                                                            t_float flags);
                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_paintAllScalarsByView            (t_glist *glist, int action);
void            canvas_paintAllScalarsByTemplate        (t_template *dummy, int action);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_writescalar                      (t_symbol *templatesym,
                                                            t_word *w,
                                                            t_buffer *b,
                                                            int amarrayelement);
                                                            
int             canvas_readscalar                       (t_glist *x,
                                                            int natoms,
                                                            t_atom *vec,
                                                            int *p_nextmsg,
                                                            int selectit);

void            canvas_serializeTemplates               (t_glist *glist, t_buffer *b);

void            canvas_read                             (t_glist *glist, t_symbol *name, t_symbol *format);
void            canvas_write                            (t_glist *glist, t_symbol *name, t_symbol *format);
void            canvas_merge                            (t_glist *glist, t_symbol *name, t_symbol *format);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outlet *voutlet_getit(t_pd *x);
t_inlet *vinlet_getit(t_pd *x);
int garray_getname(t_garray *x, t_symbol **namep);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_readfrombinbuf (t_glist *x, t_buffer *b, char *filename, int selectem);
void canvas_dataproperties (t_glist *x, t_scalar *sc, t_buffer *b);
void canvas_find_parent (t_glist *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_buffer *glist_writetobinbuf   (t_glist *x, int wholething);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void     canvas_vistext                     (t_glist *x, t_object *y);

void     canvas_zapallfortemplate           (t_glist *tmpl);
void     canvas_setusedastemplate           (t_glist *x);

int      canvas_getfont                     (t_glist *x);

void     canvas_fattenforscalars        (t_glist *x, int *x1, int *y1, int *x2, int *y2);
void     canvas_visforscalars           (t_glist *x, t_glist *glist, int vis);
int      canvas_clicksub                (t_glist *x,
                                            int xpix,
                                            int ypix,
                                            int shift,
                                            int alt,
                                            int dbl,
                                            int b);

t_glist  *canvas_getglistonsuper        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void         garray_properties       (t_garray *x);
t_template   *garray_template           (t_garray *x);




void         array_resize               (t_array *x, int n);
void         array_free                 (t_array *x);
void         array_redraw               (t_array *a, t_glist *glist);
void         array_resize_and_redraw    (t_array *array, t_glist *glist, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_gstub  *gstub_new             (t_glist *gl, t_array *a);
void     gstub_cutoff           (t_gstub *gs);
void     gpointer_setglist      (t_gpointer *gp, t_glist *glist, t_scalar *x);
void     gpointer_setarray      (t_gpointer *gp, t_array *array, t_word *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_scalar *scalar_new            (t_glist *owner, t_symbol *templatesym);
void     word_init              (t_word *wp, t_template *tmpl, t_gpointer *gp);
void     word_restore           (t_word *wp, t_template *tmpl, int argc, t_atom *argv);
void     word_free              (t_word *wp, t_template *tmpl);
void     scalar_getbasexy       (t_scalar *x, t_float *basex, t_float *basey);
void     scalar_redraw          (t_scalar *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void array_getcoordinate        (t_glist *glist,
                                    char *elem,
                                    int xonset,
                                    int yonset,
                                    int wonset,
                                    int indx,
                                    t_float basex,
                                    t_float basey,
                                    t_float xinc,
                                    t_fielddescriptor *xfielddesc,
                                    t_fielddescriptor *yfielddesc,
                                    t_fielddescriptor *wfielddesc,
                                    t_float *xp,
                                    t_float *yp,
                                    t_float *wp);

int array_getfields             (t_symbol *elemtemplatesym,
                                    t_glist **elemtemplatecanvasp,
                                    t_template **elemtemplatep,
                                    int *elemsizep,
                                    t_fielddescriptor *xfielddesc,
                                    t_fielddescriptor *yfielddesc,
                                    t_fielddescriptor *wfielddesc, 
                                    int *xonsetp,
                                    int *yonsetp, 
                                    int *wonsetp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_template   *template_new          (t_symbol *sym, int argc, t_atom *argv);
void         template_free          (t_template *x);
int          template_match         (t_template *x1, t_template *x2);
int          template_find_field    (t_template *x,
                                        t_symbol *name,
                                        int *p_onset,
                                        int *p_type,
                                        t_symbol **p_arraytype);

t_float      template_getfloat      (t_template *x, t_symbol *fieldname, t_word *wp, int loud);
void         template_setfloat      (t_template *x, t_symbol *fieldname, t_word *wp, t_float f, int loud);
t_symbol     *template_getsymbol    (t_template *x, t_symbol *fieldname, t_word *wp, int loud);
void         template_setsymbol     (t_template *x, t_symbol *fieldname, t_word *wp, t_symbol *s, int loud);
t_template   *gtemplate_get         (t_gtemplate *x);
t_template   *template_findbyname   (t_symbol *s);
t_glist      *template_findcanvas   (t_template *tmpl);
void         template_notify        (t_template *tmpl, t_symbol *s, int argc, t_atom *argv);
t_float      template_getfloat      (t_template *x, t_symbol *fieldname, t_word *wp, int loud);
void         template_setfloat      (t_template *x, t_symbol *fieldname, t_word *wp, t_float f, int loud);
t_symbol     *template_getsymbol    (t_template *x, t_symbol *fieldname, t_word *wp, int loud);
void         template_setsymbol     (t_template *x, t_symbol *fieldname, t_word *wp, t_symbol *s, int loud);

t_float      fielddesc_getcoord     (t_fielddescriptor *f, t_template *tmpl, t_word *wp, int loud);
void         fielddesc_setcoord     (t_fielddescriptor *f, t_template *tmpl, t_word *wp, t_float pix, int loud);
t_float      fielddesc_cvttocoord   (t_fielddescriptor *f, t_float val);
t_float      fielddesc_cvtfromcoord (t_fielddescriptor *f, t_float coord);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_canvas_h_
