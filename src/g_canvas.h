
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

#define TYPE_TEXT                       0
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

#define CURSOR_EDIT_NOTHING             4
#define CURSOR_EDIT_CONNECT             5
#define CURSOR_EDIT_DISCONNECT          6
#define CURSOR_EDIT_RESIZE              7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BOX_TEXT_DOWN                   1
#define BOX_TEXT_DRAG                   2
#define BOX_TEXT_DOUBLE                 3
#define BOX_TEXT_SHIFT                  4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define UNDO_FREE                       0
#define UNDO_UNDO                       1
#define UNDO_REDO                       2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCALAR_REDRAW                   0
#define SCALAR_DRAW                     1
#define SCALAR_ERASE                    2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INLETS_WIDTH                    7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define INLETS_OFFSET(width, i, n)      ((((width) - INLETS_WIDTH) * (i)) / (((n) == 1) ? 1 : ((n) - 1)))
#define INLETS_MIDDLE(width, i, n)      INLETS_OFFSET (width, i, n) + ((INLETS_WIDTH - 1) / 2)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GLIST_DEFAULT_X                         100
#define GLIST_DEFAULT_Y                         20
#define GLIST_DEFAULT_WIDTH                     200
#define GLIST_DEFAULT_HEIGHT                    140

#define GLIST_DEFAULT_START                     0
#define GLIST_DEFAULT_END                       100
#define GLIST_DEFAULT_UP                        1.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CANVAS_OBJECT_DEFAULT_X                 40
#define CANVAS_OBJECT_DEFAULT_Y                 40

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef __APPLE__
    #define CANVAS_WINDOW_HEADER_HEIGHT         22
#else
    #define CANVAS_WINDOW_HEADER_HEIGHT         50
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define CANVAS_WINDOW_DEFAULT_WIDTH             450
#define CANVAS_WINDOW_DEFAULT_HEIGHT            300

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _gtemplate;
struct _guiconnect;
struct _environment;
struct _fielddescriptor;
struct _boxtext;

#define t_gtemplate                     struct _gtemplate
#define t_guiconnect                    struct _guiconnect
#define t_environment                   struct _environment
#define t_fielddescriptor               struct _fielddescriptor
#define t_boxtext                       struct _boxtext

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
    
typedef struct _tick {
    t_float             k_point;
    t_float             k_inc;
    int                 k_lperb;
    } t_tick;

typedef struct _selection {
    t_gobj              *sel_what;
    struct _selection   *sel_next;
    } t_selection;
    
