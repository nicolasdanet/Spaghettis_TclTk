
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_macros_h_
#define __m_macros_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_SHORT_FILE       (strrchr (__FILE__, '/') ? strrchr (__FILE__, '/') + 1 : __FILE__)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_DEBUG

    #define PD_BUG          PD_ASSERT (0)
    #define PD_ASSERT(x)    if (!(x)) { post_log ("*** Assert / %s / line %d", PD_SHORT_FILE, __LINE__); }

#else
    
    #define PD_BUG
    #define PD_ASSERT(x)    ((void)(x))

#endif // PD_WITH_DEBUG

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_ABORT(x)         if (x) { abort(); }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_MAX(a,b)         ((a)>(b)?(a):(b))
#define PD_MIN(a,b)         ((a)<(b)?(a):(b))

#define PD_ABS(a)           ((a)<0?-(a):(a))
#define PD_CLAMP(u,a,b)     ((u)<(a)?(a):(u)>(b)?(b):(u))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_INT_MAX          0x7fffffff
#define PD_FLT_MIN          FLT_MIN
#define PD_FLT_MAX          FLT_MAX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define pd_class(x)         (*((t_pd *)(x)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define cast_pd(x)          ((t_pd *)(x))
#define cast_iem(x)         ((t_iem *)(x))
#define cast_gobj(x)        ((t_gobj *)(x))
#define cast_glist(x)       ((t_glist *)(x))
#define cast_scalar(x)      ((t_scalar *)(x))
#define cast_object(x)      ((t_object *)(x))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define cast_objectIfPatchable(x)           (pd_class (x)->c_isBox ? (t_object *)(x) : NULL)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist *cast_glistChecked                  (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SET_NULL(atom)                      ((atom)->a_type = A_NULL)
#define SET_SEMICOLON(atom)                 ((atom)->a_type = A_SEMICOLON, (atom)->a_w.w_index = 0)
#define SET_COMMA(atom)                     ((atom)->a_type = A_COMMA, (atom)->a_w.w_index = 0)
#define SET_POINTER(atom, gp)               ((atom)->a_type = A_POINTER, (atom)->a_w.w_gpointer = (gp))
#define SET_FLOAT(atom, f)                  ((atom)->a_type = A_FLOAT, (atom)->a_w.w_float = (f))
#define SET_SYMBOL(atom, s)                 ((atom)->a_type = A_SYMBOL, (atom)->a_w.w_symbol = (s))
#define SET_DOLLAR(atom, n)                 ((atom)->a_type = A_DOLLAR, (atom)->a_w.w_index = (n))
#define SET_DOLLARSYMBOL(atom, s)           ((atom)->a_type = A_DOLLARSYMBOL, (atom)->a_w.w_symbol= (s))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GET_POINTER(atom)                   ((atom)->a_w.w_gpointer)
#define GET_FLOAT(atom)                     ((atom)->a_w.w_float)
#define GET_SYMBOL(atom)                    ((atom)->a_w.w_symbol)
#define GET_DOLLAR(atom)                    ((atom)->a_w.w_index)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define ADDRESS_FLOAT(atom)                 &((atom)->a_w.w_float)
#define ADDRESS_SYMBOL(atom)                &((atom)->a_w.w_symbol)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IS_NULL(atom)                       ((atom)->a_type == A_NULL)
#define IS_SEMICOLON(atom)                  ((atom)->a_type == A_SEMICOLON)
#define IS_COMMA(atom)                      ((atom)->a_type == A_COMMA)
#define IS_POINTER(atom)                    ((atom)->a_type == A_POINTER)
#define IS_FLOAT(atom)                      ((atom)->a_type == A_FLOAT)
#define IS_SYMBOL(atom)                     ((atom)->a_type == A_SYMBOL)
#define IS_DOLLAR(atom)                     ((atom)->a_type == A_DOLLAR)
#define IS_DOLLARSYMBOL(atom)               ((atom)->a_type == A_DOLLARSYMBOL)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IS_SYMBOL_OR_FLOAT(atom)            (IS_SYMBOL(atom) || IS_FLOAT(atom))
#define IS_SEMICOLON_OR_COMMA(atom)         (IS_SEMICOLON(atom) || IS_COMMA(atom))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_gotfn1)(void *x, void *arg1);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define mess1(x, s, a)                      ((*(t_gotfn1)class_getMethod (*(x), (s)))((x), (a)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define class_addDSP(c, m)                  class_addMethod ((c), (t_method)(m), \
                                                sym_dsp, \
                                                A_CANT, \
                                                A_NULL);
                                                
