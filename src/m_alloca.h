
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_alloca_h_
#define __m_alloca_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_ALLOCA

    #include <alloca.h>
    
    #define ATOMS_ALLOCA(x, n)  \
        (x) = (t_atom *)((n) < 64 ? alloca ((n) * sizeof (t_atom)) : PD_MEMORY_GET ((n) * sizeof (t_atom)))
        
    #define ATOMS_FREEA(x, n)   \
        if (n >= 64) { PD_MEMORY_FREE ((x)); }
    
#else

    #define ATOMS_ALLOCA(x, n)  \
        (x) = (t_atom *)PD_MEMORY_GET ((n) * sizeof (t_atom))
        
    #define ATOMS_FREEA(x, n)   \
        PD_MEMORY_FREE ((x))

#endif // PD_WITH_ALLOCA

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_alloca_h_
