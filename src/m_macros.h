
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

#ifndef _MSC_VER

    #if defined ( __i386__ ) || defined ( __x86_64__ ) || defined ( __arm__ )

        typedef union {
            t_float f;
            unsigned int ui;
        } t_bigorsmall32;

        static inline int PD_BADFLOAT (t_float f)       /* Malformed float. */
        {
            t_bigorsmall32 pun;
            pun.f = f;
            pun.ui &= 0x7f800000;
            return ((pun.ui == 0) | (pun.ui == 0x7f800000));
        }

        static inline int PD_BIGORSMALL (t_float f)     /* Exponent underflow or overflow. */
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
#endif // __m_macros_h_
