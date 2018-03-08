
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2018 Nicolas Danet.
    
*/  

/* < http://opensource.org/licenses/BSD-2-Clause > */ 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

template < class T > class LeakDetector {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    class LeakCounter {
    
    public:
        LeakCounter() : numberOfInstance_ (0)
        {
        }
        
        ~LeakCounter()
        {
            if (numberOfInstance_ != 0) { Assert::debugLeak (getClassName(), numberOfInstance_); }
        }
    
    #if PRIM_CPP11

    public:
        LeakCounter (const LeakCounter&) = delete;
        LeakCounter& operator = (const LeakCounter&) = delete;
        
    #endif
    
    public:
        int numberOfInstance_;      /* Public for convenience. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    LeakDetector()
    {
        ++(getCounter().numberOfInstance_);
    }
    
    LeakDetector (const LeakDetector&)
    { 
        ++(getCounter().numberOfInstance_);
    }

#if PRIM_CPP11

public:
    LeakDetector (LeakDetector&&)
    {
        ++(getCounter().numberOfInstance_);
    }
    
public:
    LeakDetector& operator = (const LeakDetector&) = default;
    LeakDetector& operator = (LeakDetector&&) = default;
        
#endif
    
    ~LeakDetector()
    {
        --(getCounter().numberOfInstance_);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    static const char* getClassName()
    {
        return T::getClassName();
    }

    static LeakCounter& getCounter()
    {
        static LeakCounter counter; return counter;
    }
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if BELLE_WITH_TEST
    #define PRIM_LEAK_DETECTOR(Owner) \
        friend class prim::LeakDetector < Owner >; \
        static const char* getClassName() { return #Owner; } \
        prim::LeakDetector < Owner > leakDetector ## __LINE__;
#else
    #define PRIM_LEAK_DETECTOR(Owner)
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
