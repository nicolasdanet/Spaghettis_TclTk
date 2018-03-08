
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2018 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef BELLE_LIBRARY
#define BELLE_LIBRARY

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* This library is a fork of "Belle, Bonne, Sage" from William Andrew Burnson. */
/* Notice that all files have been largely modified. */
/* Original version has been extended since and can be found at the link below. */

/* < https://github.com/burnson/belle > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_WITH_TEST
#define PRIM_WITH_TEST      0       /* Enable memory leaks detector and assertions. */
#endif

#ifndef PRIM_WITH_BELLE
#define PRIM_WITH_BELLE     1       /* Unset it to use MICA only. */
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_LIBRARY
    #ifdef BELLE_COMPILE_INLINE
        #define PRIM_COMPILE_INLINE
    #endif
    #include "Core/Prim/Prim.hpp"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifndef MICA_LIBRARY
    #ifdef BELLE_COMPILE_INLINE
        #include "Core/MICA/Mica.cpp"
    #endif
    #include "Core/MICA/Mica.hpp"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_WITH_BELLE

    #include "Core/Core.hpp"
    #include "Fonts/Fonts.hpp"
    #include "Symbols/Symbols.hpp"
    #include "Modern/Modern.hpp"
    #include "Painters/Painters.hpp"

#endif // PRIM_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // BELLE_LIBRARY
