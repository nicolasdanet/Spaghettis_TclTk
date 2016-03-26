
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

#define INLETS_WIDTH                7
#define INLETS_MIDDLE               ((INLETS_WIDTH - 1) / 2)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GLIST_DEFAULT_WIDTH         200
#define GLIST_DEFAULT_HEIGHT        140

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define TYPE_TEXT                   0
#define TYPE_OBJECT                 1
#define TYPE_MESSAGE                2
#define TYPE_ATOM                   3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DATA_FLOAT                  0
#define DATA_SYMBOL                 1
#define DATA_TEXT                   2
#define DATA_ARRAY                  3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define POINTER_NONE                0
#define POINTER_GLIST               1
#define POINTER_ARRAY               2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define ACTION_NONE                 0
#define ACTION_MOVE                 1
#define ACTION_CONNECT              2
#define ACTION_REGION               3
#define ACTION_PASS                 4
#define ACTION_DRAG                 5
#define ACTION_RESIZE               6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PLOT_POINTS                 0
#define PLOT_POLYGONS               1
#define PLOT_CURVES                 2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CURSOR_NOTHING              0
#define CURSOR_CLICKME              1
#define CURSOR_THICKEN              2
#define CURSOR_ADDPOINT             3
#define CURSOR_EDIT_NOTHING         4
#define CURSOR_EDIT_CONNECT         5
#define CURSOR_EDIT_DISCONNECT      6
#define CURSOR_EDIT_RESIZE          7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define RTEXT_DOWN                  1
#define RTEXT_DRAG                  2
#define RTEXT_DBL                   3
#define RTEXT_SHIFT                 4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define UNDO_FREE                   0
#define UNDO_UNDO                   1
#define UNDO_REDO                   2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _rtext;
struct _gtemplate;
struct _guiconnect;
struct _canvasenvironment;
struct _fielddesc;

#define t_rtext                     struct _rtext
#define t_gtemplate                 struct _gtemplate
#define t_guiconnect                struct _guiconnect
#define t_canvasenvironment         struct _canvasenvironment
#define t_fielddesc                 struct _fielddesc

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void (*t_glistkeyfn)(void *z, t_float key);
typedef void (*t_glistmotionfn)(void *z, t_float dx, t_float dy);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tick {
    t_float     k_point;
    t_float     k_inc;
    int         k_lperb;
    } t_tick;

typedef struct _selection {
    t_gobj              *sel_what;
    struct _selection   *sel_next;
    } t_selection;

typedef struct _editor {
    t_selection         *e_updlist;
    t_rtext             *e_rtext;
    t_selection         *e_selection;
    t_rtext             *e_textedfor;
    t_gobj              *e_grab;
    t_glistmotionfn     e_motionfn;
    t_glistkeyfn        e_keyfn;
    t_buffer            *e_connectbuf;
    t_buffer            *e_deleted;
    t_guiconnect        *e_guiconnect;
    t_glist             *e_glist;
    int                 e_xwas;
    int                 e_ywas;
    int                 e_selectline_index1;
    int                 e_selectline_outno;
    int                 e_selectline_index2;
    int                 e_selectline_inno;
    t_outconnect        *e_selectline_tag;
    char                e_onmotion;
    char                e_lastmoved;
    char                e_textdirty;
    char                e_selectedline;
    t_clock             *e_clock;
    int                 e_xnew;
    int                 e_ynew;
    } t_editor;

struct _glist {  
    t_object            gl_obj;
    t_gobj              *gl_list;
    t_gstub             *gl_stub;
    int                 gl_valid;
    t_glist             *gl_owner;
    int                 gl_pixwidth;
    int                 gl_pixheight;
    t_float             gl_x1;
    t_float             gl_y1;
    t_float             gl_x2;
    t_float             gl_y2;
    int                 gl_screenx1;
    int                 gl_screeny1;
    int                 gl_screenx2;
    int                 gl_screeny2;
    int                 gl_xmargin;
    int                 gl_ymargin;
    t_tick              gl_xtick;
    int                 gl_nxlabels;
    t_symbol            **gl_xlabel;
    t_float             gl_xlabely;
    t_tick              gl_ytick;
    int                 gl_nylabels;
    t_symbol            **gl_ylabel;
    t_float             gl_ylabelx;
    t_editor            *gl_editor;
    t_symbol            *gl_name;
    int                 gl_font;
    t_glist             *gl_next;
    t_canvasenvironment *gl_env;
    char                gl_havewindow;
    char                gl_mapped;
    char                gl_dirty;
    char                gl_loading;
    char                gl_willvis;
    char                gl_edit;
    char                gl_isdeleting;
    char                gl_goprect;
    char                gl_isgraph;
    char                gl_hidetext;
    char                gl_private;
    };

