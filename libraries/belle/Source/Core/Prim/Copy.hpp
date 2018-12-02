
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > static void ArrayToList (Array < T > & a, List < T > & b, bool reversed = false)
{
    List < T > scoped;
    
    if (reversed) { for (int i = a.size() - 1; i >= 0; --i) { scoped.add (a[i]); } }
    else {
        for (int i = 0; i < a.size(); ++i) { scoped.add (a[i]); }
    }
    
    b.swapWith (scoped);
}

template < class T > static void ListToArray (List < T > & a, Array < T > & b, bool reversed = false)
{
    Array < T > scoped;
    
    scoped.resize (a.size());
    
    if (reversed) { for (int i = 0, k = a.size() - 1; i < a.size(); ++i) { scoped[k - i] = a[i]; } }
    else {
        for (int i = 0; i < a.size(); ++i) { scoped[i] = a[i]; }
    }
    
    b.swapWith (scoped);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
