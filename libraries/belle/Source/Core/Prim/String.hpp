
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2018 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Do NOT manage UTF-8 (only ASCII). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Consider to use std::string instead? */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Below, the magic number 4 is sizeof (uint32). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class String {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

friend class Iterator;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

private:
    class Iterator {

    public:
        Iterator()
        {
            reset();
        }
    
        Iterator (const Iterator&)
        {   
            reset();
        }
    
        Iterator& operator = (const Iterator&)
        {
            reset(); return *this;
        }
    
        #if PRIM_CPP11
        
        Iterator (Iterator&&) noexcept
        {
            reset();
        }
        
        Iterator& operator = (Iterator&&)
        {
            reset(); return *this;
        }
        
        #endif
        
    public:
        int next (const String* s)
        {
            if (f_ != 1 && holdOnce_) { holdOnce_ = false; return -1; }
            
            if (f_ == 1) {
                i_ = 0;
                f_ = s->readMarker (1);
                n_ = s->readMarker (f_);
                return f_;
                
            } else {
                i_ += n_;
                p_ = f_ + 4 + n_;
                f_ = s->readMarker (p_);
                if (!f_) { n_ = 0; }
                else { 
                    n_ = s->readMarker (f_); 
                }
                return f_;
            }
        }
      
        void reset()
        {
            i_ = 0;
            f_ = 1;
            n_ = 0;
            p_ = 1;
            holdOnce_ = false;
        }
    
    public:
        int i_;             /* No getters and setters for convenience. */
        int f_;
        int n_;
        int p_;
        bool holdOnce_;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -


public:
    String() 
    { 
        clear(); 
    }
    
    String (const ascii* s) 
    {
        clear(); append (s);
    }
    
    String & operator = (const String& o)
    {
        if (this != &o) {
            String scoped (o); scoped.swapWith (*this);
        }
        
        return *this;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    String (const String&) = default;
    String (String&&) = default;
    String& operator = (String&&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void swapWith (String& o) noexcept
    {
        using std::swap;
        
        data_.swapWith (o.data_);
        
        swap (length_, o.length_);
        swap (last_, o.last_);
        
        iter_.reset(); o.iter_.reset();
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

    void clear()
    {
        iter_.reset();
        data_.resize (last_ = length_ = 0);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    int length() const 
    { 
        return length_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    ascii get (int i) const
    {
        if (i >= 0 && i < length_) {
            if (i < iter_.i_) { iter_.reset(); }
            iter_.holdOnce_ = true;
            while (iter_.next (this)) {
                if (i < iter_.i_ + iter_.n_) { return data_[iter_.f_ + 4 + (i - iter_.i_)]; }
            }
        }
        
        return 0;
    }
    
    ascii operator[] (int i) const
    {   
        return get (i);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    String& append (const ascii* s) 
    {
        appendFragment (s, lengthOf (s)); return *this;
    }
    
    String& prepend (const ascii* s) 
    {
        prependFragment (s, lengthOf (s)); return *this;
    }
    
    String& insert (const ascii* s, int before)
    {
        insertFragment (s, lengthOf (s), before); return *this;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    int fragments() const
    {
        int count = 0;
        
        if (length_) { Iterator q; while (q.next (this)) { count++; } }
        
        return count;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    const ascii* merge() const
    {
        if (length_ == 0) { return &String::empty_; } 
        if (data_.size() == 1 + 4 + 4 + length_ + 4) { return &data_[1 + 4 + 4]; }
        
        iter_.reset();
                
        Array < ascii > scoped;
        
        scoped.resize (1 + 4 + 4 + length_ + 4);
        scoped[0] = 0;
      
        uint32 f = 1 + 4; 
        uint32 n = static_cast < uint32 > (length_);
        uint32 z = 0;
        
        memcpy (&scoped[1], &f, sizeof (f)); 
        memcpy (&scoped[f], &n, sizeof (n));
        memcpy (&scoped[1 + 4 + 4 + length_], &z, sizeof (z));

        ascii* s = &scoped[1 + 4 + 4];
        
        Iterator q;
        while (q.next (this)) { Memory::copy (&s[q.i_], &data_[q.f_ + 4], q.n_); }

        data_.swapWith (scoped);
        last_ = 1 + 4;
        
        return s;
    }
    
    const ascii* toCString() const 
    {
        return merge();
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    bool operator < (const String& o) const
    {
        return strcmp (merge(), o.merge()) < 0;
    }

    bool operator > (const String& o) const
    {
        return strcmp (merge(), o.merge()) > 0;
    }
    
    bool operator == (const String& o) const
    {
        if (length_ != o.length_) { return false; } else { return strcmp (merge(), o.merge()) == 0; }
    }
    
    bool operator != (const String& o) const
    {
        return !((*this) == o);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    template < class T > String& operator << (T v)
    {
        std::stringstream ss;
        ss.setf (std::ios::fixed, std::ios::floatfield);
        ss << v;
        char buffer[32]; 
        ss.read (&buffer[0], 32);
        appendFragment (buffer, static_cast < int > (ss.gcount()));
        return *this;
    }

    String& operator << (const ascii* s)
    {
        append (s);
        return *this;
    }
    
    String& operator << (const String& s)
    {
        appendFragment (s.merge(), s.length());
        return *this;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    bool isInteger() const
    {
        char* p;
        strtoll (merge(), &p, 0);
        return (*p) == 0;
    }
    
    bool isFloat() const
    {
        char* p;
        strtod (merge(), &p);
        return (*p) == 0;
    }
    
    bool isBoolean() const
    {
        if ((*this) == String ("False") || (*this) == String ("True")) { return true; }
        else {
            return false;
        }
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    int64 getInteger() const
    {
        return strtoll (merge(), nullptr, 0);
    }
    
    double getFloat() const
    {
        return strtod (merge(), nullptr);
    }
    
    bool getBoolean() const
    {
        if ((*this) == String ("True")) { return true; }
        else {
            return false;
        }
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    template < class T > static String paddedLeft (T v, int n, ascii c = ' ')
    {
        String raw, padded; 
        raw << v;
        for (int i = 0; i < Math::max (n - raw.length(), 0); ++i) { padded << c; }
        padded << raw;
        return padded;
    }

public:
    static String asHex (uint32 u)
    {
        return toHexString (u);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    template < class T > static String toHexString (T v)
    {
        static const char hex[] = "0123456789abcdef";
        const int size = 32;
        
        char buffer[size] = { 0 };
        int i = size - 1;

        do {
        //
        buffer[--i] = hex[static_cast < size_t > (v & 15)];
        v >>= 4;
        //
        } while (v != 0);
        
        return String (buffer + i);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    int readMarker (int i) const
    {
        uint32 t; 
        memcpy (&t, &data_[i], sizeof (t));
        return static_cast < int > (t);
    }
    
    void writeMarker (int i, int v)
    {
        uint32 t = static_cast < uint32 > (v);
        memcpy (&data_[i], &t, sizeof (t));
    }

    void writeFragment (int start, int size, const ascii* fragment, int next)
    {
        writeMarker (start, size);
        writeMarker (start + 4 + size, next);
        Memory::copy (&data_[start + 4], fragment, size);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    void appendFragment (const ascii* fragment, int size)
    {
        if (size <= 0) { return; }
        
        iter_.reset();
                
        if (length_ == 0) {
            data_.resize (1 + 4 + 4 + size + 4);
            data_[0] = 0;
            writeMarker (1, 5);
            writeFragment (5, size, fragment, 0);
            last_ = 5;

        } else if (last_ + 4 + readMarker (last_) + 4 == data_.size()) {
            data_.resize (data_.size() + size);
            int old = readMarker (last_);
            writeMarker (last_, old + size);
            Memory::copy (&data_[last_ + 4 + old], fragment, size);
            writeMarker (last_ + 4 + old + size, 0);

        } else {
            int old = data_.size();
            data_.resize (old + 4 + size + 4);
            writeMarker (last_ + 4 + readMarker (last_), old);
            writeFragment (old, size, fragment, 0);
            last_ = old;
        }
      
        length_ += size;
    }

    void prependFragment (const ascii* fragment, int size)
    {
        if (size <= 0) { return; }    
        if (length_ == 0) { appendFragment (fragment, size); return; }
        
        iter_.reset();
        
        int old = data_.size();
        int first = readMarker (1);
        
        data_.resize (old + 4 + size + 4);
        writeFragment (old, size, fragment, first);
        writeMarker (1, old);
        
        length_ += size;
    }
    
    void insertFragment (const ascii* fragment, int size, int before)
    {
        if (size <= 0)   { return; }
        if (before <= 0) { return prependFragment (fragment, size); }
        if (length_ == 0 || before >= length_) { return appendFragment (fragment, size); }
        
        const ascii* a = merge();
        
        Array < ascii > scoped;
        
        scoped.resize (1 + 4 + 4 + length_ + size + 4);
        scoped[0] = 0; 
        
        uint32 f = 1 + 4; 
        uint32 n = static_cast < uint32 > (length_ + size);
        uint32 z = 0;
        
        memcpy (&scoped[1], &f, sizeof (f)); 
        memcpy (&scoped[f], &n, sizeof (n));
        memcpy (&scoped[1 + 4 + 4 + length_ + size], &z, sizeof (z));
        
        ascii* b = &scoped[1 + 4 + 4];

        Memory::copy (&b[0], a, before);
        Memory::copy (&b[before], fragment, size);
        Memory::copy (&b[before + size], a + before, length_ - before);
        
        data_.swapWith (scoped);
        length_ += size;
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    static int lengthOf (const ascii* s)
    {
        if (!s) { return 0; }
        else {
            return static_cast < int > (strchr (s, 0) - s);
        }
    }

private:
    static const ascii empty_;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

private:
    mutable Array < ascii > data_;
    mutable int length_;
    mutable int last_;
    mutable Iterator iter_;

private:
    PRIM_LEAK_DETECTOR (String)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void swap (String& a, String& b) noexcept;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

const ascii String::empty_ = 0;   /* The empty null-terminated string. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void swap (String& a, String& b) noexcept
{
    a.swapWith (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // BELLE_COMPILE_INLINE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
