
/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TEST_CLOCKS_SIZE    32
#define TEST_CLOCKS_LOOP    100000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_int32Atomic        test_clocksWait;
static t_int32Atomic        test_clocksStop;
static int                  test_clocksCounter;
static t_float64Atomic      test_clocksSystime;
static int                  test_clocksFails;
static t_clock              test_clocksA[TEST_CLOCKS_SIZE];
static t_clock              test_clocksB[TEST_CLOCKS_SIZE];

static t_clocks             *test_clocksManager;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Mimic the behavior of application clocks. */

void test_clocksUnset (t_clock *x)
{
    clocks_remove (test_clocksManager, x);
}

void test_clocksDelay (t_clock *x, double delay)
{
    test_clocksUnset (x);
    
    PD_ATOMIC_FLOAT64_WRITE (delay, &x->c_systime);
    
    clocks_add (test_clocksManager, x);
}

void test_clocksTaskA (void *x)
{
    t_systime t = scheduler_getLogicalTime();
    
    test_clocksFails |= (t < PD_ATOMIC_FLOAT64_READ (&test_clocksSystime));
    
    PD_ATOMIC_FLOAT64_WRITE (t, &test_clocksSystime);
}

void test_clocksTaskB (void *x)
{
    test_clocksCounter++;
}

void test_clocksTick (t_systime t)
{
    clocks_tick (test_clocksManager, t);
}

int test_clocksClean (void)
{
    return clocks_clean (test_clocksManager);
}

void test_clocksInitialize (void)
{
    int i;
    
    for (i = 0; i < TEST_CLOCKS_SIZE; i++) {
    //
    t_clock *c = NULL;
    
    c = &test_clocksA[i]; c->c_fn = test_clocksTaskA;
    c = &test_clocksB[i]; c->c_fn = test_clocksTaskB;
    //
    }
    
    test_clocksManager = clocks_new();
}

void test_clocksRelease (void)
{
    clocks_free (test_clocksManager);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void test_clocksSynchronize (void)
{
    PD_ATOMIC_INT32_INCREMENT (&test_clocksWait);
    
    while (PD_ATOMIC_INT32_READ (&test_clocksWait) < ttt_testThreadsNumber()) { }
}

void test_clocksDebug (int n)
{
    ttt_stdout (TTT_COLOR_BLUE, "%s", clocks_debug (test_clocksManager, n));
}

int test_clocksRandom (int n)
{
    static t_rand48 seed; static int once = 0; if (!once) { PD_RAND48_INIT (seed); once = 1; }
    
    int k = (int)(PD_RAND48_DOUBLE (seed) * n);
    
    return k;
}

void test_clocksDoSomething (t_clock *x, double delay)
{
    if (test_clocksRandom (2)) { test_clocksDelay (x, delay); } else { test_clocksUnset (x); }
}

t_clock *test_clocksGetRandomA (void)
{
    int i = test_clocksRandom (TEST_CLOCKS_SIZE); return &test_clocksA[i];
}

t_clock *test_clocksGetB (int i)
{
    return &test_clocksB[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *test_clocksAtomicTask (void *x)
{
    int n = (int)(long)x;
    int j, i = 0;
    
    TTTWaste w;
    
    ttt_wasteInit (&w, n);
    
    test_clocksSynchronize();
    
    if (n == 0) {
        
        while (PD_ATOMIC_INT32_READ (&test_clocksStop) == 0) {
            for (j = 0; j < TEST_CLOCKS_SIZE; j++) {
                test_clocksDoSomething (test_clocksGetRandomA(), test_clocksRandom (1000));
                ttt_wasteTime (&w);
            }
            
            if (++i % 10 == 0) { test_clocksClean(); }
        }
    }
    
    if (n == 1) {
    
        for (i = 0; i < TEST_CLOCKS_LOOP; i++) {
            for (j = 0; j < TEST_CLOCKS_SIZE; j++) {
                test_clocksDelay (test_clocksGetB (j), test_clocksRandom (500));
                ttt_wasteTime (&w);
            }
            
            PD_ATOMIC_FLOAT64_WRITE (0, &test_clocksSystime); test_clocksTick (250.0);
            PD_ATOMIC_FLOAT64_WRITE (0, &test_clocksSystime); test_clocksTick (750.0);
        }
        
        PD_ATOMIC_INT32_WRITE (1, &test_clocksStop);
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
void test16__atomic() {
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (ClocksAtomic, 16, "Atomic - Clocks")

    if (ttt_testThreadsNumber() >= 2) {
    //
    test_clocksInitialize();
    
    if (ttt_testThreadsLaunch (test_clocksAtomicTask) != TTT_GOOD) { TTT_FAIL; }
    else {
    //
    TTT_EXPECT (test_clocksCounter == TEST_CLOCKS_SIZE * TEST_CLOCKS_LOOP);
    TTT_EXPECT (test_clocksFails   == 0);
    //
    }
    
    test_clocksRelease();
    //
    }
    
TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
