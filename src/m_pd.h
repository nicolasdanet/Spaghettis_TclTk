
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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
#pragma mark -

/* < http://en.cppreference.com/w/cpp/language/types > */

#if defined ( _ILP32 ) || defined ( __ILP32__ )
    #define     PD_ILP32        1
#endif

#if defined ( __LP64__ ) || defined ( _LP64 )
    #define     PD_LP64         1
#elif defined ( _WIN64 )
    #define     PD_LLP64        1
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Type int must be at least 32-bit. */

#ifdef PD_LP64
#else
#ifdef PD_LLP64
#else
#ifdef PD_ILP32
    #error "Unsupported data model!"
#endif
#endif
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

    #if PD_MSVC
    #ifdef _WIN64
        #define PD_64BIT        1
    #else
        #define PD_32BIT        1
    #endif
    #endif

    #if PD_MINGW
    #ifdef __MINGW64__
        #define PD_64BIT        1
    #else
        #define PD_32BIT        1
    #endif
    #endif
    
#else

    #if defined ( __LP64__ ) || defined ( _LP64 ) || defined ( __arm64__ )
        #define PD_64BIT        1
    #else
        #define PD_32BIT        1
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
#pragma mark -

#if defined ( _BIG_ENDIAN ) || defined ( __BIG_ENDIAN__ )
    #define PD_BIG_ENDIAN       1
#else
#if defined ( PD_WINDOWS ) || defined ( __LITTLE_ENDIAN__ )
    #define PD_LITTLE_ENDIAN    1
#else
    #include <endian.h>
    #if ( BYTE_ORDER == LITTLE_ENDIAN )
    #define PD_LITTLE_ENDIAN    1
    #else
    #define PD_BIG_ENDIAN       1
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
#pragma mark -

#ifndef PD_WITH_DEBUG
#define PD_WITH_DEBUG           0           /* Debug mode. */
#endif

#ifndef PD_WITH_LOGGER
#define PD_WITH_LOGGER          0           /* Debug with wait-free logger. */
#endif

#ifndef PD_WITH_ALLOCA
#define PD_WITH_ALLOCA          1           /* Message passing uses alloca function. */
#endif

#ifndef PD_WITH_NOGUI           
#define PD_WITH_NOGUI           0           /* Don't use the GUI. */
#endif

#ifndef PD_WITH_LEGACY
#define PD_WITH_LEGACY          1           /* Compatibility. */
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ( PD_LINUX || PD_BSD || PD_HURD || PD_CYGWIN || PD_APPLE )

#ifndef PD_WITH_REALTIME
#define PD_WITH_REALTIME        1
#endif

#else

#ifndef PD_WITH_REALTIME
#define PD_WITH_REALTIME        0
#endif

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ( PD_WITH_REALTIME && ( PD_LINUX || PD_BSD || PD_HURD ) )

#define PD_WATCHDOG             1

#else

#define PD_WATCHDOG             0

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_NAME                 "PureData"
#define PD_NAME_LOWERCASE       "puredata"
#define PD_NAME_SHORT           "Pd"

#define PD_VERSION              "0.9"
               
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_PATCH                ".pd"
#define PD_HELP                 ".pdhelp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ( PD_LINUX || PD_BSD || PD_HURD )
    #if PD_64BIT
        #define PD_PLUGIN       ".pdobject64"
    #else
        #define PD_PLUGIN       ".pdobject32"
    #endif
#elif PD_APPLE
    #if PD_64BIT
        #define PD_PLUGIN       ".pdbundle64"
    #else
        #define PD_PLUGIN       ".pdbundle32"
    #endif
#elif ( PD_WINDOWS || PD_CYGWIN )
    #if PD_64BIT
        #define PD_PLUGIN       ".pdlibrary64"
    #else
        #define PD_PLUGIN       ".pdlibrary32"
    #endif
#else
    #define PD_PLUGIN           ".so"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_TRANSLATE(s)         (s)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS
#if PD_BUILDING_APPLICATION
    #define PD_DLL              __declspec(dllexport)
#else
    #define PD_DLL              __declspec(dllimport) 
#endif
#else
    #define PD_DLL              __attribute__((visibility ("default")))
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS
    #define PD_STUB             __declspec(dllexport)
#else
    #define PD_STUB             __attribute__((visibility ("default")))
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ! ( PD_BUILDING_APPLICATION )           /* Avoid namespace pollution. */

#include <stdlib.h>

#else 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
    
#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_APPLE

