
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS

static LARGE_INTEGER    time_NTTime;            /* Static. */
static double           time_NTFrequency;       /* Static. */

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

static void time_initializeClock (void)
{
    LARGE_INTEGER f1;
    LARGE_INTEGER now;
    
    QueryPerformanceCounter (&now);
    
    if (!QueryPerformanceFrequency (&f1)) { PD_BUG; f1.QuadPart = 1; }
    
    time_NTTime = now;
    time_NTFrequency = f1.QuadPart;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

double clock_getRealTimeInSeconds (void)    
{
    LARGE_INTEGER now;
    
    QueryPerformanceCounter (&now);
    
    if (time_NTFrequency == 0) { time_initializeClock(); }
    
    return (((double)(now.QuadPart - time_NTTime.QuadPart)) / time_NTFrequency);
}

#else 

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
    
    return NANOSECONDS_TO_SECONDS (elapsed);
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

#endif // PD_WINDOWS

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
        #if PD_WITH_LEGACY
        
            if (s == sym_perms)         { err = clock_parseUnit (f, sym_permillisecond, n, isSamples); }
            else if (s == sym_permsec)  { err = clock_parseUnit (f, sym_permillisecond, n, isSamples); }
            else if (s == sym_persec)   { err = clock_parseUnit (f, sym_persecond,      n, isSamples); }
            else if (s == sym_permin)   { err = clock_parseUnit (f, sym_perminute,      n, isSamples); }
            else if (s == sym_msec)     { err = clock_parseUnit (f, sym_millisecond,    n, isSamples); }
            else if (s == sym_ms)       { err = clock_parseUnit (f, sym_millisecond,    n, isSamples); }
            else if (s == sym_sec)      { err = clock_parseUnit (f, sym_second,         n, isSamples); }
            else if (s == sym_min)      { err = clock_parseUnit (f, sym_minute,         n, isSamples); }
            else if (s == sym_sam)      { err = clock_parseUnit (f, sym_sample,         n, isSamples); }
            else if (s == sym_samp)     { err = clock_parseUnit (f, sym_sample,         n, isSamples); }
            else {
                err = PD_ERROR;
            }
        
        #else
        
        err = PD_ERROR;
        
        #endif
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_clock *clock_new (void *owner, t_method fn)
{
    t_clock *x = (t_clock *)PD_MEMORY_GET (sizeof (t_clock));
    
    x->c_systime    = -1.0;
    x->c_unit       = SYSTIME_PER_MILLISECOND;
    x->c_fn         = (t_clockfn)fn;
    x->c_owner      = owner;
    x->c_next       = NULL;

    return x;
}

void clock_free (t_clock *x)
{
    clock_unset (x);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void clock_unset (t_clock *x)
{
    if (x->c_systime >= 0.0) { instance_clockUnset (x); x->c_systime = -1.0; }
}

static void clock_set (t_clock *x, t_systime systime)
{
    if (systime < scheduler_getLogicalTime()) { systime = scheduler_getLogicalTime(); }
    
    clock_unset (x);
    
    x->c_systime = systime;
    
    instance_clockAdd (x);
}

void clock_delay (t_clock *x, double delay)     /* Could be in milliseconds or in samples. */
{
    double d;
    t_systime systime;
    
    if (x->c_unit > 0) { d = x->c_unit; }
    else {
        d = -(x->c_unit * (SYSTIME_PER_SECOND / audio_getSampleRate()));
    }

    systime = scheduler_getLogicalTime() + (d * delay);
    
    clock_set (x, systime);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void clock_setUnit (t_clock *x, double unit, int isSamples)
{
    double timeLeft = -1.0;
    
    if (unit <= 0.0) { unit = 1.0; }
    
    if (isSamples) { if (unit == -x->c_unit) { return; } }
    else { 
        if (unit == x->c_unit * SYSTIME_PER_MILLISECOND) { return; }
    }
    
    if (x->c_systime >= 0.0) {
    //
    double d = (x->c_unit > 0) ? x->c_unit : (x->c_unit * (SYSTIME_PER_SECOND / audio_getSampleRate()));
    timeLeft = (x->c_systime - scheduler_getLogicalTime()) / d;
    //
    }
    
    if (isSamples) { x->c_unit = -unit; }
    else {
        x->c_unit = unit * SYSTIME_PER_MILLISECOND; 
    }
    
    if (timeLeft >= 0.0) { clock_delay (x, timeLeft); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void clock_setUnitAsSamples (t_clock *x, double samples) 
{
    clock_setUnit (x, samples, 1);
}

void clock_setUnitAsMilliseconds (t_clock *x, double ms) 
{
    clock_setUnit (x, ms, 0);
}

t_error clock_setUnitParsed (t_clock *x, t_float f, t_symbol *unitName)
{
    t_float n; int isSamples;
    t_error err = clock_parseUnit (f, unitName, &n, &isSamples);
    
    if (!err) {
        if (isSamples) { clock_setUnitAsSamples (x, n); }
        else {
            clock_setUnitAsMilliseconds (x, n);
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
