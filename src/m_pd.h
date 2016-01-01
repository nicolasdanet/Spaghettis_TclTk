
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
#pragma mark -

/* < http://sourceforge.net/p/predef/wiki/OperatingSystems/ > */

#if defined ( _WIN32 ) || defined ( _WIN64 )
    #define     PD_WINDOWS      1
#elif defined ( __CYGWIN__ ) 
    #define     PD_CYGWIN       1
#elif defined ( ANDROID ) || defined ( __ANDROID__ )
    #define     PD_ANDROID      1
#elif defined ( LINUX ) || defined ( __linux__ )
    #define     PD_LINUX        1
#elif defined ( __APPLE__ )
    #define     PD_APPLE        1
    #if defined ( TARGET_OS_IPHONE ) || defined ( TARGET_IPHONE_SIMULATOR )
        #define PD_IOS          1
    #else
        #define PD_OSX          1
    #endif
#elif defined ( __FreeBSD__ ) || defined ( __FreeBSD_kernel__ )
    #define     PD_BSD          1
#elif defined ( __GNU__ )
    #define     PD_HURD         1
#else
    #error "Unknown platform!"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* < http://sourceforge.net/p/predef/wiki/Compilers/ > */

#if defined ( __clang__ )
    #define     PD_CLANG        1
    #define     PD_GCC          1
#elif defined ( __GNUC__ )
    #define     PD_GCC          1
#elif defined ( _MSC_VER )
    #define     PD_MSVC         1
#elif defined ( __MINGW32__ ) || defined ( __MINGW64__ )
    #define     PD_MINGW        1
#else
  #error "Unknown compiler!"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* < http://sourceforge.net/p/predef/wiki/Architectures/ > */

#if defined ( __i386__ )
    #define     PD_CPU_x86      1
#elif defined ( __x86_64__ )
    #define     PD_CPU_AMD64    1
#elif defined ( __arm__ )
    #define     PD_CPU_ARM      1
#else
    #error "Unknown processor!"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __LP64__ ) || defined ( _LP64 )
    #define     PD_LP64         1
#elif defined ( _WIN64 )
    #define     PD_LLP64        1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __cplusplus )

extern "C" {

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef PD_DEBUG
#define PD_DEBUG            1                       /* Log false assertions. */
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_NAME             "PureData"
#define PD_FILE             ".pd"
#define PD_VERSION          "0.46.7"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_TRANSLATE(s)     (s)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_STRUCT           struct

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS
    #if PD_EXPORT
        #define PD_DLL      __declspec(dllexport) extern
    #else
        #define PD_DLL      __declspec(dllimport) extern 
    #endif
#else
    #define PD_DLL          __attribute__((visibility("default"))) extern
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS
    #include <io.h>
#else
    #include <unistd.h>
#endif

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

#if PD_LLP64
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
    int gs_count;
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
    t_float         w_float;
    int             w_index;
    t_symbol        *w_symbol;
    t_gpointer      *w_gpointer;
    t_array         *w_array;
    t_binbuf        *w_binbuf;
    } t_word;

typedef enum {
    A_NULL          = 0,
    A_FLOAT         = 1,
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
    t_atomtype      a_type;
    union word      a_w;
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
    t_gobj          sc_g;
    t_symbol        *sc_template;
    t_word          sc_vector[1];               /* Indeterminate size (see above link). */
    } t_scalar;

typedef struct _text {
    t_gobj          te_g;
    t_binbuf        *te_binbuf;
    t_outlet        *te_outlet;
    t_inlet         *te_inlet;
    int             te_xCoordinate;
    int             te_yCoordinate;
    int             te_width; 
    int             te_type;
    } t_text;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _text t_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_method)(void *);
typedef void *(*t_newmethod)(void);

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

PD_DLL t_symbol *gensym                 (const char *s);

PD_DLL void     *sys_getMemory          (size_t n);
PD_DLL void     *sys_getMemoryCopy      (void *src, size_t n);
PD_DLL void     *sys_getMemoryResize    (void *ptr, size_t oldSize, size_t newSize);

PD_DLL void     sys_freeMemory          (void *ptr, size_t n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_pd     *pd_new                 (t_class *c);
PD_DLL t_pd     *pd_findByClass         (t_symbol *s, t_class *c);
PD_DLL t_symbol *pd_getfilename         (void);
PD_DLL t_symbol *pd_getdirname          (void);

PD_DLL void     pd_free                 (t_pd *x);
PD_DLL void     pd_bind                 (t_pd *x, t_symbol *s);
PD_DLL void     pd_unbind               (t_pd *x, t_symbol *s);
PD_DLL void     pd_bang                 (t_pd *x);
PD_DLL void     pd_pointer              (t_pd *x, t_gpointer *gp);
PD_DLL void     pd_float                (t_pd *x, t_float f);
PD_DLL void     pd_symbol               (t_pd *x, t_symbol *s);
PD_DLL void     pd_list                 (t_pd *x, int argc, t_atom *argv);
PD_DLL void     pd_message              (t_pd *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CLASS_SIGNAL(c, t, field)           class_addSignal (c, (char *)(&((t *)0)->field) - (char *)0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_class  *class_new                  (t_symbol *name,
                                                t_newmethod newMethod,
                                                t_method freeMethod,
                                                size_t size,
                                                int flags,
                                                t_atomtype type1, ...);

PD_DLL void     class_addSignal             (t_class *c, int offset);
PD_DLL void     class_addCreator            (t_newmethod newmethod, t_symbol *s, t_atomtype type1, ...);
PD_DLL void     class_addMethod             (t_class *c, t_method fn, t_symbol *s, t_atomtype type1, ...);

PD_DLL void     class_addBang               (t_class *c, t_method fn);
PD_DLL void     class_addPointer            (t_class *c, t_method fn);
PD_DLL void     class_addFloat              (t_class *c, t_method fn);
PD_DLL void     class_addSymbol             (t_class *c, t_method fn);
PD_DLL void     class_addList               (t_class *c, t_method fn);
PD_DLL void     class_addAnything           (t_class *c, t_method fn);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void     obj_saveformat              (t_object *x, t_binbuf *bb);

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

PD_DLL int      sys_fontwidth               (int fontsize);
PD_DLL int      sys_fontheight              (int fontsize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void post            (const char *fmt, ...);
PD_DLL void post_log        (const char *fmt, ...);
PD_DLL void post_error      (const char *fmt, ...);

PD_DLL void post_atoms      (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    int                 s_blockSize;
    int                 s_vectorSize;
    t_sample            *s_vector;
    t_float             s_sampleRate;
    int                 s_count;
    int                 s_isBorrowed;
    struct _signal      *s_borrowedFrom;
    struct _signal      *s_nextFree;
    struct _signal      *s_nextUsed;
    } t_signal;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_int *(*t_perform)(t_int *args);

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

PD_DLL void     dsp_add             (t_perform f, int n, ...);
PD_DLL void     dsp_addv            (t_perform f, int n, t_int *vec);
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
    int         r_type;
    int         r_downSample;
    int         r_upSample;
    int         r_vectorSize;
    int         r_coefficientsSize;
    int         r_bufferSize;
    t_sample    *r_vector;
    t_sample    *r_coefficients;
    t_sample    *r_buffer;
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

#if defined ( __cplusplus )

}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_pd_h_