typedef struct _dataslot {
    int         ds_type;
    t_symbol    *ds_name;
    t_symbol    *ds_arraytemplate;
    } t_dataslot;

typedef struct _template {
    t_pd        t_pdobj;   
    t_gtemplate *t_list;  
    t_symbol    *t_sym;    
    int         t_n;    
    t_dataslot  *t_vec;  
    } t_template;

struct _array {
    int         a_n;
    int         a_elemsize;
    char        *a_vec;
    t_symbol    *a_templatesym;
    int         a_valid;
    t_gpointer  a_gp;
    t_gstub     *a_stub;
    };

typedef struct _linetraverser {
    t_glist        *tr_x;
    t_object        *tr_ob;
    int             tr_nout;
    int             tr_outno;
    t_object        *tr_ob2;
    t_outlet        *tr_outlet;
    t_inlet         *tr_inlet;
    int             tr_nin;
    int             tr_inno;
    int             tr_x11, tr_y11, tr_x12, tr_y12;
    int             tr_x21, tr_y21, tr_x22, tr_y22;
    int             tr_lx1, tr_ly1, tr_lx2, tr_ly2;
    t_outconnect    *tr_nextoc;
    int             tr_nextoutno;
    } t_linetraverser;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_getrectfn)(t_gobj *x, t_glist *glist, int *x1, int *y1, int *x2, int *y2);