#include <mach-o/dyld.h>

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_BUILDING_APPLICATION

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_STRING               1024                /* Maximum size for a string. */
#define PD_ARGUMENTS            5                   /* Maximum number of typechecked arguments. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CLASS_DEFAULT           0
#define CLASS_ABSTRACT          1
#define CLASS_NOBOX             2
#define CLASS_GRAPHIC           3
#define CLASS_BOX               4

#define CLASS_NOINLET           8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_LLP64
    typedef long long           t_int;              /* A pointer-size integer (LLP64). */
#else
    typedef long                t_int;              /* Ditto (LP64 / ILP64). */
#endif

typedef float                   t_float;
typedef float                   t_sample;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef int                     t_error;
typedef int                     t_color;
typedef int                     t_fontsize;
typedef double                  t_systime;
typedef unsigned long           t_unique;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_LP64
    typedef unsigned int        t_keycode;          // uint32_t
#else
    typedef unsigned long       t_keycode;
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_ERROR                1
#define PD_ERROR_NONE           0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _array;
struct _boxtext;
struct _class;
struct _clock;
struct _dspcontext;
struct _environment;
struct _fielddescriptor;
struct _garray;
struct _gatom;
struct _glist;
struct _gmaster;
struct _gpointer;
struct _guiconnect;
struct _inlet;
struct _iterator;
struct _listinletelement;
struct _message;
struct _outconnect;
struct _outlet;
struct _painterwidgetbehavior;
struct _pdinstance;
struct _signal;
struct _struct;
struct _template;
struct _vinlet;
struct _voutlet;
struct _widgetbehavior;

#define t_array                     struct _array
#define t_boxtext                   struct _boxtext
#define t_class                     struct _class
#define t_clock                     struct _clock
#define t_dspcontext                struct _dspcontext
#define t_environment               struct _environment
#define t_fielddescriptor           struct _fielddescriptor
#define t_garray                    struct _garray
#define t_gatom                     struct _gatom
#define t_glist                     struct _glist
#define t_gmaster                   struct _gmaster
#define t_gpointer                  struct _gpointer
#define t_guiconnect                struct _guiconnect
#define t_inlet                     struct _inlet
#define t_iterator                  struct _iterator
#define t_listinletelement          struct _listinletelement
#define t_message                   struct _message
#define t_outconnect                struct _outconnect
#define t_outlet                    struct _outlet
#define t_painterwidgetbehavior     struct _painterwidgetbehavior
#define t_pdinstance                struct _pdinstance
#define t_signal                    struct _signal
#define t_struct                    struct _struct
#define t_template                  struct _template
#define t_vinlet                    struct _vinlet
#define t_voutlet                   struct _voutlet
#define t_widgetbehavior            struct _widgetbehavior

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _symbol {
    char            *s_name;
    t_class         **s_thing;
    struct _symbol  *s_next;
    } t_symbol;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _buffer;

typedef union word {
    t_float         w_float;
    int             w_index;
    t_symbol        *w_symbol;
    t_gpointer      *w_gpointer;
    t_array         *w_array;
    struct _buffer  *w_buffer;
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
    t_word          a_w;
    } t_atom;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _buffer {
    int             b_size;
    t_atom          *b_vector;
    } t_buffer;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_class *t_pd;

typedef struct _gobj {
    t_pd            g_pd;                       /* MUST be the first. */
    struct _gobj    *g_next;
    } t_gobj;

typedef struct _scalar {
    t_gobj          sc_g;                       /* MUST be the first. */
    t_symbol        *sc_templateIdentifier;
    t_word          *sc_vector;
    } t_scalar;

