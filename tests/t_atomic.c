
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://github.com/mintomic/mintomic/tree/master/tests > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_int32Atomic    test_int32Shared;
static t_uint32Atomic   test_uInt32Shared;
static t_uint64Atomic   test_uInt64Shared;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *test_increment (void *x)
{
    int i, n = (int)(long)x;
    TTTWaste w;
    
    ttt_wasteInit (&w, n);
    
    if ((n % 2) == 0) {
    //
    for (i = 0; i < 1000000; i++) {
        PD_ATOMIC_INT32_INCREMENT (&test_int32Shared);
        // test_int32Shared++;
        ttt_wasteTime (&w);
    }
    //
    } else {
    //
    for (i = 0; i < 1000000; i++) {
        PD_ATOMIC_INT32_DECREMENT (&test_int32Shared);
        ttt_wasteTime (&w);
    }
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
void test10__increment() {    /* Increment / Decrement. */
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (AtomicIncrement, 10, "Atomic - Increment")

    if (ttt_testThreadsLaunch (test_increment) != TTT_GOOD) { TTT_FAIL; }
    else {
        int t = PD_ATOMIC_INT32_READ (&test_int32Shared); TTT_EXPECT (t == 0);
    }

TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int test_atomicFailed;

static uint32_t test_uInt32Limit = 0xffffff00U;
static uint64_t test_uInt64Limit = 0xffffff0000000000ULL;

/* Satisfy (x * x >= test_uInt32Limit). */

static uint32_t test_uInt32Values[8] =
    {
        0x37b91364U,
        0x1c5970efU,
        0x536c76baU,
        0x0a10207fU,
        0x71043c77U,
        0x4db84a83U,
        0x27cf0273U,
        0x74a15a69U
    };

/* Satisfy (x * x >= test_uInt64Limit). */

static uint64_t test_uInt64Values[8] =
    {
        0x3c116d2362a21633ULL,
        0x7747508552ab6bc6ULL,
        0x289a0e1528a43422ULL,
        0x15e36d0a61d326eaULL,
        0x3ccb2e8c0c6224c4ULL,
        0x074504c13a1716e1ULL,
        0x6c82417a3ad77b24ULL,
        0x3124440040454919ULL
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void *test_loadStore (void *x)
{
    int i, n = (int)(long)x;
    
    if ((n % 2) == 0) {
    
        t_rand48 seed;
        
        PD_RAND48_INIT (seed);
        
        for (i = 0; i < 1000000; i++) {
        //
        int k = (int)(PD_RAND48_DOUBLE (seed) * 8);
        
        PD_ATOMIC_INT32_WRITE  ((int32_t)test_uInt32Values[k], &test_int32Shared);
        PD_ATOMIC_UINT32_WRITE (test_uInt32Values[k], &test_uInt32Shared);
        PD_ATOMIC_UINT64_WRITE (test_uInt64Values[k], &test_uInt64Shared);
        // test_uInt64Shared = test_uInt64Values[k];
        
        PD_MEMORY_BARRIER;     /* Prevent hoisting the store out of the loop. */
        //
        }

    } else {
    
        for (i = 0; i < 1000000; i++) {
        //
        PD_MEMORY_BARRIER;
        
        int32_t  a = PD_ATOMIC_INT32_READ  (&test_int32Shared);
        uint32_t b = PD_ATOMIC_UINT32_READ (&test_uInt32Shared);
        uint64_t c = PD_ATOMIC_UINT64_READ (&test_uInt64Shared);
        
        if ((uint32_t)(a * a) < test_uInt32Limit) { test_atomicFailed = 1; }
        if ((uint32_t)(b * b) < test_uInt32Limit) { test_atomicFailed = 1; }
        if ((uint64_t)(c * c) < test_uInt64Limit) { test_atomicFailed = 1; }
        //
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
void test11__assignment() {    /* Read / Write. */
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (AtomicAssignment, 11, "Atomic - Assignment")

    test_int32Shared  = test_uInt32Values[0];
    test_uInt32Shared = test_uInt32Values[0];
    test_uInt64Shared = test_uInt64Values[0];

    if (ttt_testThreadsLaunch (test_loadStore) != TTT_GOOD) { TTT_FAIL; }
    else {
        TTT_EXPECT (test_atomicFailed == 0);
    }

TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
