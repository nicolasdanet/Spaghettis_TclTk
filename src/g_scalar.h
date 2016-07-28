
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_scalar_h_
#define __g_scalar_h_

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

#define PLOT_POLYGONS                   0
#define PLOT_POINTS                     1
#define PLOT_CURVES                     2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _gpointer {
    union {   
        struct _scalar  *gp_scalar;
        union word      *gp_w;
    } gp_un;
    t_gmaster           *gp_master;
    t_unique            gp_uniqueIdentifier;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _array {
    int                 a_size;
    int                 a_stride;
    char                *a_vector;
    t_symbol            *a_templateIdentifier;
    t_gmaster           *a_master;
    t_unique            a_uniqueIdentifier;
    t_gpointer          a_parent;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _fielddescriptor {
    char                fd_type;
    char                fd_isVariable;
    union {
        t_float         fd_float;
        t_symbol        *fd_variableName;
    } fd_un;
    t_float             fd_v1;
    t_float             fd_v2;
    t_float             fd_screen1;
    t_float             fd_screen2;
    t_float             fd_quantum;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _dataslot {
    int                 ds_type;
    t_symbol            *ds_fieldName;
    t_symbol            *ds_templateIdentifier;
    } t_dataslot;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _template {
    t_pd                tp_pd;                      /* MUST be the first. */
    int                 tp_size;    
    t_dataslot          *tp_vector;   
    t_symbol            *tp_templateIdentifier; 
    t_struct            *tp_instance;               /* For now, only one instance is allowed. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define ARRAY_WORD      sizeof (t_word)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GPOINTER_INIT   { NULL, NULL, 0UL }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            word_init                               (t_word *w, t_template *tmpl, t_gpointer *gp);
void            word_restore                            (t_word *w, t_template *tmpl, int argc, t_atom *argv);
void            word_free                               (t_word *w, t_template *tmpl);

t_array         *word_getArray                          (t_word *w, t_template *tmpl, t_symbol *fieldName);
t_symbol        *word_getSymbol                         (t_word *w, t_template *tmpl, t_symbol *fieldName);

t_float         word_getFloat                           (t_word *w, t_template *tmpl, t_symbol *fieldName);
void            word_setFloat                           (t_word *w, 
                                                            t_template *tmpl,
                                                            t_symbol *fieldName,
                                                            t_float f);

void            word_setSymbol                          (t_word *w,
                                                            t_template *tmpl,
                                                            t_symbol *fieldName,
                                                            t_symbol *s);

t_float         word_getFloatByField                    (t_word *w, t_template *tmpl, t_fielddescriptor *fd);
t_float         word_getFloatByFieldAsPosition          (t_word *w, t_template *tmpl, t_fielddescriptor *fd);
void            word_setFloatByFieldAsPosition          (t_word *w,
                                                            t_template *tmpl,
                                                            t_fielddescriptor *fd,
                                                            t_float position);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_scalar        *scalar_new                             (t_glist *owner, t_symbol *templateIdentifier);
t_word          *scalar_getData                         (t_scalar *x);
t_symbol        *scalar_getTemplateIdentifier           (t_scalar *x);
t_array         *scalar_getArray                        (t_scalar *x, t_symbol *fieldName);

t_float         scalar_getFloat                         (t_scalar *x, t_symbol *fieldName);
void            scalar_setFloat                         (t_scalar *x, t_symbol *fieldName, t_float f);

void            scalar_redraw                           (t_scalar *x, t_glist *glist);
void            scalar_redrawByPointer                  (t_gpointer *gp);
void            scalar_setVisibilityByPointer           (t_gpointer *gp, int isVisible);

int             scalar_performClick                     (t_word *w,
                                                            t_template *tmpl,
                                                            t_scalar *scalar,
                                                            t_array *array,
                                                            t_glist *glist,
                                                            t_float offsetX,
                                                            t_float offestY,
                                                            int a,
                                                            int b,
                                                            int shift,
                                                            int alt,
                                                            int dbl,
                                                            int clicked);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_array         *array_new                              (t_symbol *templateIdentifier, t_gpointer *parent);
t_gpointer      *array_getTopParentArray                (t_gpointer *gp);
t_symbol        *array_getTemplateIdentifier            (t_array *x);
t_word          *array_getData                          (t_array *x);

int             array_getSize                           (t_array *x);
int             array_getElementSize                    (t_array *x);

void            array_free                              (t_array *x);
void            array_resize                            (t_array *x, int n);
void            array_redraw                            (t_array *x, t_glist *glist);
void            array_resizeAndRedraw                   (t_array *x, t_glist *glist, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_gmaster       *gpointer_masterCreateWithGlist         (t_glist *glist);
t_gmaster       *gpointer_masterCreateWithArray         (t_array *array);

void            gpointer_masterRelease                  (t_gmaster *master);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void            gpointer_init                           (t_gpointer *gp);
void            gpointer_setAsScalar                    (t_gpointer *gp, t_glist *owner, t_scalar *scalar);
void            gpointer_setAsWord                      (t_gpointer *gp, t_array *owner, t_word *w);
void            gpointer_setByCopy                      (t_gpointer *gp, t_gpointer *toSet);
void            gpointer_unset                          (t_gpointer *gp);
t_unique        gpointer_getUniqueIdentifier            (t_gpointer *gp);
int             gpointer_isSet                          (t_gpointer *gp);
int             gpointer_isNull                         (t_gpointer *gp);
int             gpointer_isValid                        (t_gpointer *gp);
int             gpointer_isValidNullAllowed             (t_gpointer *gp);
int             gpointer_isScalar                       (t_gpointer *gp);
int             gpointer_isWord                         (t_gpointer *gp);

void            gpointer_retain                         (t_gpointer *gp);
void            gpointer_rawCopy                        (t_gpointer *src, t_gpointer *dest);

t_scalar        *gpointer_getScalar                     (t_gpointer *gp);
t_word          *gpointer_getWord                       (t_gpointer *gp);
t_glist         *gpointer_getParentGlist                (t_gpointer *gp);
t_array         *gpointer_getParentArray                (t_gpointer *gp);

t_glist         *gpointer_getView                       (t_gpointer *gp);
t_word          *gpointer_getData                       (t_gpointer *gp);
t_symbol        *gpointer_getTemplateIdentifier         (t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol        *template_makeBindSymbolWithWildcard    (t_symbol *s);

t_template      *template_findByIdentifier              (t_symbol *templateIdentifier);
t_template      *template_new                           (t_symbol *templateIdentifier,
                                                            int argc,
                                                            t_atom *argv);

int             template_getSize                        (t_template *x);
t_dataslot      *template_getData                       (t_template *x);
t_glist         *template_getFirstInstanceView          (t_template *x);

int             template_hasInstance                    (t_template *x);
void            template_registerInstance               (t_template *x, t_struct *o);
void            template_unregisterInstance             (t_template *x, t_struct *o);

void            template_notify                         (t_template *x, 
                                                            t_glist *owner,
                                                            t_scalar *scalar,
                                                            t_symbol *s,
                                                            int argc,
                                                            t_atom *argv);

void            template_free                           (t_template *x);
int             template_isValid                        (t_template *x);
    
int             template_findField                      (t_template *x,
                                                            t_symbol *fieldName,
                                                            int *onset,
                                                            int *type,
                                                            t_symbol **templateIdentifier);

int             template_hasField                       (t_template *x, t_symbol *fieldName);
int             template_getIndexOfField                (t_template *x, t_symbol *fieldName);
int             template_getRaw                         (t_template *x,
                                                            t_symbol *fieldName,
                                                            int *index,
                                                            int *type,
                                                            t_symbol **templateIdentifier);
                                                            
int             template_fieldIsFloat                   (t_template *x, t_symbol *fieldName);
int             template_fieldIsSymbol                  (t_template *x, t_symbol *fieldName);
int             template_fieldIsArrayAndValid           (t_template *x, t_symbol *fieldName);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist         *struct_getView                         (t_struct *x);

void            struct_notify                           (t_struct *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            pointer_error                           (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            field_setAsFloatConstant                (t_fielddescriptor *fd, t_float f);
void            field_setAsFloatVariable                (t_fielddescriptor *fd, t_symbol *s);
void            field_setAsFloat                        (t_fielddescriptor *fd, int argc, t_atom *argv);
void            field_setAsArray                        (t_fielddescriptor *fd, int argc, t_atom *argv);

int             field_isFloat                           (t_fielddescriptor *fd);
int             field_isFloatConstant                   (t_fielddescriptor *fd);
int             field_isArray                           (t_fielddescriptor *fd);
int             field_isVariable                        (t_fielddescriptor *fd);

t_float         field_getFloatConstant                  (t_fielddescriptor *fd);
t_symbol        *field_getVariableName                  (t_fielddescriptor *fd);

t_float         field_convertValueToPosition            (t_fielddescriptor *fd, t_float v);
t_float         field_convertPositionToValue            (t_fielddescriptor *fd, t_float v);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            paint_scalarsEraseAll                   (void);
void            paint_scalarsDrawAll                    (void);
void            paint_scalarsRedrawAll                  (void);

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

void signal_setborrowed(t_signal *sig, t_signal *sig2);
void signal_makereusable(t_signal *sig);

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

#endif // __g_scalar_h_