#define class_addKey(c, m)                  class_addMethod ((c), (t_method)(m), \
                                                sym_key, \
                                                A_GIMME, \
                                                A_NULL);

#define class_addMouse(c, m)                class_addMethod ((c), (t_method)(m), \
                                                sym_mouse, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_NULL);
                                                
#define class_addMouseUp(c, m)              class_addMethod ((c), (t_method)(m), \
                                                sym_mouseup, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_NULL);
    
#define class_addClick(c, m)                class_addMethod ((c), (t_method)(m), \
                                                sym_click, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_NULL);

#define class_addMotion(c, m)               class_addMethod ((c), (t_method)(m), \
                                                sym_motion, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_FLOAT, \
                                                A_NULL);

#define class_addPolling(c, m)              class_addMethod ((c), (t_method)(m), \
                                                sym__polling, \
                                                A_NULL);

#define class_addAutorelease(c, m)          class_addMethod ((c), (t_method)(m), \
                                                sym__autorelease, \
                                                A_NULL);
                                                
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_PI                               3.14159
#define PD_LOGTEN                           2.302585092994
#define PD_EPSILON                          1E-9

#define PD_ISPOWER2(v)                      (!((v) & ((v) - 1)))
#define PD_NEXTPOWER2(v)                    sys_nextPowerOf2 ((unsigned long)(v))
#define PD_TORADIANS(degrees)               ((PD_PI * (degrees)) / 180.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Returns the next power of two (zero on overflow). */

static inline unsigned long sys_nextPowerOf2 (unsigned long v) 
{
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
    v |= (v >> 32);
    
    return v + 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SECONDS_TO_MILLISECONDS(n)          ((n) * (double)1000.0)
#define MILLISECONDS_TO_SECONDS(n)          ((n) * (double)1e-3)
#define SECONDS_TO_MICROSECONDS(n)          ((n) * (double)1000000.0)
#define MICROSECONDS_TO_SECONDS(n)          ((n) * (double)1e-6)
#define MILLISECONDS_TO_MICROSECONDS(n)     ((n) * (double)1000.0)
#define MICROSECONDS_TO_MILLISECONDS(n)     ((n) * (double)1e-3)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef union {
    t_float     z_f;
    uint32_t    z_i;
    } t_rawcast;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ! ( PD_MSVC )

    #if ( PD_CPU_x86 || PD_CPU_AMD64 || PD_CPU_ARM )
        
        static inline int PD_DENORMAL_OR_ZERO (t_float f)   /* Is malformed (denormal, infinite, NaN)? */
        {
            t_rawcast z;
            z.z_f = f;
            z.z_i &= 0x7f800000;
            return ((z.z_i == 0) | (z.z_i == 0x7f800000));
        }
        
        static inline int PD_BIG_OR_SMALL (t_float f)       /* If exponent falls out (-64, 64) range. */
        {
            t_rawcast z;
            z.z_f = f;
            return ((z.z_i & 0x20000000) == ((z.z_i >> 1) & 0x20000000)); 
        }

    #else
    
        #error
        
        #define PD_DENORMAL_OR_ZERO(f)  0
        #define PD_BIG_OR_SMALL(f)      0
        
    #endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

        #define PD_DENORMAL_OR_ZERO(f)  ((((*(unsigned int*)&(f)) & 0x7f800000) == 0) || \
                                            (((*(unsigned int*)&(f)) & 0x7f800000) == 0x7f800000))
    
        #define PD_BIG_OR_SMALL(f)      ((((*(unsigned int*)&(f)) & 0x60000000) == 0) || \
                                            (((*(unsigned int*)&(f)) & 0x60000000) == 0x60000000))

#endif // PD_MSVC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_macros_h_