typedef struct _text {
    t_gobj          te_g;                       /* MUST be the first. */
    t_buffer        *te_buffer;
    t_inlet         *te_inlet;
    t_outlet        *te_outlet;
    int             te_xCoordinate;
    int             te_yCoordinate;
    int             te_width;                   /* Zero for undefined. */
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

#if ! ( PD_WITH_DEBUG )

#define PD_MEMORY_GET(n)                sys_getMemory (n)
#define PD_MEMORY_RESIZE(ptr, m, n)     sys_getMemoryResize ((ptr), (m), (n))
#define PD_MEMORY_FREE(ptr)             sys_freeMemory (ptr)

#else

#define PD_MEMORY_GET(n)                sys_getMemoryChecked (n, __FUNCTION__, __LINE__)
#define PD_MEMORY_RESIZE(ptr, m, n)     sys_getMemoryResizeChecked ((ptr), (m), (n), __FUNCTION__, __LINE__)
#define PD_MEMORY_FREE(ptr)             sys_freeMemoryChecked (ptr, __FUNCTION__, __LINE__);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define CLASS_SIGNAL(c, t, field)       class_addSignal (c, (char *)(&((t *)0)->field) - (char *)0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __cplusplus )

extern "C" {

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_symbol *gensym                         (const char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void     *sys_getMemory                  (size_t n);
PD_DLL void     *sys_getMemoryResize            (void *ptr, size_t oldSize, size_t newSize);

PD_DLL void     sys_freeMemory                  (void *ptr);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void     *sys_getMemoryChecked           (size_t n, const char *f, const int line);
PD_DLL void     *sys_getMemoryResizeChecked     (void *ptr,
                                                    size_t oldSize,
                                                    size_t newSize,
                                                    const char *f,
                                                    const int line);

PD_DLL void     sys_freeMemoryChecked           (void *ptr, const char *f, const int line);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_pd     *pd_new                         (t_class *c);

PD_DLL void     pd_free                         (t_pd *x);
PD_DLL void     pd_bind                         (t_pd *x, t_symbol *s);
PD_DLL void     pd_unbind                       (t_pd *x, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_class  *class_new                      (t_symbol *name,
                                                    t_newmethod newMethod,
                                                    t_method freeMethod,
                                                    size_t size,
                                                    int flags,
                                                    t_atomtype type1, ...);

PD_DLL void     class_addSignal                 (t_class *c, int offset);
PD_DLL void     class_addCreator                (t_newmethod newMethod, t_symbol *s, t_atomtype type1, ...);
PD_DLL void     class_addMethod                 (t_class *c, t_method fn, t_symbol *s, t_atomtype type1, ...);

PD_DLL void     class_addBang                   (t_class *c, t_method fn);
PD_DLL void     class_addFloat                  (t_class *c, t_method fn);
PD_DLL void     class_addSymbol                 (t_class *c, t_method fn);
PD_DLL void     class_addPointer                (t_class *c, t_method fn);
PD_DLL void     class_addList                   (t_class *c, t_method fn);
PD_DLL void     class_addAnything               (t_class *c, t_method fn);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_inlet  *inlet_newPointer               (t_object *owner, t_gpointer *gp);
PD_DLL t_inlet  *inlet_newFloat                 (t_object *owner, t_float *fp);
PD_DLL t_inlet  *inlet_newSymbol                (t_object *owner, t_symbol **sp);
PD_DLL t_inlet  *inlet_newSignal                (t_object *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_outlet *outlet_new                     (t_object *owner, t_symbol *s);

PD_DLL void     outlet_bang                     (t_outlet *x);
PD_DLL void     outlet_pointer                  (t_outlet *x, t_gpointer *gp);
PD_DLL void     outlet_float                    (t_outlet *x, t_float f);
PD_DLL void     outlet_symbol                   (t_outlet *x, t_symbol *s);
PD_DLL void     outlet_list                     (t_outlet *x, int argc, t_atom *argv);
PD_DLL void     outlet_anything                 (t_outlet *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_symbol *atom_getSymbol                 (t_atom *a);
PD_DLL t_symbol *atom_getSymbolAtIndex          (int n, int argc, t_atom *argv);

PD_DLL t_float  atom_getFloat                   (t_atom *a);
PD_DLL t_float  atom_getFloatAtIndex            (int n, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_buffer *buffer_new                     (void);
PD_DLL t_atom   *buffer_atoms                   (t_buffer *x);

PD_DLL void     buffer_free                     (t_buffer *x);
PD_DLL int      buffer_size                     (t_buffer *x);
PD_DLL void     buffer_reset                    (t_buffer *x);
PD_DLL void     buffer_append                   (t_buffer *x, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL t_clock  *clock_new                      (void *owner, t_method fn);

PD_DLL void     clock_free                      (t_clock *x);
PD_DLL void     clock_unset                     (t_clock *x);
PD_DLL void     clock_delay                     (t_clock *x, double delay);     /* Usable in DSP perform. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

PD_DLL void     post                            (const char *fmt, ...);
PD_DLL void     post_warning                    (const char *fmt, ...);
PD_DLL void     post_error                      (const char *fmt, ...);
PD_DLL void     post_log                        (const char *fmt, ...);
                                       
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( __cplusplus )

}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_pd_h_
