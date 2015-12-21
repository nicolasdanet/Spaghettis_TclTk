
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
#define PD_PATCH_VERSION    7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_COMPATIBILITY    46                  /* Compile time compatibility level. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PD_STRUCT           struct

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef _WIN32
    #ifdef PD_INTERNAL
        #define PD_DLL      __declspec(dllexport) extern
    #else
        #define PD_DLL      __declspec(dllimport) extern 
    #endif
#else
    #define PD_DLL          __attribute__((visibility("default"))) extern
#endif



// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_STRING           1024                /* Maximum size for a string. */
#define PD_ARGUMENTS        5                   /* Maximum number of typechecked arguments. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CLASS_DEFAULT       0
#define CLASS_PURE          1
#define CLASS_GRAPHIC       2
#define CLASS_BOX           3
#define CLASS_NOINLET       8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef _WIN64
    typedef long long       t_int;              /* A pointer-size integer (LLP64). */
#else
    typedef long            t_int;              /* Ditto (LP64 / ILP64). */
#endif

typedef float               t_float;            /* A float type. */
typedef float               t_floatarg;         /* A float type parameter. */
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_STRUCT _array;
PD_STRUCT _class;
PD_STRUCT _outlet;
PD_STRUCT _inlet;
PD_STRUCT _binbuf;
PD_STRUCT _clock;
PD_STRUCT _outconnect;
PD_STRUCT _glist;
PD_STRUCT _widgetbehavior;
PD_STRUCT _parentwidgetbehavior;
PD_STRUCT _garray;
PD_STRUCT _pdinstance;

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
    A_SEMICOLON,
    A_COMMA,
    A_DEFFLOAT,
    A_DEFSYMBOL,
    A_DOLLAR, 
    A_DOLLARSYMBOL,
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
    t_pd            g_pd;                       /* MUST be the first. */
    struct _gobj    *g_next;
    } t_gobj;

typedef struct _scalar {
    t_gobj          sc_g;                       /* MUST be the first. */
    t_symbol        *sc_template;
    t_word          sc_vec[1];                  /* Indeterminate-length array (see above link). */
    } t_scalar;

