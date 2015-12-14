
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_pd_h_
#define __m_pd_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __cplusplus )

extern "C" {
    
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_MAJOR_VERSION    0
#define PD_MINOR_VERSION    46
#define PD_BUGFIX_VERSION   7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PD_COMPATIBILITY    46                      /* Compile time compatibility level. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef _WIN32
    #ifdef PD_INTERNAL
        #define EXTERN __declspec(dllexport) extern
    #else
        #define EXTERN __declspec(dllimport) extern
    #endif
#else
    #define EXTERN extern
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( _MSC_VER ) && !defined ( __cplusplus )
    #define EXTERN_STRUCT extern struct
#else
    #define EXTERN_STRUCT struct
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Microsoft Visual Studio does NOT provide the "stdint.h" header. */

#ifdef _MSC_VER
    typedef signed __int8               int8_t;
    typedef signed __int16              int16_t;
    typedef signed __int32              int32_t;
    typedef signed __int64              int64_t;
    typedef unsigned __int8             uint8_t;
    typedef unsigned __int16            uint16_t;
    typedef unsigned __int32            uint32_t;
    typedef unsigned __int64            uint64_t;
#else
    #include <stdint.h>
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdio.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MAXPDSTRING         1024                    /* Maximum size for a string. */
#define MAXPDARG            5                       /* Maximum number of typechecked arguments. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define T_TEXT              0
#define T_OBJECT            1
#define T_MESSAGE           2
#define T_ATOM              3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GP_NONE             0
#define GP_GLIST            1
#define GP_ARRAY            2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CLASS_DEFAULT       0
#define CLASS_PD            1
#define CLASS_GOBJ          2
#define CLASS_PATCHABLE     3
#define CLASS_NOINLET       8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef long    t_int;                  /* A pointer-size integer. */
typedef float   t_float;                /* A float type. */
typedef float   t_floatarg;             /* A float type parameter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN_STRUCT _array;
EXTERN_STRUCT _class;
EXTERN_STRUCT _outlet;
EXTERN_STRUCT _inlet;
EXTERN_STRUCT _binbuf;
EXTERN_STRUCT _clock;
EXTERN_STRUCT _outconnect;
EXTERN_STRUCT _glist;
EXTERN_STRUCT _widgetbehavior;
EXTERN_STRUCT _parentwidgetbehavior;
EXTERN_STRUCT _garray;
EXTERN_STRUCT _pdinstance;

#define t_array                 struct _array
#define t_class                 struct _class
#define t_outlet                struct _outlet
#define t_inlet                 struct _inlet
#define t_binbuf                struct _binbuf
#define t_clock                 struct _clock
#define t_outconnect            struct _outconnect
#define t_glist                 struct _glist
#define t_canvas                struct _glist
#define t_widgetbehavior        struct _widgetbehavior
#define t_parentwidgetbehavior  struct _parentwidgetbehavior
#define t_garray                struct _garray
#define t_pdinstance            struct _pdinstance

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _gstub {
    union {
        t_glist *gs_glist;
        t_array *gs_array;
    } gs_un;
    int gs_type;
    int gs_refcount;
    } t_gstub;

typedef struct _gpointer {
    union {   
        struct _scalar  *gp_scalar;
        union word      *gp_w;
    } gp_un;
    int     gp_valid;
    t_gstub *gp_stub;
    } t_gpointer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _symbol {
    char            *s_name;
    t_class         **s_thing;
    struct _symbol  *s_next;
    } t_symbol;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef union word {
    t_float     w_float;
    int         w_index;
    t_symbol    *w_symbol;
    t_gpointer  *w_gpointer;
    t_array     *w_array;
    t_binbuf    *w_binbuf;
    } t_word;

typedef enum {
    A_NULL,
    A_FLOAT,
    A_SYMBOL,
    A_POINTER,
    A_SEMI,
    A_COMMA,
    A_DEFFLOAT,
    A_DEFSYM,
    A_DOLLAR, 
    A_DOLLSYM,
    A_GIMME,
    A_CANT
    } t_atomtype;

typedef struct _atom {
    t_atomtype a_type;
    union word a_w;
    } t_atom;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://comments.gmane.org/gmane.comp.multimedia.puredata.general/94098 > */

typedef t_class *t_pd;

typedef struct _gobj {
    t_pd            g_pd;
    struct _gobj    *g_next;
    } t_gobj;

typedef struct _scalar {                
    t_gobj          sc_g;         
    t_symbol        *sc_template;
    t_word          sc_vec[1];          /* Indeterminate-length array (see above link). */
    } t_scalar;

typedef struct _text {
    t_gobj          te_g;
    t_binbuf        *te_binbuf;
    t_outlet        *te_outlet;
    t_inlet         *te_inlet;
    short           te_xpix;
    short           te_ypix;
    short           te_width; 
    short           te_type;
    } t_text;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _text t_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void (*t_method)(void);
typedef void *(*t_newmethod)(void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __APPLE__ ) && defined ( __aarch64__ )
    typedef void (*t_gotfn)(void *x);
#else
    typedef void (*t_gotfn)(void *x, ...);
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN t_pd pd_objectmaker;
EXTERN t_pd pd_canvasmaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_symbol s_pointer;
EXTERN t_symbol s_float;
EXTERN t_symbol s_symbol;
EXTERN t_symbol s_bang;
EXTERN t_symbol s_list;
EXTERN t_symbol s_anything;
EXTERN t_symbol s_signal;
EXTERN t_symbol s__N;
EXTERN t_symbol s__X;
EXTERN t_symbol s_x;
EXTERN t_symbol s_y;
EXTERN t_symbol s_;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_class *garray_class;
EXTERN t_class *scalar_class;
EXTERN t_class *glob_pdobject;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN void pd_typedmess    (t_pd *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void pd_forwardmess  (t_pd *x, int argc, t_atom *argv);
EXTERN void pd_vmess        (t_pd *x, t_symbol *s, char *fmt, ...);

EXTERN t_gotfn  getfn       (t_pd *x, t_symbol *s);
EXTERN t_gotfn  zgetfn      (t_pd *x, t_symbol *s);
EXTERN void     nullfn      (void);

EXTERN t_symbol *gensym     (const char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_gotfn1)(void *x, void *arg1);
typedef void (*t_gotfn2)(void *x, void *arg1, void *arg2);
typedef void (*t_gotfn3)(void *x, void *arg1, void *arg2, void *arg3);
typedef void (*t_gotfn4)(void *x, void *arg1, void *arg2, void *arg3, void *arg4);
typedef void (*t_gotfn5)(void *x, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);
    
#define mess0(x, s)                 ((*getfn((x), (s)))((x)))
#define mess1(x, s, a)              ((*(t_gotfn1)getfn((x), (s)))((x), (a)))
#define mess2(x, s, a, b)           ((*(t_gotfn2)getfn((x), (s)))((x), (a), (b)))
#define mess3(x, s, a, b, c)        ((*(t_gotfn3)getfn((x), (s)))((x), (a), (b), (c)))
#define mess4(x, s, a, b, c, d)     ((*(t_gotfn4)getfn((x), (s)))((x), (a), (b), (c), (d)))
#define mess5(x, s, a, b, c, d, e)  ((*(t_gotfn5)getfn((x), (s)))((x), (a), (b), (c), (d), (e)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN void *getbytes       (size_t nbytes);
EXTERN void *getzbytes      (size_t nbytes);
EXTERN void *copybytes      (void *src, size_t nbytes);
EXTERN void *resizebytes    (void *x, size_t oldsize, size_t newsize);
EXTERN void freebytes       (void *x, size_t nbytes);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SETSEMI(atom)               ((atom)->a_type = A_SEMI, (atom)->a_w.w_index = 0)
#define SETCOMMA(atom)              ((atom)->a_type = A_COMMA, (atom)->a_w.w_index = 0)
#define SETPOINTER(atom, gp)        ((atom)->a_type = A_POINTER, (atom)->a_w.w_gpointer = (gp))
#define SETFLOAT(atom, f)           ((atom)->a_type = A_FLOAT, (atom)->a_w.w_float = (f))
#define SETSYMBOL(atom, s)          ((atom)->a_type = A_SYMBOL, (atom)->a_w.w_symbol = (s))
#define SETDOLLAR(atom, n)          ((atom)->a_type = A_DOLLAR, (atom)->a_w.w_index = (n))
#define SETDOLLSYM(atom, s)         ((atom)->a_type = A_DOLLSYM, (atom)->a_w.w_symbol= (s))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN t_float  atom_getfloat               (t_atom *a);
EXTERN t_int    atom_getint                 (t_atom *a);
EXTERN t_float  atom_getfloatarg            (int which, int argc, t_atom *argv);
EXTERN t_int    atom_getintarg              (int which, int argc, t_atom *argv);

EXTERN t_symbol *atom_getsymbol             (t_atom *a);
EXTERN t_symbol *atom_getsymbolarg          (int which, int argc, t_atom *argv);

EXTERN void     atom_string                 (t_atom *a, char *buf, unsigned int bufsize);
EXTERN t_symbol *atom_gensym                (t_atom *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_binbuf *binbuf_new                 (void);
EXTERN t_binbuf *binbuf_duplicate           (t_binbuf *y);

EXTERN void     binbuf_free                 (t_binbuf *x);

EXTERN void     binbuf_text                 (t_binbuf *x, char *text, size_t size);
EXTERN void     binbuf_gettext              (t_binbuf *x, char **bufp, int *lengthp);
EXTERN void     binbuf_clear                (t_binbuf *x);
EXTERN void     binbuf_add                  (t_binbuf *x, int argc, t_atom *argv);
EXTERN void     binbuf_addv                 (t_binbuf *x, char *fmt, ...);
EXTERN void     binbuf_addbinbuf            (t_binbuf *x, t_binbuf *y);
EXTERN void     binbuf_addsemi              (t_binbuf *x);
EXTERN void     binbuf_restore              (t_binbuf *x, int argc, t_atom *argv);
EXTERN void     binbuf_print                (t_binbuf *x);
EXTERN int      binbuf_getnatom             (t_binbuf *x);
EXTERN t_atom   *binbuf_getvec              (t_binbuf *x);
EXTERN int      binbuf_resize               (t_binbuf *x, int newsize);
EXTERN void     binbuf_eval                 (t_binbuf *x, t_pd *target, int argc, t_atom *argv);
EXTERN int      binbuf_read                 (t_binbuf *b, char *filename, char *dirname, int crflag);
EXTERN int      binbuf_read_via_canvas      (t_binbuf *b, char *filename, t_canvas *canvas, int crflag);
EXTERN int      binbuf_read_via_path        (t_binbuf *b, char *filename, char *dirname, int crflag);
EXTERN int      binbuf_write                (t_binbuf *x, char *filename, char *dir, int crflag);
EXTERN void     binbuf_evalfile             (t_symbol *name, t_symbol *dir);
EXTERN t_symbol *binbuf_realizedollsym      (t_symbol *s, int ac, t_atom *av, int tonew);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_clock  *clock_new                  (void *owner, t_method fn);
EXTERN void     clock_free                  (t_clock *x);
EXTERN void     clock_set                   (t_clock *x, double systime);
EXTERN void     clock_delay                 (t_clock *x, double delaytime);
EXTERN void     clock_unset                 (t_clock *x);
EXTERN void     clock_setunit               (t_clock *x, double timeunit, int sampflag);
EXTERN double   clock_getlogicaltime        (void);
EXTERN double   clock_getsystime            (void);
EXTERN double   clock_gettimesince          (double prevsystime);
EXTERN double   clock_gettimesincewithunits (double prevsystime, double units, int sampflag);
EXTERN double   clock_getsystimeafter       (double delaytime);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_pd     *pd_new                     (t_class *c);

EXTERN void     pd_free                     (t_pd *x);
EXTERN void     pd_bind                     (t_pd *x, t_symbol *s);
EXTERN void     pd_unbind                   (t_pd *x, t_symbol *s);
EXTERN void     pd_pushsym                  (t_pd *x);
EXTERN void     pd_popsym                   (t_pd *x);
EXTERN void     pd_bang                     (t_pd *x);
EXTERN void     pd_pointer                  (t_pd *x, t_gpointer *gp);
EXTERN void     pd_float                    (t_pd *x, t_float f);
EXTERN void     pd_symbol                   (t_pd *x, t_symbol *s);
EXTERN void     pd_list                     (t_pd *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void     pd_anything                 (t_pd *x, t_symbol *s, int argc, t_atom *argv);

EXTERN t_pd     *pd_findbyclass             (t_symbol *s, t_class *c);
EXTERN t_symbol *pd_getfilename             (void);
EXTERN t_symbol *pd_getdirname              (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN void     gpointer_init               (t_gpointer *gp);
EXTERN void     gpointer_copy               (const t_gpointer *gpfrom, t_gpointer *gpto);
EXTERN void     gpointer_unset              (t_gpointer *gp);
EXTERN int      gpointer_check              (const t_gpointer *gp, int headok);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN int      garray_getfloatarray        (t_garray *x, int *size, t_float **vec);
EXTERN int      garray_getfloatwords        (t_garray *x, int *size, t_word **vec);
EXTERN void     garray_redraw               (t_garray *x);
EXTERN int      garray_npoints              (t_garray *x);
EXTERN char     *garray_vec                 (t_garray *x);
EXTERN void     garray_resize               (t_garray *x, t_floatarg f);
EXTERN void     garray_resize_long          (t_garray *x, long n);
EXTERN void     garray_usedindsp            (t_garray *x);
EXTERN void     garray_setsaveit            (t_garray *x, int saveit);
EXTERN t_glist  *garray_getglist            (t_garray *x);
EXTERN t_array  *garray_getarray            (t_garray *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_float  *value_get                  (t_symbol *s);
EXTERN void     value_release               (t_symbol *s);
EXTERN int      value_getfloat              (t_symbol *s, t_float *f);
EXTERN int      value_setfloat              (t_symbol *s, t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_inlet  *inlet_new                  (t_object *owner, t_pd *dest, t_symbol *s1, t_symbol *s2);
EXTERN t_inlet  *pointerinlet_new           (t_object *owner, t_gpointer *gp);
EXTERN t_inlet  *floatinlet_new             (t_object *owner, t_float *fp);
EXTERN t_inlet  *symbolinlet_new            (t_object *owner, t_symbol **sp);
EXTERN t_inlet  *signalinlet_new            (t_object *owner, t_float f);
EXTERN void     inlet_free                  (t_inlet *x);

EXTERN t_outlet *outlet_new                 (t_object *owner, t_symbol *s);
EXTERN void     outlet_bang                 (t_outlet *x);
EXTERN void     outlet_pointer              (t_outlet *x, t_gpointer *gp);
EXTERN void     outlet_float                (t_outlet *x, t_float f);
EXTERN void     outlet_symbol               (t_outlet *x, t_symbol *s);
EXTERN void     outlet_list                 (t_outlet *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void     outlet_anything             (t_outlet *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void     outlet_free                 (t_outlet *x);

EXTERN void     obj_list                    (t_object *x, t_symbol *s, int argc, t_atom *argv);

EXTERN t_symbol *outlet_getsymbol           (t_outlet *x);
EXTERN t_object *pd_checkobject             (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN void     glob_setfilename            (void *dummy, t_symbol *name, t_symbol *dir);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN void     canvas_setargs              (int argc, t_atom *argv);
EXTERN void     canvas_getargs              (int *argcp, t_atom **argvp);
EXTERN void     canvas_makefilename         (t_glist *c, char *file, char *result, int resultsize);
EXTERN t_symbol *canvas_getcurrentdir       (void);
EXTERN t_glist  *canvas_getcurrent          (void);
EXTERN t_symbol *canvas_getdir              (t_glist *x);

EXTERN void     canvas_dataproperties       (t_glist *x, t_scalar *sc, t_binbuf *b);
EXTERN int      canvas_open                 (t_canvas *x,
                                                const char *name,
                                                const char *ext,
                                                char *dirresult,
                                                char **nameresult,
                                                unsigned int size,
                                                int bin);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN void     sys_getversion              (int *major, int *minor, int *bugfix);
EXTERN int      sys_fontwidth               (int fontsize);
EXTERN int      sys_fontheight              (int fontsize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_parentwidgetbehavior *pd_getparentwidget   (t_pd *x);
EXTERN t_parentwidgetbehavior *class_parentwidget   (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define pd_class(x) (*(x))

#define CLASS_MAINSIGNALIN(c, type, field) class_domainsignalin(c, (char *)(&((type *)0)->field) - (char *)0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_savefn)(t_gobj *x, t_binbuf *b);
typedef void (*t_propertiesfn)(t_gobj *x, struct _glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN t_class  *class_new                  (t_symbol *name,
                                                t_newmethod newmethod,
                                                t_method freemethod,
                                                size_t size,
                                                int flags,
                                                t_atomtype arg1, ...);
                                                
EXTERN void     class_addcreator            (t_newmethod newmethod, t_symbol *s, t_atomtype type1, ...);
EXTERN void     class_addmethod             (t_class *c, t_method fn, t_symbol *sel, t_atomtype arg1, ...);
EXTERN void     class_addbang               (t_class *c, t_method fn);
EXTERN void     class_addpointer            (t_class *c, t_method fn);
EXTERN void     class_doaddfloat            (t_class *c, t_method fn);
EXTERN void     class_addsymbol             (t_class *c, t_method fn);
EXTERN void     class_addlist               (t_class *c, t_method fn);
EXTERN void     class_addanything           (t_class *c, t_method fn);
EXTERN void     class_sethelpsymbol         (t_class *c, t_symbol *s);
EXTERN void     class_setwidget             (t_class *c, t_widgetbehavior *w);
EXTERN void     class_setparentwidget       (t_class *c, t_parentwidgetbehavior *w);

EXTERN char     *class_getname              (t_class *c);
EXTERN char     *class_gethelpname          (t_class *c);
EXTERN char     *class_gethelpdir           (t_class *c);
EXTERN void     class_setdrawcommand        (t_class *c);
EXTERN int      class_isdrawcommand         (t_class *c);
EXTERN void     class_domainsignalin        (t_class *c, int onset);
EXTERN void     class_set_extern_dir        (t_symbol *s);

EXTERN void     class_setsavefn             (t_class *c, t_savefn f);
EXTERN t_savefn class_getsavefn             (t_class *c);
EXTERN void     obj_saveformat              (t_object *x, t_binbuf *bb);

EXTERN void           class_setpropertiesfn (t_class *c, t_propertiesfn f);
EXTERN t_propertiesfn class_getpropertiesfn (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef PD_CLASS_DEF
    #define class_addbang(x, y)         class_addbang((x), (t_method)(y))
    #define class_addpointer(x, y)      class_addpointer((x), (t_method)(y))
    #define class_addfloat(x, y)        class_doaddfloat((x), (t_method)(y))
    #define class_addsymbol(x, y)       class_addsymbol((x), (t_method)(y))
    #define class_addlist(x, y)         class_addlist((x), (t_method)(y))
    #define class_addanything(x, y)     class_addanything((x), (t_method)(y))
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN void post            (const char *fmt, ...);
EXTERN void startpost       (const char *fmt, ...);
EXTERN void poststring      (const char *s);
EXTERN void postfloat       (t_floatarg f);
EXTERN void postatom        (int argc, t_atom *argv);
EXTERN void endpost         (void);
EXTERN void error           (const char *fmt, ...);
EXTERN void verbose         (int level, const char *fmt, ...);
EXTERN void bug             (const char *fmt, ...);
EXTERN void pd_error        (void *object, const char *fmt, ...);
EXTERN void logpost         (const void *object, const int level, const char *fmt, ...);
EXTERN void sys_logerror    (const char *object, const char *s);
EXTERN void sys_unixerror   (const char *object);
EXTERN void sys_ouch        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN int      sched_geteventno    (void);
EXTERN double   sys_getrealtime     (void);
EXTERN int      sys_isreadablefile  (const char *name);
EXTERN int      sys_isabsolutepath  (const char *dir);
EXTERN void     sys_bashfilename    (const char *from, char *to);
EXTERN void     sys_unbashfilename  (const char *from, char *to);
EXTERN int      open_via_path       (const char *dir, 
                                        const char *name,
                                        const char *ext,
                                        char *dirresult,
                                        char **nameresult,
                                        unsigned int size,
                                        int bin);
                                    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN int  sys_open    (const char *path, int oflag, ...);
EXTERN int  sys_close   (int fd);

EXTERN FILE *sys_fopen  (const char *filename, const char *mode);
EXTERN int  sys_fclose  (FILE *stream);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN void sys_lock    (void);
EXTERN void sys_unlock  (void);
EXTERN int  sys_trylock (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef float t_sample;

typedef struct _signal {
    int                 s_n;
    t_sample            *s_vec;
    t_float             s_sr;
    int                 s_refcount;
    int                 s_isborrowed; 
    struct _signal      *s_borrowedfrom;
    struct _signal      *s_nextfree;
    struct _signal      *s_nextused;
    int                 s_vecsize;
    } t_signal;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_int *(*t_perfroutine)(t_int *args);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN t_int    *plus_perform       (t_int *args);
EXTERN t_int    *zero_perform       (t_int *args);
EXTERN t_int    *copy_perform       (t_int *args);

EXTERN void     dsp_add_plus        (t_sample *in1, t_sample *in2, t_sample *out, int n);
EXTERN void     dsp_add_copy        (t_sample *in, t_sample *out, int n);
EXTERN void     dsp_add_scalarcopy  (t_float *in, t_sample *out, int n);
EXTERN void     dsp_add_zero        (t_sample *out, int n);

EXTERN int      sys_getblksize      (void);
EXTERN t_float  sys_getsr           (void);
EXTERN int      sys_get_inchannels  (void);
EXTERN int      sys_get_outchannels (void);

EXTERN void     dsp_add             (t_perfroutine f, int n, ...);
EXTERN void     dsp_addv            (t_perfroutine f, int n, t_int *vec);
EXTERN void     pd_fft              (t_float *buf, int npoints, int inverse);
EXTERN int      ilog2               (int n);

EXTERN void     mayer_fht           (t_sample *fz, int n);
EXTERN void     mayer_fft           (int n, t_sample *real, t_sample *imag);
EXTERN void     mayer_ifft          (int n, t_sample *real, t_sample *imag);
EXTERN void     mayer_realfft       (int n, t_sample *real);
EXTERN void     mayer_realifft      (int n, t_sample *real);

EXTERN int      canvas_suspend_dsp  (void);
EXTERN void     canvas_resume_dsp   (int oldstate);
EXTERN void     canvas_update_dsp   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _resample {
    int         method;
    int         downsample;
    int         upsample;
    t_sample    *s_vec;
    int         s_n;
    t_sample    *coeffs;
    int         coefsize;
    t_sample    *buffer;
    int         bufsize;
    } t_resample;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN void resample_init       (t_resample *x);
EXTERN void resample_free       (t_resample *x);

EXTERN void resample_dsp        (t_resample *x, t_sample *in, int insize, t_sample *out, int outsize, int m);
EXTERN void resamplefrom_dsp    (t_resample *x, t_sample *in, int insize, int outsize, int m);
EXTERN void resampleto_dsp      (t_resample *x, t_sample *out, int insize, int outsize, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN t_float mtof     (t_float);
EXTERN t_float ftom     (t_float);
EXTERN t_float rmstodb  (t_float);
EXTERN t_float powtodb  (t_float);
EXTERN t_float dbtorms  (t_float);
EXTERN t_float dbtopow  (t_float);
EXTERN t_float q8_sqrt  (t_float);
EXTERN t_float q8_rsqrt (t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_guicallbackfn)(t_gobj *client, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN void sys_vgui                (char *fmt, ...);
EXTERN void sys_gui                 (char *s);
EXTERN void sys_pretendguibytes     (int n);
EXTERN void sys_queuegui            (void *client, t_glist *glist, t_guicallbackfn f);
EXTERN void sys_unqueuegui          (void *client);
EXTERN void gfxstub_new             (t_pd *owner, void *key, const char *cmd);
EXTERN void gfxstub_deleteforkey    (void *key);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_pdinstance     *pdinstance_new     (void);
EXTERN void             pd_setinstance      (t_pdinstance *x);
EXTERN void             pdinstance_free     (t_pdinstance *x);
EXTERN t_canvas         *pd_getcanvaslist   (void);
EXTERN int              pd_getdspstate      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef _MSC_VER

    #if defined ( __i386__ ) || defined ( __x86_64__ ) || defined ( __arm__ )

        typedef union {
            t_float f;
            unsigned int ui;
        } t_bigorsmall32;

        static inline int PD_BADFLOAT (t_float f)   /* Malformed float. */
        {
            t_bigorsmall32 pun;
            pun.f = f;
            pun.ui &= 0x7f800000;
            return ((pun.ui == 0) | (pun.ui == 0x7f800000));
        }

        static inline int PD_BIGORSMALL (t_float f)  /* Exponent underflow or overflow. */
        {
            t_bigorsmall32 pun;
            pun.f = f;
            return ((pun.ui & 0x20000000) == ((pun.ui >> 1) & 0x20000000));
        }

    #else
    
        #define PD_BADFLOAT(f)      0
        #define PD_BIGORSMALL(f)    0
        
    #endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else   // _MSC_VER

        #define PD_BADFLOAT(f)      ((((*(unsigned int*)&(f)) & 0x7f800000) == 0) || \
                                        (((*(unsigned int*)&(f)) & 0x7f800000) == 0x7f800000))
    
        #define PD_BIGORSMALL(f)    ((((*(unsigned int*)&(f)) & 0x60000000) == 0) || \
                                        (((*(unsigned int*)&(f)) & 0x60000000) == 0x60000000))

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined (__cplusplus)

}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_pd_h_
