
/* Copyright (c) 2017 Nicolas Danet. */

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef MICA_MIR_TUPLE_HPP
#define MICA_MIR_TUPLE_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace MIR {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < int N > class Tuple {        /* Concepts grouped in a basic tuple. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Tuple()
    {
    }
    
    template < class T > Tuple (const prim::Array < T > & a, int offset = 0) 
    { 
        int k = 0;
        for (int i = offset; i < offset + N; ++i) { if (i < a.size()) { tuple_[k++] = Concept (a[i]); } }
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    Tuple (const Tuple < N > &) = default;
    Tuple (Tuple < N > &&) = default;
    Tuple < N > & operator = (const Tuple < N > &) = default;
    Tuple < N > & operator = (Tuple < N > &&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Make the Tuple class sortable. */

public:
    bool operator < (const Tuple& o) const
    {
        for (int i = 0; i < N; ++i) {
            if (tuple_[i] < o.tuple_[i])      { return true;  }
            else if (tuple_[i] > o.tuple_[i]) { return false; }
        }
        
        return false;
    }
  
    bool operator > (const Tuple& o) const
    {
        for (int i = 0; i < N; ++i) {
            if (tuple_[i] > o.tuple_[i])      { return true;  }
            else if (tuple_[i] < o.tuple_[i]) { return false; }
        }
        
        return false;
    }
    
    bool operator == (const Tuple& o) const
    {
        for (int i = 0; i < N; ++i) { if (tuple_[i] != o.tuple_[i]) { return false; } }
        
        return true;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Make the Tuple class hashable. */

public:
    std::size_t hash() const
    {
        Concept hashed = tuple_[0];
    
        for (int i = 1; i < N; ++i) { hashed = UUID::merge (hashed, tuple_[i]); }
        
        return hashed.hash();
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Concept& operator[] (int i)
    {
        PRIM_ASSERT (i < N); return tuple_[i];
    }
    
    const Concept& operator[] (int i) const
    {   
        PRIM_ASSERT (i < N); return tuple_[i];
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

public:
    Concept (&getRaw())[N]      /* < http://stackoverflow.com/a/5399014 > */
    {
        return tuple_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Use C++11 variadic templates (or even std::tuplet for all that class)? */

public:
    template < class T > Tuple (const T& a)
    {
        PRIM_STATIC_ASSERT (N == 1);
        
        tuple_[0] = Concept (a);
    }
    
    template < class T > Tuple (const T& a, const T& b)
    {
        PRIM_STATIC_ASSERT (N == 2);
        
        tuple_[0] = Concept (a);
        tuple_[1] = Concept (b);
    }
    
    template < class T > Tuple (const T& a, const T& b, const T& c)
    {
        PRIM_STATIC_ASSERT (N == 3);
        
        tuple_[0] = Concept (a);
        tuple_[1] = Concept (b);
        tuple_[2] = Concept (c);
    }
    
    template < class T > Tuple (const T& a, const T& b, const T& c, const T& d)
    {
        PRIM_STATIC_ASSERT (N == 4);
        
        tuple_[0] = Concept (a);
        tuple_[1] = Concept (b);
        tuple_[2] = Concept (c);
        tuple_[3] = Concept (d);
    }
    
    template < class T > Tuple (const T& a, const T& b, const T& c, const T& d, const T& e)
    {
        PRIM_STATIC_ASSERT (N == 5);
        
        tuple_[0] = Concept (a);
        tuple_[1] = Concept (b);
        tuple_[2] = Concept (c);
        tuple_[3] = Concept (d);
        tuple_[4] = Concept (e);
    }

private:
    Concept tuple_[N];
    
private:
    PRIM_LEAK_DETECTOR (Tuple)
};
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace MIR

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // MICA_MIR_TUPLE_HPP