typedef struct _text {
    t_gobj          te_g;                       /* MUST be the first. */
    t_binbuf        *te_binbuf;
    t_outlet        *te_outlet;
    t_inlet         *te_inlet;
    int             te_xpix;
    int             te_ypix;
    int             te_width; 
    int             te_type;
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

PD_DLL t_symbol s_pointer;
PD_DLL t_symbol s_float;
PD_DLL t_symbol s_symbol;
PD_DLL t_symbol s_bang;
PD_DLL t_symbol s_list;
PD_DLL t_symbol s_anything;
PD_DLL t_symbol s_signal;
PD_DLL t_symbol s__N;
PD_DLL t_symbol s__X;
PD_DLL t_symbol s_x;
PD_DLL t_symbol s_y;
PD_DLL t_symbol s_;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void pd_typedmess    (t_pd *x, t_symbol *s, int argc, t_atom *argv);
PD_DLL void pd_forwardmess  (t_pd *x, int argc, t_atom *argv);
PD_DLL void pd_vmess        (t_pd *x, t_symbol *s, char *fmt, ...);

PD_DLL t_gotfn  getfn       (t_pd *x, t_symbol *s);
PD_DLL t_gotfn  zgetfn      (t_pd *x, t_symbol *s);
PD_DLL void     nullfn      (void);

PD_DLL t_symbol *gensym     (const char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void *getbytes       (size_t nbytes);
PD_DLL void *getzbytes      (size_t nbytes);
PD_DLL void *copybytes      (void *src, size_t nbytes);
PD_DLL void *resizebytes    (void *x, size_t oldsize, size_t newsize);
PD_DLL void freebytes       (void *x, size_t nbytes);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_float  atom_getfloat               (t_atom *a);
PD_DLL t_int    atom_getint                 (t_atom *a);
PD_DLL t_float  atom_getfloatarg            (int which, int argc, t_atom *argv);
PD_DLL t_int    atom_getintarg              (int which, int argc, t_atom *argv);

PD_DLL t_symbol *atom_getsymbol             (t_atom *a);
PD_DLL t_symbol *atom_getsymbolarg          (int which, int argc, t_atom *argv);

PD_DLL void     atom_string                 (t_atom *a, char *buf, unsigned int bufsize);
PD_DLL t_symbol *atom_gensym                (t_atom *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL t_binbuf *binbuf_new                 (void);
PD_DLL t_binbuf *binbuf_duplicate           (t_binbuf *y);

PD_DLL void     binbuf_free                 (t_binbuf *x);

PD_DLL void     binbuf_text                 (t_binbuf *x, char *text, size_t size);
PD_DLL void     binbuf_gettext              (t_binbuf *x, char **bufp, int *lengthp);
PD_DLL void     binbuf_clear                (t_binbuf *x);
PD_DLL void     binbuf_add                  (t_binbuf *x, int argc, t_atom *argv);
PD_DLL void     binbuf_addv                 (t_binbuf *x, char *fmt, ...);
PD_DLL void     binbuf_addbinbuf            (t_binbuf *x, t_binbuf *y);
PD_DLL void     binbuf_addsemi              (t_binbuf *x);
PD_DLL void     binbuf_restore              (t_binbuf *x, int argc, t_atom *argv);
PD_DLL void     binbuf_print                (t_binbuf *x);
PD_DLL int      binbuf_getnatom             (t_binbuf *x);
PD_DLL t_atom   *binbuf_getvec              (t_binbuf *x);
PD_DLL int      binbuf_resize               (t_binbuf *x, int newsize);
PD_DLL void     binbuf_eval                 (t_binbuf *x, t_pd *target, int argc, t_atom *argv);
PD_DLL int      binbuf_read                 (t_binbuf *b, char *filename, char *dirname, int crflag);
PD_DLL int      binbuf_read_via_canvas      (t_binbuf *b, char *filename, t_canvas *canvas, int crflag);
PD_DLL int      binbuf_read_via_path        (t_binbuf *b, char *filename, char *dirname, int crflag);
PD_DLL int      binbuf_write                (t_binbuf *x, char *filename, char *dir, int crflag);
PD_DLL void     binbuf_evalfile             (t_symbol *name, t_symbol *dir);
PD_DLL t_symbol *binbuf_realizedollsym      (t_symbol *s, int ac, t_atom *av, int tonew);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL t_clock  *clock_new                  (void *owner, t_method fn);
PD_DLL void     clock_free                  (t_clock *x);
PD_DLL void     clock_set                   (t_clock *x, double systime);
PD_DLL void     clock_delay                 (t_clock *x, double delaytime);
PD_DLL void     clock_unset                 (t_clock *x);
PD_DLL void     clock_setunit               (t_clock *x, double timeunit, int sampflag);
PD_DLL double   clock_getlogicaltime        (void);
PD_DLL double   clock_getsystime            (void);
PD_DLL double   clock_gettimesince          (double prevsystime);
PD_DLL double   clock_gettimesincewithunits (double prevsystime, double units, int sampflag);
PD_DLL double   clock_getsystimeafter       (double delaytime);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL t_pd     *pd_new                     (t_class *c);

PD_DLL void     pd_free                     (t_pd *x);
PD_DLL void     pd_bind                     (t_pd *x, t_symbol *s);
PD_DLL void     pd_unbind                   (t_pd *x, t_symbol *s);
PD_DLL void     pd_pushsym                  (t_pd *x);
PD_DLL void     pd_popsym                   (t_pd *x);
PD_DLL void     pd_bang                     (t_pd *x);
PD_DLL void     pd_pointer                  (t_pd *x, t_gpointer *gp);
PD_DLL void     pd_float                    (t_pd *x, t_float f);
PD_DLL void     pd_symbol                   (t_pd *x, t_symbol *s);
PD_DLL void     pd_list                     (t_pd *x, t_symbol *s, int argc, t_atom *argv);
PD_DLL void     pd_anything                 (t_pd *x, t_symbol *s, int argc, t_atom *argv);

PD_DLL t_pd     *pd_findbyclass             (t_symbol *s, t_class *c);
PD_DLL t_symbol *pd_getfilename             (void);
PD_DLL t_symbol *pd_getdirname              (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void     gpointer_init               (t_gpointer *gp);
PD_DLL void     gpointer_copy               (const t_gpointer *gpfrom, t_gpointer *gpto);
PD_DLL void     gpointer_unset              (t_gpointer *gp);
PD_DLL int      gpointer_check              (const t_gpointer *gp, int headok);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL int      garray_getfloatarray        (t_garray *x, int *size, t_float **vec);
PD_DLL int      garray_getfloatwords        (t_garray *x, int *size, t_word **vec);
PD_DLL void     garray_redraw               (t_garray *x);
PD_DLL int      garray_npoints              (t_garray *x);
PD_DLL char     *garray_vec                 (t_garray *x);
PD_DLL void     garray_resize               (t_garray *x, t_floatarg f);
PD_DLL void     garray_resize_long          (t_garray *x, long n);
PD_DLL void     garray_usedindsp            (t_garray *x);
PD_DLL void     garray_setsaveit            (t_garray *x, int saveit);
PD_DLL t_glist  *garray_getglist            (t_garray *x);
PD_DLL t_array  *garray_getarray            (t_garray *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL t_float  *value_get                  (t_symbol *s);
PD_DLL void     value_release               (t_symbol *s);
PD_DLL int      value_getfloat              (t_symbol *s, t_float *f);
PD_DLL int      value_setfloat              (t_symbol *s, t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL t_inlet  *inlet_new                  (t_object *owner, t_pd *dest, t_symbol *s1, t_symbol *s2);
PD_DLL t_inlet  *pointerinlet_new           (t_object *owner, t_gpointer *gp);
PD_DLL t_inlet  *floatinlet_new             (t_object *owner, t_float *fp);
PD_DLL t_inlet  *symbolinlet_new            (t_object *owner, t_symbol **sp);
PD_DLL t_inlet  *signalinlet_new            (t_object *owner, t_float f);
PD_DLL void     inlet_free                  (t_inlet *x);

PD_DLL t_outlet *outlet_new                 (t_object *owner, t_symbol *s);
PD_DLL void     outlet_bang                 (t_outlet *x);
PD_DLL void     outlet_pointer              (t_outlet *x, t_gpointer *gp);
PD_DLL void     outlet_float                (t_outlet *x, t_float f);
PD_DLL void     outlet_symbol               (t_outlet *x, t_symbol *s);
PD_DLL void     outlet_list                 (t_outlet *x, t_symbol *s, int argc, t_atom *argv);
PD_DLL void     outlet_anything             (t_outlet *x, t_symbol *s, int argc, t_atom *argv);
PD_DLL void     outlet_free                 (t_outlet *x);

PD_DLL void     obj_list                    (t_object *x, t_symbol *s, int argc, t_atom *argv);

PD_DLL t_symbol *outlet_getsymbol           (t_outlet *x);
PD_DLL t_object *pd_checkobject             (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void     canvas_setargs              (int argc, t_atom *argv);
PD_DLL void     canvas_getargs              (int *argcp, t_atom **argvp);
PD_DLL void     canvas_makefilename         (t_glist *c, char *file, char *result, int resultsize);
PD_DLL t_symbol *canvas_getcurrentdir       (void);
PD_DLL t_glist  *canvas_getcurrent          (void);
PD_DLL t_symbol *canvas_getdir              (t_glist *x);

PD_DLL void     canvas_dataproperties       (t_glist *x, t_scalar *sc, t_binbuf *b);
PD_DLL int      canvas_open                 (t_canvas *x,
                                                const char *name,
                                                const char *ext,
                                                char *dirresult,
                                                char **nameresult,
                                                unsigned int size,
                                                int bin);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void     glob_setfilename            (void *dummy, t_symbol *name, t_symbol *dir);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void     sys_getversion              (int *major, int *minor, int *bugfix);
PD_DLL int      sys_fontwidth               (int fontsize);
PD_DLL int      sys_fontheight              (int fontsize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL t_parentwidgetbehavior *pd_getparentwidget   (t_pd *x);
PD_DLL t_parentwidgetbehavior *class_parentwidget   (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CLASS_SIGNAL(c, type, field)    class_domainsignalin(c, (char *)(&((type *)0)->field) - (char *)0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_savefn)(t_gobj *x, t_binbuf *b);
typedef void (*t_propertiesfn)(t_gobj *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_class  *class_new                  (t_symbol *name,
                                                t_newmethod newmethod,
                                                t_method freemethod,
                                                size_t size,
                                                int flags,
                                                t_atomtype arg1, ...);
                                                
PD_DLL void     class_addcreator            (t_newmethod newmethod, t_symbol *s, t_atomtype type1, ...);
PD_DLL void     class_addmethod             (t_class *c, t_method fn, t_symbol *sel, t_atomtype arg1, ...);
PD_DLL void     class_addbang               (t_class *c, t_method fn);
PD_DLL void     class_addpointer            (t_class *c, t_method fn);
PD_DLL void     class_addfloat              (t_class *c, t_method fn);
PD_DLL void     class_addsymbol             (t_class *c, t_method fn);
PD_DLL void     class_addlist               (t_class *c, t_method fn);
PD_DLL void     class_addanything           (t_class *c, t_method fn);
PD_DLL void     class_sethelpsymbol         (t_class *c, t_symbol *s);
PD_DLL void     class_setwidget             (t_class *c, t_widgetbehavior *w);
PD_DLL void     class_setparentwidget       (t_class *c, t_parentwidgetbehavior *w);

PD_DLL char     *class_getname              (t_class *c);
PD_DLL char     *class_gethelpname          (t_class *c);
PD_DLL char     *class_gethelpdir           (t_class *c);
PD_DLL void     class_setdrawcommand        (t_class *c);
PD_DLL int      class_isdrawcommand         (t_class *c);
PD_DLL void     class_domainsignalin        (t_class *c, int onset);
PD_DLL void     class_set_extern_dir        (t_symbol *s);

PD_DLL void     class_setsavefn             (t_class *c, t_savefn f);
PD_DLL t_savefn class_getsavefn             (t_class *c);
PD_DLL void     obj_saveformat              (t_object *x, t_binbuf *bb);

PD_DLL void           class_setpropertiesfn (t_class *c, t_propertiesfn f);
PD_DLL t_propertiesfn class_getpropertiesfn (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void post            (const char *fmt, ...);
PD_DLL void startpost       (const char *fmt, ...);
PD_DLL void poststring      (const char *s);
PD_DLL void postfloat       (t_floatarg f);
PD_DLL void postatom        (int argc, t_atom *argv);
PD_DLL void endpost         (void);
PD_DLL void error           (const char *fmt, ...);
PD_DLL void logpost         (const void *object, const int level, const char *fmt, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL int      sched_geteventno    (void);
PD_DLL double   sys_getrealtime     (void);
PD_DLL int      sys_isreadablefile  (const char *name);
PD_DLL int      sys_isabsolutepath  (const char *dir);
PD_DLL void     sys_bashfilename    (const char *from, char *to);
PD_DLL void     sys_unbashfilename  (const char *from, char *to);
PD_DLL int      open_via_path       (const char *dir, 
                                        const char *name,
                                        const char *ext,
                                        char *dirresult,
                                        char **nameresult,
                                        unsigned int size,
                                        int bin);
                                    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL int  sys_open    (const char *path, int oflag, ...);
PD_DLL int  sys_close   (int fd);

PD_DLL FILE *sys_fopen  (const char *filename, const char *mode);
PD_DLL int  sys_fclose  (FILE *stream);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void sys_lock    (void);
PD_DLL void sys_unlock  (void);
PD_DLL int  sys_trylock (void);

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

PD_DLL t_int    *plus_perform       (t_int *args);
PD_DLL t_int    *zero_perform       (t_int *args);
PD_DLL t_int    *copy_perform       (t_int *args);

PD_DLL void     dsp_add_plus        (t_sample *in1, t_sample *in2, t_sample *out, int n);
PD_DLL void     dsp_add_copy        (t_sample *in, t_sample *out, int n);
PD_DLL void     dsp_add_scalarcopy  (t_float *in, t_sample *out, int n);
PD_DLL void     dsp_add_zero        (t_sample *out, int n);

PD_DLL int      sys_getblksize      (void);
PD_DLL t_float  sys_getsr           (void);
PD_DLL int      sys_get_inchannels  (void);
PD_DLL int      sys_get_outchannels (void);

PD_DLL void     dsp_add             (t_perfroutine f, int n, ...);
PD_DLL void     dsp_addv            (t_perfroutine f, int n, t_int *vec);
PD_DLL void     pd_fft              (t_float *buf, int npoints, int inverse);
PD_DLL int      ilog2               (int n);

PD_DLL void     mayer_fht           (t_sample *fz, int n);
PD_DLL void     mayer_fft           (int n, t_sample *real, t_sample *imag);
PD_DLL void     mayer_ifft          (int n, t_sample *real, t_sample *imag);
PD_DLL void     mayer_realfft       (int n, t_sample *real);
PD_DLL void     mayer_realifft      (int n, t_sample *real);

PD_DLL int      canvas_suspend_dsp  (void);
PD_DLL void     canvas_resume_dsp   (int oldstate);
PD_DLL void     canvas_update_dsp   (void);

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

PD_DLL void resample_init       (t_resample *x);
PD_DLL void resample_free       (t_resample *x);

PD_DLL void resample_dsp        (t_resample *x, t_sample *in, int insize, t_sample *out, int outsize, int m);
PD_DLL void resamplefrom_dsp    (t_resample *x, t_sample *in, int insize, int outsize, int m);
PD_DLL void resampleto_dsp      (t_resample *x, t_sample *out, int insize, int outsize, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_float mtof     (t_float);
PD_DLL t_float ftom     (t_float);
PD_DLL t_float rmstodb  (t_float);
PD_DLL t_float powtodb  (t_float);
PD_DLL t_float dbtorms  (t_float);
PD_DLL t_float dbtopow  (t_float);
PD_DLL t_float q8_sqrt  (t_float);
PD_DLL t_float q8_rsqrt (t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_guicallbackfn)(t_gobj *client, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void sys_vgui                (char *fmt, ...);
PD_DLL void sys_gui                 (char *s);
PD_DLL void sys_pretendguibytes     (int n);
PD_DLL void sys_queuegui            (void *client, t_glist *glist, t_guicallbackfn f);
PD_DLL void sys_unqueuegui          (void *client);
PD_DLL void gfxstub_new             (t_pd *owner, void *key, const char *cmd);
PD_DLL void gfxstub_deleteforkey    (void *key);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL t_canvas *pd_getcanvaslist   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined (__cplusplus)

}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_pd_h_