typedef struct _editor {
    t_guiconnect        *e_guiconnect;
    t_boxtext           *e_text;
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
    t_keyfn             e_fnKey;
    int                 e_previousX;
    int                 e_previousY;
    int                 e_newX;
    int                 e_newY;
    int                 e_onMotion;
    int                 e_isTextDirty;
    int                 e_isSelectedline;
    } t_editor;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _glist {  
    t_object            gl_obj;         /* MUST be the first. */
    t_gobj              *gl_graphics;
    t_gstub             *gl_stub;
    t_glist             *gl_owner;
    t_glist             *gl_next;
    t_environment       *gl_environment;
    t_symbol            *gl_name;
    t_editor            *gl_editor;
    int                 gl_magic;
    int                 gl_width;
    int                 gl_height;
    t_float             gl_indexStart;
    t_float             gl_indexEnd;
    t_float             gl_valueUp;
    t_float             gl_valueDown;
    int                 gl_windowTopLeftX;
    int                 gl_windowTopLeftY;
    int                 gl_windowBottomRightX;
    int                 gl_windowBottomRightY;
    int                 gl_marginX;
    int                 gl_marginY;
    int                 gl_fontSize;
    t_tick              gl_tickX;
    t_tick              gl_tickY;
    char                gl_isMapped;
    char                gl_isDirty;
    char                gl_isLoading;
    char                gl_isEditMode;
    char                gl_isDeleting;
    char                gl_isGraphOnParent;
    char                gl_hasRectangle;
    char                gl_hideText;
    char                gl_saveScalar;
    char                gl_haveWindow;
    char                gl_willBeVisible;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _dataslot {
    int                 ds_type;
    t_symbol            *ds_name;
    t_symbol            *ds_arraytemplate;
    } t_dataslot;

struct _array {
    int                 a_n;
    int                 a_elemsize;
    char                *a_vec;
    t_symbol            *a_templatesym;
    int                 a_valid;
    t_gpointer          a_gp;
    t_gstub             *a_stub;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _template {
    t_pd                t_pdobj;        /* MUST be the first. */
    t_gtemplate         *t_list;  
    t_symbol            *t_sym;    
    int                 t_n;    
    t_dataslot          *t_vec;  
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define canvas_castToObjectIfBox(x)     (pd_class (x)->c_isBox ? (t_object *)(x) : NULL)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist         *canvas_castToGlist                     (t_pd *x);

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
t_environment   *canvas_getEnvironment                  (t_glist *glist);
t_glist         *canvas_getRoot                         (t_glist *glist);
t_glist         *canvas_findDirty                       (t_glist *glist);
t_symbol        *canvas_expandDollar                    (t_glist *glist, t_symbol *s);
t_symbol        *canvas_makeBindSymbol                  (t_symbol *s);
t_glist         *canvas_addGraph                        (t_glist *glist,
                                                            t_symbol *name,
                                                            t_float indexStart,
                                                            t_float valueUp,
                                                            t_float indexEnd,
                                                            t_float valueDown,
                                                            t_float topLeftX,
                                                            t_float topLeftY,
                                                            t_float bottomRightX,
                                                            t_float bottomRightY);

int             canvas_isVisible                        (t_glist *glist);
int             canvas_isTopLevel                       (t_glist *glist);
int             canvas_isAbstraction                    (t_glist *glist);

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

void            canvas_setLastCoordinates               (t_glist *glist, int a, int b);
void            canvas_getLastCoordinates               (t_glist *glist, int *a, int *b);

int             canvas_getIndexOfObject                 (t_glist *glist, t_gobj *object);
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

void            canvas_drawGraphOnParentRectangle       (t_glist *glist);
void            canvas_deleteGraphOnParentRectangle     (t_glist *glist);
void            canvas_redrawAllByTemplate              (t_template *dummy, int action);
void            canvas_redrawAllByTemplateByCanvas      (t_glist *glist, int action);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_removeSelectedLine               (t_glist *x);
int             canvas_isObjectSelected                 (t_glist *glist, t_gobj *y);
void            canvas_selectObject                     (t_glist *glist, t_gobj *y);
void            canvas_selectLine                       (t_glist *glist, 
                                                            t_outconnect *connection,
                                                            int indexOfObjectOut,
                                                            int indexOfOutlet,
                                                            int indexOfObjectIn,
                                                            int indexOfInlet);
                                                            
void            canvas_deselectObject                   (t_glist *glist, t_gobj *y);
void            canvas_deselectAll                      (t_glist *glist);
int             canvas_getNumberOfUnselectedObjects     (t_glist *glist);
int             canvas_getIndexOfObjectAmongSelected    (t_glist *glist, t_gobj *y);
int             canvas_getIndexOfObjectAmongUnselected  (t_glist *glist, t_gobj *y);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void            canvas_createEditor                     (t_glist *glist);
void            canvas_destroyEditor                    (t_glist *glist);

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

void            canvas_setBounds                        (t_glist *glist,
                                                            t_float a,
                                                            t_float b,
                                                            t_float c,
                                                            t_float d);

void            canvas_motion                           (t_glist *glist,
                                                            t_float deltaX,
                                                            t_float deltaY,
                                                            t_float modifier);

void            canvas_mouse                            (t_glist *glist,
                                                            t_float a,
                                                            t_float b,
                                                            t_float button,
                                                            t_float modifier);

void            canvas_mouseup                          (t_glist *glist,
                                                            t_float a,
                                                            t_float b,
                                                            t_float button);
                                                            
void            canvas_done_popup                       (t_glist *glist, t_float item, t_float a, t_float b);
void            canvas_donecanvasdialog                 (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_key                              (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_restore                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_loadbang                         (t_glist *glist);
void            canvas_map                              (t_glist *glist, t_float f);
void            canvas_dirty                            (t_glist *glist, t_float f);
void            canvas_pop                              (t_glist *glist, t_float f);
void            canvas_close                            (t_glist *glist, t_float f);
void            canvas_editmode                         (t_glist *glist, t_float f);
void            canvas_cut                              (t_glist *glist);
void            canvas_copy                             (t_glist *glist);
void            canvas_paste                            (t_glist *glist);
void            canvas_duplicate                        (t_glist *glist);
void            canvas_selectall                        (t_glist *glist);

void            canvas_connect                          (t_glist *x,
                                                            t_float indexOfObjectOut,
                                                            t_float indexOfOutlet,
                                                            t_float indexOfObjectIn,
                                                            t_float indexOfInlet);

void            canvas_disconnect                       (t_glist *x,
                                                            t_float indexOfObjectOut,
                                                            t_float indexOfOutlet,
                                                            t_float indexOfObjectIn,
                                                            t_float indexOfInlet);
                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_dsp                              (t_glist *x, t_signal **sp);

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

void            gobj_getRectangle                       (t_gobj *x, t_glist *owner,
                                                            int *a,
                                                            int *b,
                                                            int *c,
                                                            int *d);
                                                            
void            gobj_displace                           (t_gobj *x, t_glist *owner, int deltaX, int deltaY);
void            gobj_select                             (t_gobj *x, t_glist *owner, int state);
void            gobj_activate                           (t_gobj *x, t_glist *owner, int state);
void            gobj_delete                             (t_gobj *x, t_glist *owner);
int             gobj_shouldBeVisible                    (t_gobj *x, t_glist *owner);
void            gobj_visibleHasChanged                  (t_gobj *x, t_glist *owner, int isVisible);
int             gobj_click                              (t_gobj *x,
                                                            t_glist *owner,
                                                            int a,
                                                            int b,
                                                            int shift,
                                                            int alt,
                                                            int dbl,
                                                            int k);
                                                        
void            gobj_save                               (t_gobj *x, t_buffer *buffer);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_readfrombinbuf (t_glist *x, t_buffer *b, char *filename, int selectem);

void canvas_dataproperties (t_glist *x, t_scalar *sc, t_buffer *b);

void canvas_properties (t_gobj *z, t_glist *canvas);
void canvas_objfor (t_glist *gl, t_object *x, int argc, t_atom *argv);
void canvas_menusave (t_glist *x, float fdestroy);
void canvas_menusaveas (t_glist *x, float fdestroy);
void canvas_find_parent (t_glist *x);
void canvas_setgraph (t_glist *x, int flag, int nogoprect);

void glist_text(t_glist *x, t_symbol *s, int argc, t_atom *argv);
void canvas_obj(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_bng(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_toggle(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_vslider(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_hslider(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_hradio(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_vradio(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_vumeter(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_mycnv(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_numbox(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_msg(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_floatatom(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void canvas_symbolatom(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
void glist_scalar(t_glist *canvas, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist  *glist_new             (void);
void     glist_add              (t_glist *x, t_gobj *g);
void     glist_clear            (t_glist *x);
t_glist *glist_getcanvas        (t_glist *x);
void     glist_delete           (t_glist *x, t_gobj *y);
void     glist_retext           (t_glist *x, t_object *y);
t_glist *glist_findgraph        (t_glist *x);
void     glist_sort             (t_glist *canvas);
void     glist_read             (t_glist *x, t_symbol *filename, t_symbol *format);
void     glist_mergefile        (t_glist *x, t_symbol *filename, t_symbol *format);
void     glist_grab             (t_glist *x,
                                    t_gobj *y,
                                    t_motionfn motionfn,
                                    t_keyfn keyfn,
                                    int xpos,
                                    int ypos);

t_float  glist_pixelstox        (t_glist *x, t_float xpix);
t_float  glist_pixelstoy        (t_glist *x, t_float ypix);
t_float  glist_xtopixels        (t_glist *x, t_float xval);
t_float  glist_ytopixels        (t_glist *x, t_float yval);
t_float  glist_dpixtodx         (t_glist *x, t_float dxpix);
t_float  glist_dpixtody         (t_glist *x, t_float dypix);

void     glist_arraydialog      (t_glist *parent,
                                    t_symbol *name,
                                    t_float size,
                                    t_float saveit);

t_buffer *glist_writetobinbuf   (t_glist *x, int wholething);
void     glist_redraw           (t_glist *x);
void     glist_drawio           (t_glist *x,
                                    t_object *ob,
                                    int firsttime,
                                    char *tag,
                                    int x1,
                                    int y1,
                                    int x2,
                                    int y2);

void     glist_eraseio          (t_glist *glist, t_object *ob, char *tag);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void text_save          (t_gobj *z, t_buffer *b);
void text_setto         (t_object *x, t_glist *glist, char *buf, int bufsize);
void text_drawborder    (t_object *x, t_glist *glist, char *tag, int width, int height, int firsttime);
void text_eraseborder   (t_object *x, t_glist *glist, char *tag);
int  text_xcoord        (t_object *x, t_glist *glist);
int  text_ycoord        (t_object *x, t_glist *glist);
int  text_xpix          (t_object *x, t_glist *glist);
int  text_ypix          (t_object *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_boxtext   *rtext_new          (t_glist *glist, t_object *who);
void        rtext_draw          (t_boxtext *x);
void        rtext_erase         (t_boxtext *x);
t_boxtext   *rtext_remove       (t_boxtext *first, t_boxtext *x);
int         rtext_height        (t_boxtext *x);
void        rtext_displace      (t_boxtext *x, int dx, int dy);
void        rtext_select        (t_boxtext *x, int state);
void        rtext_activate      (t_boxtext *x, int state);
void        rtext_free          (t_boxtext *x);
void        rtext_key           (t_boxtext *x, int n, t_symbol *s);
void        rtext_mouse         (t_boxtext *x, int xval, int yval, int flag);
void        rtext_retext        (t_boxtext *x);
int         rtext_width         (t_boxtext *x);
int         rtext_height        (t_boxtext *x);
char        *rtext_gettag       (t_boxtext *x);
void        rtext_gettext       (t_boxtext *x, char **buf, int *bufsize);
void        rtext_getseltext    (t_boxtext *x, char **buf, int *bufsize);
t_boxtext   *glist_findrtext    (t_glist *gl, t_object *who);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void     canvas_vistext                     (t_glist *x, t_object *y);
void     canvas_stowconnections             (t_glist *x);
void     canvas_restoreconnections          (t_glist *x);
t_inlet  *canvas_addinlet                   (t_glist *x, t_pd *who, t_symbol *sym);
void     canvas_rminlet                     (t_glist *x, t_inlet *ip);
t_outlet *canvas_addoutlet                  (t_glist *x, t_pd *who, t_symbol *sym);
void     canvas_rmoutlet                    (t_glist *x, t_outlet *op);
void     canvas_zapallfortemplate           (t_glist *tmpl);
void     canvas_setusedastemplate           (t_glist *x);

int      canvas_getfont                     (t_glist *x);
void     canvas_resortinlets            (t_glist *x);
void     canvas_resortoutlets           (t_glist *x);

int      canvas_istable                 (t_glist *x);
void     canvas_vis                     (t_glist *x, t_float f);

int      canvas_hitbox                  (t_glist *x,
                                            t_gobj *y,
                                            int xpos,
                                            int ypos,
                                            int *x1p,
                                            int *y1p,
                                            int *x2p,
                                            int *y2p);

int      canvas_setdeleting             (t_glist *x, int flag);

int      canvas_isconnected             (t_glist *x,
                                            t_object *ob1,
                                            int n1,
                                            t_object *ob2,
                                            int n2);

void     canvas_selectinrect            (t_glist *x, int lox, int loy, int hix, int hiy);
void     canvas_fattenforscalars        (t_glist *x, int *x1, int *y1, int *x2, int *y2);
void     canvas_visforscalars           (t_glist *x, t_glist *glist, int vis);
int      canvas_clicksub                (t_glist *x,
                                            int xpix,
                                            int ypix,
                                            int shift,
                                            int alt,
                                            int dbl,
                                            int b);
void    canvas_setcursor                (t_glist *x, unsigned int cursornum);

t_glist  *canvas_getglistonsuper        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void         garray_properties       (t_garray *x);
t_template   *garray_template           (t_garray *x);
t_garray     *graph_array               (t_glist *gl,
                                            t_symbol *s,
                                            t_symbol *tmpl,
                                            t_float f,
                                            t_float saveit);

void        garray_init                 (void);

t_array      *array_new                 (t_symbol *templatesym, t_gpointer *parent);
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
void     canvas_writescalar     (t_symbol *templatesym, t_word *w, t_buffer *b, int amarrayelement);
int      canvas_readscalar      (t_glist *x, int natoms, t_atom *vec, int *p_nextmsg, int selectit);

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
