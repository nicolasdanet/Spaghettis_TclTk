
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

#define PD_BUG
#define PD_ASSERT(x)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define pd_class(x)                 (*(x))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SETSEMICOLON(atom)          ((atom)->a_type = A_SEMICOLON, (atom)->a_w.w_index = 0)
#define SETCOMMA(atom)              ((atom)->a_type = A_COMMA, (atom)->a_w.w_index = 0)
#define SETPOINTER(atom, gp)        ((atom)->a_type = A_POINTER, (atom)->a_w.w_gpointer = (gp))
#define SETFLOAT(atom, f)           ((atom)->a_type = A_FLOAT, (atom)->a_w.w_float = (f))
#define SETSYMBOL(atom, s)          ((atom)->a_type = A_SYMBOL, (atom)->a_w.w_symbol = (s))
#define SETDOLLAR(atom, n)          ((atom)->a_type = A_DOLLAR, (atom)->a_w.w_index = (n))
#define SETDOLLARSYMBOL(atom, s)    ((atom)->a_type = A_DOLLARSYMBOL, (atom)->a_w.w_symbol= (s))

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
#pragma mark -

#ifndef _MSC_VER

    #if defined ( __i386__ ) || defined ( __x86_64__ ) || defined ( __arm__ )

        typedef union {
            t_float f;
            unsigned int ui;
        } t_bigorsmall32;

        static inline int PD_DENORMAL_OR_ZERO (t_float f)   /* Is malformed (denormal, infinite, NaN)? */
        {
            t_bigorsmall32 pun;
            pun.f = f;
            pun.ui &= 0x7f800000;
            return ((pun.ui == 0) | (pun.ui == 0x7f800000));
        }
        
        static inline int PD_BIG_OR_SMALL (t_float f)       /* If exponent falls out (-64, 64) range. */
        {
            t_bigorsmall32 pun;
            pun.f = f;
            return ((pun.ui & 0x20000000) == ((pun.ui >> 1) & 0x20000000)); 
        }

    #else
    
        #define PD_DENORMAL_OR_ZERO(f)  0
        #define PD_BIG_OR_SMALL(f)      0
        
    #endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else   // _MSC_VER

        #define PD_DENORMAL_OR_ZERO(f)  ((((*(unsigned int*)&(f)) & 0x7f800000) == 0) || \
                                            (((*(unsigned int*)&(f)) & 0x7f800000) == 0x7f800000))
    
        #define PD_BIG_OR_SMALL(f)      ((((*(unsigned int*)&(f)) & 0x60000000) == 0) || \
                                            (((*(unsigned int*)&(f)) & 0x60000000) == 0x60000000))

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_macros_h_
