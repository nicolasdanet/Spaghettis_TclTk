
#ifndef __m_spaghettis_h_
#define __m_spaghettis_h_

#if 0   // Python script.

""" "

#endif  //

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://sourceforge.net/p/predef/wiki/OperatingSystems/ > */

#if defined ( _WIN32 ) || defined ( _WIN64 )
    #define     PD_WINDOWS          1
#elif defined ( __CYGWIN__ ) 
    #define     PD_CYGWIN           1
#elif defined ( __ANDROID__ )
    #define     PD_ANDROID          1
#elif defined ( __linux__ )
    #define     PD_LINUX            1
#elif defined ( __APPLE__ )
    #define     PD_APPLE            1
    #if defined ( TARGET_OS_IPHONE ) || defined ( TARGET_IPHONE_SIMULATOR )
        #define PD_IOS              1
    #else
        #define PD_OSX              1
    #endif
#elif defined ( __FreeBSD__ ) || defined ( __FreeBSD_kernel__ )
    #define     PD_BSD              1
#elif defined ( __GNU__ )
    #define     PD_HURD             1
#else
    #error "Unknown platform!"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://sourceforge.net/p/predef/wiki/Compilers/ > */

#if defined ( __clang__ )
    #define     PD_CLANG            1
    #define     PD_GCC              1
#elif defined ( __GNUC__ )
    #define     PD_GCC              1
#elif defined ( _MSC_VER )
    #define     PD_MSVC             1
#elif defined ( __MINGW32__ ) || defined ( __MINGW64__ )
    #define     PD_MINGW            1
#else
  #error "Unknown compiler!"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://sourceforge.net/p/predef/wiki/Architectures/ > */

#if defined ( __i386__ )
    #define     PD_CPU_x86          1
#elif defined ( __x86_64__ )
    #define     PD_CPU_AMD64        1
#elif defined ( __arm__ )
    #define     PD_CPU_ARM          1
#else
    #error "Unknown processor!"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://en.cppreference.com/w/cpp/language/types > */

#if defined ( __linux__ ) && defined ( __i386__ )
    #define     PD_ILP32            1
#elif defined ( _ILP32 ) || defined ( __ILP32__ )
    #define     PD_ILP32            1
#elif defined ( __LP64__ ) || defined ( _LP64 )
    #define     PD_LP64             1
#elif defined ( _WIN64 )
    #define     PD_LLP64            1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Type int must be at least 32-bit. */

#ifdef PD_LP64
#else
#ifdef PD_LLP64
#else
#ifdef PD_ILP32
#else
    #error "Unsupported data model!"
#endif
#endif
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

    #if PD_MSVC
    #ifdef _WIN64
        #define PD_64BIT            1
    #else
        #define PD_32BIT            1
    #endif
    #endif

    #if PD_MINGW
    #ifdef __MINGW64__
        #define PD_64BIT            1
    #else
        #define PD_32BIT            1
    #endif
    #endif
    
#else

    #if defined ( __LP64__ ) || defined ( _LP64 ) || defined ( __arm64__ )
        #define PD_64BIT            1
    #else
        #define PD_32BIT            1
    #endif

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef PD_64BIT
#else
#ifdef PD_32BIT
#else
    #error "Unknown architecture!"
#endif
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if defined ( _BIG_ENDIAN ) || defined ( __BIG_ENDIAN__ )
    #define PD_BIG_ENDIAN           1
#else
#if defined ( PD_WINDOWS ) || defined ( __LITTLE_ENDIAN__ )
    #define PD_LITTLE_ENDIAN        1
#else
    #include <endian.h>
    #if ( BYTE_ORDER == LITTLE_ENDIAN )
    #define PD_LITTLE_ENDIAN        1
    #else
    #define PD_BIG_ENDIAN           1
    #endif
#endif
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef PD_LITTLE_ENDIAN
#else
#ifdef PD_BIG_ENDIAN
#else
    #error "Unknown endianness!"
#endif
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_GCC
    #define PD_GCC_VERSION          (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_64BIT
    #define PD_MALLOC_ALIGNED       1           /* Assume malloc aligned to 16-bytes. */
#else
    #define PD_MALLOC_ALIGNED       0
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_MALLOC_ALIGNED
#if PD_CLANG
    #if __has_builtin(__builtin_assume_aligned)
        #define PD_ASSUME_ALIGNED   1
    #else
        #define PD_ASSUME_ALIGNED   0
    #endif
#elif PD_GCC && PD_GCC_VERSION >= 40700
    #define PD_ASSUME_ALIGNED       1
#else
    #define PD_ASSUME_ALIGNED       0
#endif
#else
    #define PD_ASSUME_ALIGNED       0
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_APPLE

#include "Availability.h"

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
#else
    #error "Unsupported platform!"
#endif

#else
#if PD_LINUX
#else
    #error "Unsupported platform!"
#endif // PD_LINUX
#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __cplusplus )
    #define PD_CPP                  1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_NAME                     "Spaghettis"
#define PD_NAME_LOWERCASE           "spaghettis"

#define PD_VERSION                  "0.9"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PD_VERSION_MAJOR            0
#define PD_VERSION_MINOR            9
#define PD_VERSION_PATCH            0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_PATCH                    ".pdpatch"
#define PD_HELP                     ".pdhelp"
#define PD_TEMPLATE                 ".pdtemplate"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_LINUX
    #if PD_64BIT
        #define PD_PLUGIN           ".pdobject64"
    #else
        #define PD_PLUGIN           ".pdobject32"
    #endif
#elif PD_APPLE
    #if PD_64BIT
        #define PD_PLUGIN           ".pdbundle64"
    #else
        #define PD_PLUGIN           ".pdbundle32"
    #endif
#elif PD_WINDOWS
    #if PD_64BIT
        #define PD_PLUGIN           ".pdlibrary64"
    #else
        #define PD_PLUGIN           ".pdlibrary32"
    #endif
#else
    #define PD_PLUGIN               ".so"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_TRANSLATE(s)             (s)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS
#if PD_BUILDING_APPLICATION
    #define PD_DLL                  __declspec(dllexport)
#else
    #define PD_DLL                  __declspec(dllimport)
#endif
#else
    #define PD_DLL                  __attribute__((visibility ("default")))
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if defined ( __cplusplus )

#if PD_WINDOWS
    #define PD_STUB                 extern "C" __declspec(dllexport)
#else
    #define PD_STUB                 extern "C" __attribute__((visibility ("default")))
#endif

#else

#if PD_WINDOWS
    #define PD_STUB                 __declspec(dllexport)
#else
    #define PD_STUB                 __attribute__((visibility ("default")))
#endif

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ! ( PD_BUILDING_APPLICATION )               /* Avoid namespace pollution. */

#include <stdlib.h>
#include <stdint.h>

#else 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <alloca.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

#include <io.h>
#include <process.h>
#include <sys/timeb.h> 
#include <tchar.h>
#include <time.h>
#include <windows.h>
#include <winbase.h>
#include <winsock.h>
#include <wtypes.h>
    
#else

#include <ftw.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
    
#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_APPLE

#include <mach-o/dyld.h>
#include <mach/mach_time.h>

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifndef PD_WITH_DEBUG
#define PD_WITH_DEBUG               0                   /* Debug mode. */
#endif

#ifndef PD_WITH_DEADCODE
#define PD_WITH_DEADCODE            0                   /* Include unused code. */
#endif

#ifndef PD_WITH_MAIN
#define PD_WITH_MAIN                1                   /* Use main entry. */
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* For now, only the MICA library is used. */

#define BELLE_MICA_ONLY             1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Enable memory leaks detector and assertions. */

#if PD_WITH_DEBUG
    #define BELLE_WITH_DEBUG        1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_BUILDING_APPLICATION

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_STRING                   1024                /* Maximum size for a string. */
#define PD_ARGUMENTS                2                   /* Maximum number of typechecked arguments. */
                                                        /* Use A_GIMME when more are requiered. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define CLASS_DEFAULT               0
#define CLASS_ABSTRACT              1
#define CLASS_INVISIBLE             2
#define CLASS_GRAPHIC               3
#define CLASS_BOX                   4

#define CLASS_NOINLET               8
#define CLASS_SIGNAL                16

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_LLP64
    typedef long long               t_int;              /* A pointer-size integer (LLP64). */
#else
    typedef long                    t_int;              /* Ditto (LP64 / ILP64). */
#endif

typedef double                      t_float;
typedef float                       t_sample;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef int                         t_error;
typedef int                         t_color;
typedef double                      t_systime;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef uint32_t                    t_keycode;
typedef uint64_t                    t_rand48;
typedef uint64_t                    t_seed;
typedef uint64_t                    t_id;
typedef int64_t                     t_phase;            /* Assumed two's complement. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Alignment may not work on stack (don't use local atomic variables). */

/* < http://gcc.gnu.org/bugzilla/show_bug.cgi?id=16660 > */
/* < http://stackoverflow.com/questions/841433/gcc-attribute-alignedx-explanation > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef int32_t     __attribute__ ((__aligned__ (4)))   t_int32Atomic;
typedef uint32_t    __attribute__ ((__aligned__ (4)))   t_uint32Atomic;
typedef uint64_t    __attribute__ ((__aligned__ (8)))   t_uint64Atomic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_64BIT
    typedef void*   __attribute__ ((__aligned__ (8)))   t_pointerAtomic;
#else
    typedef void*   __attribute__ ((__aligned__ (4)))   t_pointerAtomic;
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef double      __attribute__ ((__aligned__ (8)))   t_float64Atomic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_ERROR                    1
#define PD_ERROR_NONE               0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef int t_atomtype;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

enum {
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
    A_VOID,
    A_CANT,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_BUILDING_APPLICATION

struct _abstractions;
struct _box;
struct _clocks;
struct _constructor;
struct _dspcontext;
struct _dspthread;
struct _garray;
struct _gatom;
struct _gmaster;
struct _outconnect;
struct _receiver;
struct _ringbuffer;
struct _sfthread;
struct _struct;
struct _template;
struct _vinlet;
struct _voutlet;

#endif // PD_BUILDING_APPLICATION

struct _array;
struct _chain;
struct _class;
struct _clock;
struct _glist;
struct _gpointer;
struct _inlet;
struct _outlet;

#if PD_BUILDING_APPLICATION

#define t_abstractions              struct _abstractions
#define t_box                       struct _box
#define t_clocks                    struct _clocks
#define t_constructor               struct _constructor
#define t_dspcontext                struct _dspcontext
#define t_dspthread                 struct _dspthread
#define t_garray                    struct _garray
#define t_gatom                     struct _gatom
#define t_gmaster                   struct _gmaster
#define t_outconnect                struct _outconnect
#define t_receiver                  struct _receiver
#define t_ringbuffer                struct _ringbuffer
#define t_sfthread                  struct _sfthread
#define t_struct                    struct _struct
#define t_template                  struct _template
#define t_vinlet                    struct _vinlet
#define t_voutlet                   struct _voutlet

#endif // PD_BUILDING_APPLICATION

#define t_array                     struct _array
#define t_chain                     struct _chain
#define t_class                     struct _class
#define t_clock                     struct _clock
#define t_glist                     struct _glist
#define t_gpointer                  struct _gpointer
#define t_inlet                     struct _inlet
#define t_outlet                    struct _outlet

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _symbol {
    const char      *s_name;
    t_class         **s_thing;
    struct _symbol  *s_next;
    } t_symbol;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _buffer;

typedef union _word {
    t_float64Atomic w_float;
    t_symbol        *w_symbol;
    t_array         *w_array;
    } t_word;

typedef struct _atom {
    t_atomtype      a_type;
    union {
        t_float     a_float;
        int         a_index;
        t_symbol    *a_symbol;
        t_gpointer  *a_gpointer;
        void        *a_void;
    } a_w;
    } t_atom;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _buffer {
    int             b_allocated;
    int             b_size;
    t_atom          *b_vector;
    } t_buffer;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_class *t_pd;

typedef struct _gobj {
    t_pd            g_pd;                       /* MUST be the first. */
    t_id            g_id[3];
    struct _gobj    *g_next;
    } t_gobj;

/* CLASS_GRAPHIC. */

typedef struct _scalar {                        
    t_gobj          sc_g;                       /* MUST be the first. */
    int             sc_disable;
    t_glist         *sc_owner;
    t_symbol        *sc_templateIdentifier;
    t_word          *sc_element;
    } t_scalar;

/* CLASS_BOX. */

typedef enum {
    TYPE_COMMENT    = 0,
    TYPE_OBJECT     = 1,
    TYPE_MESSAGE,
    TYPE_ATOM
    } t_objecttype;

typedef struct _object {
    t_gobj          te_g;                       /* MUST be the first. */
    t_float64Atomic te_f;
    t_buffer        *te_buffer;
    t_inlet         *te_inlets;
    t_outlet        *te_outlets;
    int             te_x;
    int             te_y;
    int             te_width;                   /* Zero for undefined. */
    t_objecttype    te_type;
    } t_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef void    (*t_method)     (void *);
typedef void    *(*t_newmethod) (void);
typedef t_int   *(*t_perform)   (t_int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_BUILDING_APPLICATION

#if PD_WITH_DEBUG

#define PD_MEMORY_GET(n)                leak_getMemoryChecked ((n), __FUNCTION__, __LINE__)
#define PD_MEMORY_RESIZE(ptr, m, n)     leak_getMemoryResizeChecked ((ptr), (m), (n), __FUNCTION__, __LINE__)
#define PD_MEMORY_FREE(ptr)             leak_freeMemoryChecked ((ptr), __FUNCTION__, __LINE__);

#else

#define PD_MEMORY_GET(n)                memory_get ((n))
#define PD_MEMORY_RESIZE(ptr, m, n)     memory_getResize ((ptr), (m), (n))
#define PD_MEMORY_FREE(ptr)             memory_free ((ptr))

#endif // PD_WITH_DEBUG

#else

#define PD_MEMORY_GET(n)                memory_getForExternal ((n))
#define PD_MEMORY_RESIZE(ptr, m, n)     memory_getResizeForExternal ((ptr), (m), (n))
#define PD_MEMORY_FREE(ptr)             memory_freeForExternal ((ptr))

#endif // PD_BUILDING_APPLICATION

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _signal {
    t_float         s_sampleRate;
    int             s_vectorSize;
    int             s_overlap;
    int             s_hasBorrowed;
    t_sample        *s_vector;
    t_sample        *s_unused;
    struct _signal  *s_next;
    } t_signal;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Deprecated. */

#define dsp_add(...)                            chain_append (instance_chainGetTemporary(), __VA_ARGS__)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Use macros below. */

#define dsp_add1(fn, a)                         chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 1, \
                                                                (t_int)(a))

#define dsp_add2(fn, a, b)                      chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 2, \
                                                                (t_int)(a), \
                                                                (t_int)(b))

#define dsp_add3(fn, a, b, c)                   chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 3, \
                                                                (t_int)(a), \
                                                                (t_int)(b), \
                                                                (t_int)(c))

#define dsp_add4(fn, a, b, c, d)                chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 4, \
                                                                (t_int)(a), \
                                                                (t_int)(b), \
                                                                (t_int)(c), \
                                                                (t_int)(d))

#define dsp_add5(fn, a, b, c, d, e)             chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 5, \
                                                                (t_int)(a), \
                                                                (t_int)(b), \
                                                                (t_int)(c), \
                                                                (t_int)(d), \
                                                                (t_int)(e))

#define dsp_add6(fn, a, b, c, d, e, f)          chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 6, \
                                                                (t_int)(a), \
                                                                (t_int)(b), \
                                                                (t_int)(c), \
                                                                (t_int)(d), \
                                                                (t_int)(e), \
                                                                (t_int)(f))

#define dsp_add7(fn, a, b, c, d, e, f, g)       chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 7, \
                                                                (t_int)(a), \
                                                                (t_int)(b), \
                                                                (t_int)(c), \
                                                                (t_int)(d), \
                                                                (t_int)(e), \
                                                                (t_int)(f), \
                                                                (t_int)(g))

#define dsp_add8(fn, a, b, c, d, e, f, g, h)    chain_append (instance_chainGetTemporary(), \
                                                                (t_perform)(fn), 8, \
                                                                (t_int)(a), \
                                                                (t_int)(b), \
                                                                (t_int)(c), \
                                                                (t_int)(d), \
                                                                (t_int)(e), \
                                                                (t_int)(f), \
                                                                (t_int)(g), \
                                                                (t_int)(h))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __cplusplus )

extern "C" {

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that NONE of those functions are considered thread-safe. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_symbol *gensym                         (const char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Use the macros instead. */

PD_DLL void     *memory_getForExternal          (size_t n);
PD_DLL void     *memory_getResizeForExternal    (void *ptr, size_t oldSize, size_t newSize);

PD_DLL void     memory_freeForExternal          (void *ptr);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_pd     *pd_new                         (t_class *c);

PD_DLL void     pd_free                         (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void     pd_bind                         (t_pd *x, t_symbol *s);
PD_DLL void     pd_unbind                       (t_pd *x, t_symbol *s);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_DLL void     pd_bang                         (t_pd *x);
PD_DLL void     pd_pointer                      (t_pd *x, t_gpointer *gp);
PD_DLL void     pd_float                        (t_pd *x, t_float f);
PD_DLL void     pd_symbol                       (t_pd *x, t_symbol *s);
PD_DLL void     pd_list                         (t_pd *x, int argc, t_atom *argv);
PD_DLL void     pd_message                      (t_pd *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_pd     *symbol_getThing                (t_symbol *s);
PD_DLL t_pd     *symbol_getThingByClass         (t_symbol *s, t_class *c);

PD_DLL const char *symbol_getName               (t_symbol *s);

PD_DLL int      symbol_hasThing                 (t_symbol *s);
PD_DLL int      symbol_hasThingQuiet            (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_class  *class_new                      (t_symbol *name,
                                                    t_newmethod newMethod,
                                                    t_method freeMethod,
                                                    size_t size,
                                                    int flags,
                                                    t_atomtype type1, ...);

PD_DLL void     class_addCreator                (t_newmethod newMethod, t_symbol *s, t_atomtype type1, ...);

PD_DLL void     class_addMethod                 (t_class *c, t_method fn, t_symbol *s, t_atomtype type1, ...);
PD_DLL void     class_free                      (t_class *c);

PD_DLL void     class_addBang                   (t_class *c, t_method fn);
PD_DLL void     class_addFloat                  (t_class *c, t_method fn);
PD_DLL void     class_addSymbol                 (t_class *c, t_method fn);
PD_DLL void     class_addPointer                (t_class *c, t_method fn);
PD_DLL void     class_addList                   (t_class *c, t_method fn);
PD_DLL void     class_addAnything               (t_class *c, t_method fn);

PD_DLL void     class_setHelpName               (t_class *c, t_symbol *s);
PD_DLL void     class_setHelpDirectory          (t_class *c, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_inlet  *inlet_newPointer               (t_object *owner, t_gpointer *gp);
PD_DLL t_inlet  *inlet_newFloat                 (t_object *owner, t_float *fp);
PD_DLL t_inlet  *inlet_newSymbol                (t_object *owner, t_symbol **sp);
PD_DLL t_inlet  *inlet_newSignal                (t_object *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_outlet *outlet_newBang                 (t_object *owner);
PD_DLL t_outlet *outlet_newPointer              (t_object *owner);
PD_DLL t_outlet *outlet_newFloat                (t_object *owner);
PD_DLL t_outlet *outlet_newSymbol               (t_object *owner);
PD_DLL t_outlet *outlet_newList                 (t_object *owner);
PD_DLL t_outlet *outlet_newAnything             (t_object *owner);
PD_DLL t_outlet *outlet_newSignal               (t_object *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL void     outlet_bang                     (t_outlet *x);
PD_DLL void     outlet_pointer                  (t_outlet *x, t_gpointer *gp);
PD_DLL void     outlet_float                    (t_outlet *x, t_float f);
PD_DLL void     outlet_symbol                   (t_outlet *x, t_symbol *s);
PD_DLL void     outlet_list                     (t_outlet *x, int argc, t_atom *argv);
PD_DLL void     outlet_anything                 (t_outlet *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL void     atom_setSymbol                  (t_atom *a, t_symbol *s);
PD_DLL void     atom_setFloat                   (t_atom *a, t_float f);

PD_DLL t_symbol *atom_getSymbol                 (t_atom *a);
PD_DLL t_symbol *atom_getSymbolAtIndex          (int n, int argc, t_atom *argv);

PD_DLL t_float  atom_getFloat                   (t_atom *a);
PD_DLL t_float  atom_getFloatAtIndex            (int n, int argc, t_atom *argv);

PD_DLL char     *atom_atomsToString             (int argc, t_atom *argv);   /* Caller acquires ownership. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_buffer *buffer_new                     (void);
PD_DLL t_buffer *buffer_newCopy                 (t_buffer *toCopy);
PD_DLL t_atom   *buffer_getAtoms                (t_buffer *x);
PD_DLL t_atom   *buffer_getAtomAtIndex          (t_buffer *x, int n);
PD_DLL t_atom   *buffer_getAtomAtIndexChecked   (t_buffer *x, int n);

PD_DLL int      buffer_getSize                  (t_buffer *x);
PD_DLL void     buffer_free                     (t_buffer *x);
PD_DLL void     buffer_clear                    (t_buffer *x);
PD_DLL void     buffer_shuffle                  (t_buffer *x);

PD_DLL void     buffer_reserve                  (t_buffer *x, int n);
PD_DLL void     buffer_resize                   (t_buffer *x, int n);
PD_DLL t_error  buffer_extend                   (t_buffer *x, int start, int end, int n);
PD_DLL t_error  buffer_getAtIndex               (t_buffer *x, int n, t_atom *a);
PD_DLL t_error  buffer_setAtIndex               (t_buffer *x, int n, t_atom *a);
PD_DLL t_error  buffer_insertAtIndex            (t_buffer *x, int n, t_atom *a);
PD_DLL t_error  buffer_pop                      (t_buffer *x, t_atom *a);

PD_DLL t_error  buffer_setFloatAtIndex          (t_buffer *x, int n, t_float f);
PD_DLL t_error  buffer_setSymbolAtIndex         (t_buffer *x, int n, t_symbol *s);

PD_DLL t_float  buffer_getFloatAtIndex          (t_buffer *x, int n);
PD_DLL t_symbol *buffer_getSymbolAtIndex        (t_buffer *x, int n);

PD_DLL void     buffer_prepend                  (t_buffer *x, int argc, t_atom *argv);
PD_DLL void     buffer_append                   (t_buffer *x, int argc, t_atom *argv);
PD_DLL void     buffer_appendAtom               (t_buffer *x, t_atom *a);
PD_DLL void     buffer_appendBuffer             (t_buffer *x, t_buffer *y);
PD_DLL void     buffer_appendFloat              (t_buffer *x, t_float f);
PD_DLL void     buffer_appendSymbol             (t_buffer *x, t_symbol *s);
PD_DLL void     buffer_appendPointer            (t_buffer *x, t_gpointer *gp);
PD_DLL void     buffer_appendDollar             (t_buffer *x, int n);
PD_DLL void     buffer_appendDollarSymbol       (t_buffer *x, t_symbol *s);
PD_DLL void     buffer_appendSemicolon          (t_buffer *x);
PD_DLL void     buffer_appendComma              (t_buffer *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that all clock functions MUST be called only by one (and the same) thread. */
/* The new and free functions might be called NON-CONCURRENTLY in other thread. */
/* E.g. constructed in the main thread with delay and unset in the DSP perform. */

PD_DLL t_clock  *clock_new                      (void *owner, t_method fn);

PD_DLL void     clock_free                      (t_clock *x);
PD_DLL void     clock_unset                     (t_clock *x);                   /* Usable in DSP. */
PD_DLL void     clock_delay                     (t_clock *x, double delay);     /* Usable in DSP. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL void     post                            (const char *fmt, ...);
PD_DLL void     post_warning                    (const char *fmt, ...);
PD_DLL void     post_error                      (const char *fmt, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_glist  *instance_contextGetCurrent     (void);
PD_DLL t_chain  *instance_chainGetTemporary     (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL t_sample *signal_getVector               (t_signal *s);

PD_DLL t_float  signal_getSampleRate            (t_signal *s);
PD_DLL int      signal_getVectorSize            (t_signal *s);
PD_DLL int      signal_getOverlap               (t_signal *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PD_DLL void     chain_append                    (t_chain *x, t_perform f, int n, ...);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __cplusplus )

}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_BUILDING_APPLICATION

#define class_addDSP(c, m)          class_addMethod ((c), (t_method)(m), sym_dsp, A_CANT, A_NULL)
#define class_addClick(c, m)        class_addMethod ((c), (t_method)(m), sym_click, A_GIMME, A_NULL)
#define class_addLoadbang(c, m)     class_addMethod ((c), (t_method)(m), sym_loadbang, A_NULL)
#define class_addClosebang(c, m)    class_addMethod ((c), (t_method)(m), sym_closebang, A_NULL)
#define class_addPolling(c, m)      class_addMethod ((c), (t_method)(m), sym__polling, A_NULL)
#define class_addAutorelease(c, m)  class_addMethod ((c), (t_method)(m), sym__autorelease, A_NULL)

#else

#define class_addDSP(c, m)          class_addMethod ((c), (t_method)(m), gensym ("dsp"), A_CANT, A_NULL)
#define class_addClick(c, m)        class_addMethod ((c), (t_method)(m), gensym ("click"), A_GIMME, A_NULL)
#define class_addLoadbang(c, m)     class_addMethod ((c), (t_method)(m), gensym ("loadbang"), A_NULL)
#define class_addClosebang(c, m)    class_addMethod ((c), (t_method)(m), gensym ("closebang"), A_NULL)

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// ====================================

#if 0

" """

