
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

static LARGE_INTEGER    time_NTTime;            /* Static. */
static double           time_NTFrequency;       /* Static. */

static void time_initializeClock (void)
{
    LARGE_INTEGER f1;
    LARGE_INTEGER now;
    
    QueryPerformanceCounter (&now);
    
    if (!QueryPerformanceFrequency (&f1)) { PD_BUG; f1.QuadPart = 1; }
    
    time_NTTime = now;
    time_NTFrequency = f1.QuadPart;
}

/*
double clock_getRealTimeInSeconds (void)
{
    LARGE_INTEGER now;
 
    QueryPerformanceCounter (&now);
 
    if (time_NTFrequency == 0) { time_initializeClock(); }
 
    return (((double)(now.QuadPart - time_NTTime.QuadPart)) / time_NTFrequency);
}
*/

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if ( PD_APPLE || PD_LINUX )

/* MUST be thread-safe (after initialization). */

double clock_getRealTimeInSeconds (void)
{
    static t_time start;                    /* Static. */
    t_time now;
    t_nano elapsed = 0ULL;
    
    time_set (&now);
    
    if (start == 0ULL) { start = now; }
    else {
        time_elapsedNanoseconds (&start, &now, &elapsed);
    }
    
    return PD_NANOSECONDS_TO_SECONDS (elapsed);
}

/*
double clock_getRealTimeInSeconds (void)
{
    static struct timeval start;
    struct timeval now;
    
    gettimeofday (&now, NULL);
    if (start.tv_sec == 0 && start.tv_usec == 0) { start = now; }
    
    return ((now.tv_sec - start.tv_sec) + (1.0 / 1000000.0) * (now.tv_usec - start.tv_usec));
}
*/

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error clock_parseUnit (t_float f, t_symbol *s, t_float *n, int *isSamples)
{
    t_error err = (f <= 0.0);
    
    *n = 1; *isSamples = 0;
    
    if (!err) {
    //
    if (s == sym_permillisecond)    { *n = (t_float)(1.0 / f);     }
    else if (s == sym_persecond)    { *n = (t_float)(1000.0 / f);  }
    else if (s == sym_perminute)    { *n = (t_float)(60000.0 / f); }
    else if (s == sym_millisecond)  { *n = f;                      }
    else if (s == sym_second)       { *n = (t_float)(1000.0 * f);  }
    else if (s == sym_minute)       { *n = (t_float)(60000.0 * f); }
    else if (s == sym_sample)       { *n = f; *isSamples = 1;      }
    else {
        err = PD_ERROR;
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void clock_setUnit (t_clock *x, double unit, int isSamples)
{
    double d = isSamples ? -unit : unit; PD_ATOMIC_FLOAT64_WRITE (d, &x->c_unit);
}

t_error clock_setUnitParsed (t_clock *x, t_float f, t_symbol *unitName)
{
    t_float n; int isSamples;
    t_error err = clock_parseUnit (f, unitName, &n, &isSamples);
    
    if (!err) {
        if (n <= 0.0)  { n = 1.0; }
        if (isSamples) { clock_setUnit (x, n, 1); }
        else {
            clock_setUnit (x, n, 0);
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_systime clock_getLogicalTime (t_clock *x)
{
    return (t_systime)PD_ATOMIC_FLOAT64_READ (&x->c_systime);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Copy of systime used only while executed. */

t_systime clock_getExecuteTime (t_clock *x)
{
    return x->c_t;
}

void clock_setExecuteTime (t_clock *x, t_systime t)
{
    x->c_t = t;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int clock_increment (t_clock *x)
{
    int t = PD_ATOMIC_INT32_INCREMENT (&x->c_count);
    
    PD_ASSERT (t == 1);
    
    return t;
}

int clock_decrement (t_clock *x)
{
    int t = PD_ATOMIC_INT32_DECREMENT (&x->c_count);
    
    PD_ASSERT (t == 0);
    
    return t;
}

int clock_isSet (t_clock *x)
{
    return (PD_ATOMIC_INT32_READ (&x->c_count) > 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void clock_execute (t_clock *x)
{
    if (x->c_fn) { (*x->c_fn)(x->c_owner); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_clock *clock_new (void *owner, t_method fn)
{
    t_clock *x = (t_clock *)PD_MEMORY_GET (sizeof (t_clock));
    
    x->c_fn    = (t_clockfn)fn;
    x->c_owner = owner;

    PD_ATOMIC_FLOAT64_WRITE (1.0, &x->c_unit);
    
    return x;
}

void clock_free (t_clock *x)
{
    clock_unset (x);
    
    PD_ASSERT (!clock_isSet (x));
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void clock_unset (t_clock *x)
{
    instance_clocksRemove (x);
}

void clock_set (t_clock *x, t_systime t)
{
    clock_unset (x);
    
    {
    //
    PD_ATOMIC_FLOAT64_WRITE (t, &x->c_systime);
    
    instance_clocksAdd (x);
    //
    }
}

double clock_quantum (t_clock *x, double t)
{
    double d = 0.0;
    double u = PD_ATOMIC_FLOAT64_READ (&x->c_unit);
    
    if (u > 0.0) { d = u; }
    else {
        d = -(u * (1000.0 / audio_getSampleRate()));
    }
    
    d *= PD_MAX (0.0, t);
    
    return d;
}

void clock_delay (t_clock *x, double delay)             /* Could be in milliseconds or in samples. */
{
    clock_set (x, scheduler_getLogicalTimeAfter (clock_quantum (x, delay)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