typedef void (*t_displacefn)(t_gobj *x, t_glist *glist, int dx, int dy);
typedef void (*t_selectfn)(t_gobj *x, t_glist *glist, int state);
typedef void (*t_activatefn)(t_gobj *x, t_glist *glist, int state);
typedef void (*t_deletefn)(t_gobj *x, t_glist *glist);
typedef void (*t_visfn)(t_gobj *x, t_glist *glist, int flag);
typedef int  (*t_clickfn)(t_gobj *x, t_glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _widgetbehavior {
    t_getrectfn     w_getrectfn;
    t_displacefn    w_displacefn;
    t_selectfn      w_selectfn;
    t_activatefn    w_activatefn;
    t_deletefn      w_deletefn;
    t_visfn         w_visfn;
    t_clickfn       w_clickfn;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_parentgetrectfn)   (t_gobj *x, t_glist *glist, t_word *data, t_template *tmpl,
                                        t_float basex,
                                        t_float basey,
                                        int *x1,
                                        int *y1,
                                        int *x2,
                                        int *y2);
typedef void (*t_parentdisplacefn)  (t_gobj *x, t_glist *glist, t_word *data, t_template *tmpl,
                                        t_float basex,
                                        t_float basey,
                                        int dx, 
                                        int dy);
typedef void (*t_parentselectfn)    (t_gobj *x, t_glist *glist, t_word *data, t_template *tmpl,
                                        t_float basex,
                                        t_float basey,
                                        int state);
typedef void (*t_parentactivatefn)  (t_gobj *x, t_glist *glist, t_word *data, t_template *tmpl,
                                        t_float basex,
                                        t_float basey,
                                        int state);
typedef void (*t_parentvisfn)       (t_gobj *x, t_glist *glist, t_word *data, t_template *tmpl,
                                        t_float basex,
                                        t_float basey,
                                        int flag);
typedef int  (*t_parentclickfn)     (t_gobj *x, t_glist *glist, t_word *data, t_template *tmpl,
                                        t_scalar *sc,
                                        t_array *ap,
                                        t_float basex,
                                        t_float basey,
                                        int xpix,
                                        int ypix,
                                        int shift,
                                        int alt,
                                        int dbl,
                                        int b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _parentwidgetbehavior {
    t_parentgetrectfn   w_parentgetrectfn;
    t_parentdisplacefn  w_parentdisplacefn;
    t_parentselectfn    w_parentselectfn;
    t_parentactivatefn  w_parentactivatefn;
    t_parentvisfn       w_parentvisfn;
    t_parentclickfn     w_parentclickfn;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma 

#define canvas_asObjectIfBox(x)     (pd_class (x)->c_isBox ? (t_object *)(x) : NULL)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setcursor   (t_glist *x, unsigned int cursornum);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gobj_getrect       (t_gobj *x, t_glist *owner, int *x1, int *y1, int *x2, int *y2);
void gobj_displace      (t_gobj *x, t_glist *owner, int dx, int dy);
void gobj_select        (t_gobj *x, t_glist *owner, int state);
void gobj_activate      (t_gobj *x, t_glist *owner, int state);
void gobj_delete        (t_gobj *x, t_glist *owner);
void gobj_properties    (t_gobj *x, t_glist *gl);
int  gobj_shouldvis     (t_gobj *x, t_glist *gl);
void gobj_vis           (t_gobj *x, t_glist *gl, int flag);
int  gobj_click         (t_gobj *x, t_glist *gl, int xpix, int ypix, int shift, int alt, int dbl, int b);
void gobj_save          (t_gobj *x, t_buffer *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist  *glist_new             (void);
void     glist_init             (t_glist *x);
void     glist_add              (t_glist *x, t_gobj *g);
void     glist_clear            (t_glist *x);
t_glist *glist_getcanvas        (t_glist *x);
int      glist_isselected       (t_glist *x, t_gobj *y);
void     glist_select           (t_glist *x, t_gobj *y);
void     glist_deselect         (t_glist *x, t_gobj *y);
void     glist_noselect         (t_glist *x);
void     glist_selectall        (t_glist *x);
void     glist_delete           (t_glist *x, t_gobj *y);
void     glist_retext           (t_glist *x, t_object *y);
int      glist_isvisible        (t_glist *x);
int      glist_istoplevel       (t_glist *x);
t_glist *glist_findgraph        (t_glist *x);
int      glist_getfont          (t_glist *x);
void     glist_sort             (t_glist *canvas);
void     glist_read             (t_glist *x, t_symbol *filename, t_symbol *format);
void     glist_mergefile        (t_glist *x, t_symbol *filename, t_symbol *format);
void     glist_grab             (t_glist *x,
                                    t_gobj *y,
                                    t_glistmotionfn motionfn,
                                    t_glistkeyfn keyfn,
                                    int xpos,
                                    int ypos);

t_float  glist_pixelstox        (t_glist *x, t_float xpix);
t_float  glist_pixelstoy        (t_glist *x, t_float ypix);
t_float  glist_xtopixels        (t_glist *x, t_float xval);
t_float  glist_ytopixels        (t_glist *x, t_float yval);
t_float  glist_dpixtodx         (t_glist *x, t_float dxpix);
t_float  glist_dpixtody         (t_glist *x, t_float dypix);
void     glist_getnextxy        (t_glist *x, int *xval, int *yval);
void     glist_glist            (t_glist *x, t_symbol *s, int argc, t_atom *argv);
t_glist *glist_addglist         (t_glist *x,
                                    t_symbol *sym,
                                    t_float x1,
                                    t_float y1,
                                    t_float x2,
                                    t_float y2,
                                    t_float px1,
                                    t_float py1,
                                    t_float px2,
                                    t_float py2);

void     glist_arraydialog      (t_glist *parent,
                                    t_symbol *name,
                                    t_float size,
                                    t_float saveit);

t_buffer *glist_writetobinbuf   (t_glist *x, int wholething);
int      glist_isgraph          (t_glist *x);
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

void canvas_popabstraction      (t_glist *x);
int  canvas_getdollarzero       (void);
void canvas_create_editor       (t_glist *x);
void canvas_destroy_editor      (t_glist *x);
void canvas_deletelinesforio    (t_glist *x, t_object *text, t_inlet *inp, t_outlet *outp);

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

t_rtext  *rtext_new         (t_glist *glist, t_object *who);
void     rtext_draw         (t_rtext *x);
void     rtext_erase        (t_rtext *x);
t_rtext  *rtext_remove      (t_rtext *first, t_rtext *x);
int      rtext_height       (t_rtext *x);
void     rtext_displace     (t_rtext *x, int dx, int dy);
void     rtext_select       (t_rtext *x, int state);
void     rtext_activate     (t_rtext *x, int state);
void     rtext_free         (t_rtext *x);
void     rtext_key          (t_rtext *x, int n, t_symbol *s);
void     rtext_mouse        (t_rtext *x, int xval, int yval, int flag);
void     rtext_retext       (t_rtext *x);
int      rtext_width        (t_rtext *x);
int      rtext_height       (t_rtext *x);
char     *rtext_gettag      (t_rtext *x);
void     rtext_gettext      (t_rtext *x, char **buf, int *bufsize);
void     rtext_getseltext   (t_rtext *x, char **buf, int *bufsize);
t_rtext  *glist_findrtext   (t_glist *gl, t_object *who);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_undofn)(t_glist *canvas, void *buf, int action);
typedef int (*t_canvasapply)(t_glist *x, t_int x1, t_int x2, t_int x3);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_new                        (void *dummy, t_symbol *sel, int argc, t_atom *argv);
t_symbol *canvas_makebindsym                (t_symbol *s);
void     canvas_vistext                     (t_glist *x, t_object *y);
void     canvas_fixlines                    (t_glist *x, t_object *text);
void     canvas_deletelines                 (t_glist *x, t_object *text);
void     canvas_stowconnections             (t_glist *x);
void     canvas_restoreconnections          (t_glist *x);
void     canvas_redraw                      (t_glist *x);
t_inlet  *canvas_addinlet                   (t_glist *x, t_pd *who, t_symbol *sym);
void     canvas_rminlet                     (t_glist *x, t_inlet *ip);
t_outlet *canvas_addoutlet                  (t_glist *x, t_pd *who, t_symbol *sym);
void     canvas_rmoutlet                    (t_glist *x, t_outlet *op);
void     canvas_redrawallfortemplate        (t_template *tmpl, int action);
void     canvas_redrawallfortemplatecanvas  (t_glist *x, int action);
void     canvas_zapallfortemplate           (t_glist *tmpl);
void     canvas_setusedastemplate           (t_glist *x);
t_glist *canvas_getcurrent                 (void);
void     canvas_setcurrent                  (t_glist *x);
void     canvas_unsetcurrent                (t_glist *x);
t_symbol *canvas_realizedollar              (t_glist *x, t_symbol *s);
t_glist *canvas_getroot                    (t_glist *x);
void     canvas_dirty                       (t_glist *x, t_float n);
int      canvas_getfont                     (t_glist *x);
t_int    *canvas_recurapply                 (t_glist *x,
                                                t_canvasapply *fn,
                                                t_int x1,
                                                t_int x2,
                                                t_int x3);

void     canvas_resortinlets            (t_glist *x);
void     canvas_resortoutlets           (t_glist *x);
void     canvas_free                    (t_glist *x);
void     canvas_updatewindowlist        (void);
void     canvas_editmode                (t_glist *x, t_float state);
int      canvas_isabstraction           (t_glist *x);
int      canvas_istable                 (t_glist *x);
int      canvas_showtext                (t_glist *x);
void     canvas_vis                     (t_glist *x, t_float f);

t_canvasenvironment *canvas_getenv      (t_glist *x);

void     canvas_rename                  (t_glist *x, t_symbol *s, t_symbol *dir);
void     canvas_loadbang                (t_glist *x);
int      canvas_hitbox                  (t_glist *x,
                                            t_gobj *y,
                                            int xpos,
                                            int ypos,
                                            int *x1p,
                                            int *y1p,
                                            int *x2p,
                                            int *y2p);

int      canvas_setdeleting             (t_glist *x, int flag);
void     canvas_setundo                 (t_glist *x,
                                            t_undofn undofn,
                                            void *buf,
                                            const char *name);

void     canvas_noundo                  (t_glist *x);
int      canvas_getindex                (t_glist *x, t_gobj *y);
void     canvas_connect                 (t_glist *x,
                                            t_float fwhoout,
                                            t_float foutno,
                                            t_float fwhoin,
                                            t_float finno);

void     canvas_disconnect              (t_glist *x,
                                            t_float index1,
                                            t_float outno,
                                            t_float index2,
                                            t_float inno);

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

t_glist  *canvas_getglistonsuper        (void);
t_glist  *pd_checkglist                 (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void         linetraverser_start        (t_linetraverser *t, t_glist *x);
t_outconnect *linetraverser_next        (t_linetraverser *t);
void         linetraverser_skipobject   (t_linetraverser *t);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_guiconnect *guiconnect_new            (t_pd *who, t_symbol *sym);
void         guiconnect_notarget        (t_guiconnect *x, double timedelay);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -


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
                                    t_fielddesc *xfielddesc,
                                    t_fielddesc *yfielddesc,
                                    t_fielddesc *wfielddesc,
                                    t_float *xp,
                                    t_float *yp,
                                    t_float *wp);

int array_getfields             (t_symbol *elemtemplatesym,
                                    t_glist **elemtemplatecanvasp,
                                    t_template **elemtemplatep,
                                    int *elemsizep,
                                    t_fielddesc *xfielddesc,
                                    t_fielddesc *yfielddesc,
                                    t_fielddesc *wfielddesc, 
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

t_float      fielddesc_getcoord     (t_fielddesc *f, t_template *tmpl, t_word *wp, int loud);
void         fielddesc_setcoord     (t_fielddesc *f, t_template *tmpl, t_word *wp, t_float pix, int loud);
t_float      fielddesc_cvttocoord   (t_fielddesc *f, t_float val);
t_float      fielddesc_cvtfromcoord (t_fielddesc *f, t_float coord);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_canvas_h_
