
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS

static LARGE_INTEGER    time_NTTime;
static double           time_NTFrequency;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

#if PD_WINDOWS

double sys_getRealTimeInSeconds (void)    
{
    LARGE_INTEGER now;
    
    QueryPerformanceCounter (&now);
    
    if (time_NTFrequency == 0) { time_initializeClock(); }
    
    return (((double)(now.QuadPart - time_NTTime.QuadPart)) / time_NTFrequency);
}

#else 

double sys_getRealTimeInSeconds (void)    
{
    static struct timeval start;
    struct timeval now;
    
    gettimeofday (&now, NULL);
    if (start.tv_sec == 0 && start.tv_usec == 0) { start = now; }
    
    return ((now.tv_sec - start.tv_sec) + (1.0 / 1000000.0) * (now.tv_usec - start.tv_usec));
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

void clock_unset (t_clock *x)
{
    if (x->c_systime >= 0.0) {
        if (x == pd_this->pd_clocks) { pd_this->pd_clocks = x->c_next; }
        else {
            t_clock *c = pd_this->pd_clocks;
            while (c->c_next != x) { c = c->c_next; } c->c_next = x->c_next;
        }
        x->c_systime = -1.0;
    }
}

static void clock_set (t_clock *x, t_systime time)
{
    if (time < pd_this->pd_systime) { time = pd_this->pd_systime; }
    
    clock_unset (x);
    
    x->c_systime = time;
    
    if (pd_this->pd_clocks && pd_this->pd_clocks->c_systime <= time) {
    
        t_clock *m = NULL;
        t_clock *n = NULL;
        
        for (m = pd_this->pd_clocks, n = pd_this->pd_clocks->c_next; m; m = n, n = m->c_next) {
            if (!n || n->c_systime > time) {
                m->c_next = x; x->c_next = n; return;
            }
        }
        
    } else {
        x->c_next = pd_this->pd_clocks; pd_this->pd_clocks = x;
    }
}

void clock_delay (t_clock *x, double delay)     /* Could be in milliseconds or in samples. */
{
    double d;
    t_systime time;
    
    if (x->c_unit > 0) { d = x->c_unit; }
    else {
        d = -(x->c_unit * (SYSTIME_PER_SECOND / audio_getSampleRate()));
    }

    time = pd_this->pd_systime + (d * delay);
    
    clock_set (x, time);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    timeLeft = (x->c_systime - pd_this->pd_systime) / d;
    //
    }
    
    if (isSamples) { x->c_unit = -unit; }
    else {
        x->c_unit = unit * SYSTIME_PER_MILLISECOND; 
    }
    
    if (timeLeft >= 0.0) { clock_delay (x, timeLeft); }
}

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
#pragma mark -

t_error clock_parseUnit (t_float f, t_symbol *s, t_float *n, int *isSamples)
{
    t_error err = (f <= 0);
    
    *n = 1; *isSamples = 0;
    
    if (!err) {
    //
    if (s == sym_permillisecond)    { *n = 1.0 / f;           }
    else if (s == sym_persecond)    { *n = 1000.0 / f;        }
    else if (s == sym_perminute)    { *n = 60000.0 / f;       }
    else if (s == sym_millisecond)  { *n = f;                 }
    else if (s == sym_second)       { *n = 1000.0 * f;        }
    else if (s == sym_minute)       { *n = 60000.0 * f;       }
    else if (s == sym_sample)       { *n = f; *isSamples = 1; }
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
