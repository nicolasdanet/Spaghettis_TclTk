
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2018 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_MEMORY_HPP
#define PRIM_MEMORY_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct Memory {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Assume no aliasing for memory operations. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

template < class T > static void clear (T* a, int items)
{
    memset (static_cast < void* > (a), 0, sizeof (T) * items);
}

template < class T > static void copy (T* destination, const T* source, int items = 1)
{
    memcpy (static_cast < void* > (destination), static_cast < const void* > (source), sizeof (T) * items);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > static void swap (T* a, T* b, int items)
{
    if (a != b) {
    //
    const int bufferSize = 1024;
    
    byte buffer[bufferSize];
    byte* ptrA = reinterpret_cast < byte* > (a);
    byte* ptrB = reinterpret_cast < byte* > (b);
    int size = static_cast < int > (sizeof (T) * items);
    
    while (size > 0) {
    //
    int n = size < bufferSize ? size : bufferSize;
    copy (buffer, ptrA, n);
    copy (ptrA, ptrB, n);
    copy (ptrB, buffer, n);
    ptrA += n;
    ptrB += n;
    size -= n;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PRIM_MEMORY_HPP