import sys
import json
import os
import subprocess

assert sys.version_info >= (3, 4)

from pathlib import Path

manifest = { }
options  = { }

def manifestParse(path):
    global manifest
    manifest["source"]       = str(path)
    manifest["visibility"]   = "-fvisibility=hidden"
    if path.match('*.cpp'):
        manifest["compiler"] = "g++"
    else:
        manifest["compiler"] = "gcc"
    if sys.platform.startswith('linux'):
        manifest["plugin"]   = "-shared -fpic"
        manifest["product"]  = str(path.with_suffix(".pdobject"))
    elif sys.platform.startswith('darwin'):
        manifest["plugin"]   = "-bundle -undefined dynamic_lookup -bind_at_load"
        manifest["product"]  = str(path.with_suffix(".pdbundle"))

def manifestParseJSON(path):
    if path.exists():
        with path.open() as f:
            global options
            try:
                options = json.load(f)
            except:
                print("Invalid JSON file: " + str(path))

def buildPlugin():
    global manifest
    command = []
    command.append(manifest["compiler"])
    command.append("-I" + manifest["spaghettis"])
    for v in options.values():
        command.append(v)
    command.append(manifest["plugin"])
    command.append(manifest["visibility"])
    command.append(manifest["source"])
    command.append("-o")
    command.append(manifest["product"])
    os.system(' '.join(command))
    object = Path(manifest["product"])
    ret = subprocess.run(["file", str(object)], stdout=subprocess.PIPE)
    if any(x in str(ret.stdout) for x in ['x86_64', '64-bit']):
        object.rename(str(object) + '64')
        print (str(object) + '64')
    elif any(x in str(ret.stdout) for x in ['i386', '32-bit']):
        object.rename(str(object) + '32')
        print (str(object) + '32')

if __name__ == "__main__":
    header = Path(sys.argv.pop(0))
    manifest["spaghettis"] = str(header.resolve().parent)
    for file in sys.argv:
        options.clear()
        p = Path(file).resolve()
        if p.exists():
            manifestParse(p)
            o = p.with_suffix('.json')
            manifestParseJSON(o)
            buildPlugin()
        else:
            print("No such file: " + file)

#endif  // Python script.

#endif  // __m_spaghettis_h_
