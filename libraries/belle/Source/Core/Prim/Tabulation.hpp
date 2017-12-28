
/* 
    Copyright 2007-2013 William Andrew Burnson. All rights reserved.

    File modified by Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_TABULATION_HPP
#define PRIM_TABULATION_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < int N > class Tabulation {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Tabulation()
    {
        std::memset (spaces_, ' ', N); spaces_[N] = 0;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    operator const ascii*()
    {
        return spaces_;
    }
    
    String operator << (const String& s) const
    {
        String t (spaces_); return t << s;
    }

private:
    char spaces_[N + 1];
};

extern Tabulation < 4 > Tab;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef PRIM_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

Tabulation < 4 > Tab;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PRIM_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PRIM_TABULATION_HPP