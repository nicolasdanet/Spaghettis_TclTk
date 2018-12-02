
/* Copyright (c) 2013-2019 Nicolas Danet. */

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../Source/Belle.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "Belle_1.hpp"
#include "Belle_2.hpp"
#include "Belle_3.hpp"
#include "Belle_4.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int main()
{
    /* LLVM analyzer seems to report false positives. */
    
    #ifndef __clang_analyzer__
    
    belle::Music music;
    
    music.setOrigin (belle::Inches (0.3, 1.5));
    
    belle::Pdf ("Belle_1.pdf").paint (music.set (Example_1()));
    belle::Pdf ("Belle_2.pdf").paint (music.set (Example_2()));
    belle::Pdf ("Belle_3.pdf").paint (music.set (Example_3()));
    belle::Pdf ("Belle_4.pdf").paint (music.set (Example_4()));
    
    #endif
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
