
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizTimePosix.h"
#include "pizTime.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizUtils.h"
#include "pizAtomic.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <errno.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_APPLE 

#include <mach/mach_time.h>

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_TIME_TRUNCATION_PATCH       1ULL
#define PIZ_TIME_HAS_BEEN_PREEMPTED     10000ULL
#define PIZ_TIME_FROM_1900_TO_1970      2208988800ULL  

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_APPLE

static mach_timebase_info_data_t pizTimeBaseInfo;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void pizTimeInitialize (void)
{
    #if PD_APPLE
    
    if (pizTimeBaseInfo.denom == 0) {
        mach_timebase_info (&pizTimeBaseInfo);   /* Must be initialized first (not thread-safe). */
    }
    
    #endif // PD_APPLE
}

void pizTimeCtor (void)  __attribute__ ((constructor));
void pizTimeCtor (void)  { pizTimeInitialize(); }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

uint64_t pizSeedMakeTimeBase16Bits (void)
{
    static uint16_t base = 0ULL;
    
    PIZTime t1, t2;
    uint16_t v1, v2;
    
    pizTimeSet (&t1);
    PIZ_MEMORY_BARRIER;     /* Avoid out-of-thin-air compiler optimizations. */
    pizTimeSet (&t2);
    
    v1 = (uint16_t)t1;
    v2 = pizUInt16Reversed ((uint16_t)t2);
    base ^= (v1 ^ v2);
    
    return (uint64_t)base;
}

uint64_t pizSeedMake (void)
{
    t_rand48 seed, lo, hi;

    seed = pizSeedMakeTimeBase16Bits();
    seed |= (pizSeedMakeTimeBase16Bits() << 16);
    seed |= (pizSeedMakeTimeBase16Bits() << 32);
    
    lo = PD_RAND48_UINT32 (seed);   /* To spread the bits. */
    hi = PD_RAND48_UINT32 (seed);   /* To spread the bits. */
    
    return ((hi << 32) | lo);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_APPLE

void pizTimeSet (PIZTime *t)
{
    (*t) = mach_absolute_time();
}

/* < https://developer.apple.com/library/mac/qa/qa1398/_index.html > */ 

void pizTimeAddNano (PIZTime *t, const PIZNano *ns)
{
    (*t) += (*ns) * pizTimeBaseInfo.denom / pizTimeBaseInfo.numer;
}

t_error pizTimeElapsedNano (const PIZTime *t0, const PIZTime *t1, PIZNano *r)
{
    (*r) = PIZ_ZERO_NANO;
    
    if ((*t1) > (*t0)) {
        uint64_t elapsed = (*t1) - (*t0);
        (*r) = elapsed * pizTimeBaseInfo.numer / pizTimeBaseInfo.denom;
        return PD_ERROR_NONE;
    }

    return PD_ERROR;
}

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_LINUX

void pizTimeSet (PIZTime *t)
{
    struct timespec time;
    
    clock_gettime (CLOCK_MONOTONIC, &time);
    
    uint64_t seconds     = (uint64_t)time.tv_sec;
    uint64_t nanoseconds = (uint64_t)time.tv_nsec;
    
    (*t) = (seconds * PIZ_NSEC_PER_SEC) + nanoseconds;
}

void pizTimeAddNano (PIZTime *t, const PIZNano *ns)
{
    (*t) += (*ns);
}

t_error pizTimeElapsedNano (const PIZTime *t0, const PIZTime *t1, PIZNano *r)
{
    (*r) = PIZ_ZERO_NANO;
    
    if ((*t1) > (*t0)) { (*r) = (*t1) - (*t0); return PD_ERROR_NONE; }

    return PD_ERROR;
}

#endif // PD_LINUX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void pizTimeCopy (PIZTime *t, const PIZTime *toCopy)
{
    (*t) = (*toCopy);
}

uint64_t pizTimeAsUInt64 (PIZTime *t)
{
    return (*t);
}

void pizTimeWithUInt64 (PIZTime *t, uint64_t n)
{
    (*t = n);
}

int pizTimeIsEqual (PIZTime *t1, PIZTime *t2)
{
    return ((*t1) == (*t2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pizNanoSleep (PIZNano *ns)
{
    if ((*ns) != PIZ_ZERO_NANO) {
    //
    struct timespec t0, t1;
    struct timespec *ptrA = &t0;
    struct timespec *ptrB = &t1;
    struct timespec *temp = NULL;

    t0.tv_sec  = (time_t)((*ns) / PIZ_NSEC_PER_SEC);
    t0.tv_nsec = (long)((*ns) % PIZ_NSEC_PER_SEC);

    while ((nanosleep (ptrA, ptrB) == -1) && (errno == EINTR)) {
        temp = ptrA;
        ptrA = ptrB;
        ptrB = temp;
    }
    //
    }
}

void pizNanoWithDouble (PIZNano *ns, double f)
{
    (*ns) = (PIZNano)f;
}

uint64_t pizNanoAsUInt64 (PIZNano *ns)
{
    return (*ns);
}

int pizNanoIsLessThan (PIZNano *t1, PIZNano *t2)
{
    return ((*t1) < (*t2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pizStampSet (PIZStamp *stamp)
{
    uint64_t s, f;
    struct timeval tv;
    
    gettimeofday (&tv, NULL);
    s = (uint64_t)tv.tv_sec + PIZ_TIME_FROM_1900_TO_1970;
    f = ((uint64_t)tv.tv_usec << 32) / PIZ_USEC_PER_SEC;
    // f = ((tv.tv_usec << 12) + (tv.tv_usec << 8) - ((tv.tv_usec * 1825) >> 5));

    (*stamp) = (s << 32) | f; 
}

void pizStampCopy (PIZStamp *stamp, const PIZStamp *toCopy)
{
    (*stamp) = (*toCopy);
}

void pizStampAddNano (PIZStamp *stamp, const PIZNano *ns)
{
    uint64_t hi = ((*stamp) >> 32);
    uint64_t lo = ((*stamp) & 0xffffffffULL);
    uint64_t s = hi;
    uint64_t n = ((lo * PIZ_NSEC_PER_SEC) >> 32) + (*ns);
        
    s += (uint64_t)(n / PIZ_NSEC_PER_SEC);
    n %= PIZ_NSEC_PER_SEC;

    (*stamp) = (s << 32) | ((uint64_t)((n << 32) / PIZ_NSEC_PER_SEC));
}

t_error pizStampElapsedNano (const PIZStamp *t0, const PIZStamp *t1, PIZNano *r)
{
    t_error err = PD_ERROR_NONE;

    uint64_t s0 = (*t0) >> 32;
    uint64_t n0 = (((*t0) & 0xffffffffULL) * PIZ_NSEC_PER_SEC) >> 32;
    uint64_t s1 = (*t1) >> 32;
    uint64_t n1 = (((*t1) & 0xffffffffULL) * PIZ_NSEC_PER_SEC) >> 32;
    
    (*r) = PIZ_ZERO_NANO;
    
    err |= (s1 < s0);                               /* < https://en.wikipedia.org/wiki/Year_2038_problem > */
    err |= (s1 == s0) && (n1 < n0);
    err |= ((*t0) == (*t1));
    
    if (!err) {
        (*r) = ((s1 * PIZ_NSEC_PER_SEC) + n1) - ((s0 * PIZ_NSEC_PER_SEC) + n0);
    }
    
    return err;
}

uint64_t pizStampAsUInt64 (PIZStamp *stamp)
{
    return (*stamp);
}

void pizStampWithUInt64 (PIZStamp *stamp, uint64_t n)
{
    (*stamp = n);
}

int pizStampIsEqual (PIZStamp *t1, PIZStamp *t2)
{
    return ((*t1) == (*t2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error pizBaseInit (PIZBase *base)
{
    PIZTime t;
    PIZNano ns;
    t_error err = PD_ERROR;
    
    do {
    
    pizTimeSet (&t);
    PIZ_MEMORY_BARRIER;                                 /* Avoid out-of-thin-air compiler optimizations. */
    err = (gettimeofday (&base->tv_, NULL) != 0);
    PIZ_MEMORY_BARRIER;
    pizTimeSet (&base->time_);
    pizTimeElapsedNano (&t, &base->time_, &ns); 
    
    } while (ns > PIZ_TIME_HAS_BEEN_PREEMPTED);
    
    return err;
}

t_error pizBaseTimeToStamp (const PIZBase *base, const PIZTime *t, PIZStamp *stamp)
{
    uint64_t f, s;
    PIZNano elapsed, n;
    t_error err = PD_ERROR;
    
    (*stamp) = PIZ_ZERO_STAMP;
    
    if (!(err = pizTimeElapsedNano (&base->time_, t, &elapsed))) {
        n = elapsed + ((uint64_t)base->tv_.tv_usec * PIZ_NSEC_PER_USEC);
        s = (uint64_t)base->tv_.tv_sec + PIZ_TIME_FROM_1900_TO_1970 + (uint64_t)(n / PIZ_NSEC_PER_SEC);
        f = (uint64_t)(((n % PIZ_NSEC_PER_SEC) << 32) / PIZ_NSEC_PER_SEC);
        
        (*stamp) = (s << 32) | f;
    }
    
    return err;
}

t_error pizBaseStampToTime (const PIZBase *base, const PIZStamp *stamp, PIZTime *t)
{
    t_error err = PD_ERROR;
    
    (*t) = PIZ_ZERO_TIME;
    
    if ((*stamp) != PIZ_ZERO_STAMP) {
    //
    uint64_t hi = ((*stamp) >> 32);
    uint64_t lo = ((*stamp) & 0xffffffffULL);
    uint64_t s0 = (uint64_t)base->tv_.tv_sec + PIZ_TIME_FROM_1900_TO_1970;
    uint64_t n0 = (uint64_t)base->tv_.tv_usec * PIZ_NSEC_PER_USEC;
    uint64_t s1 = hi;
    uint64_t n1 = ((lo * PIZ_NSEC_PER_SEC) >> 32);
    
    err = PD_ERROR_NONE;
    err |= (s1 < s0);                               /* < https://en.wikipedia.org/wiki/Year_2038_problem > */
    err |= (s1 == s0) && (n1 <= n0);
    
    if (!err) {
        (*t) = ((s1 * PIZ_NSEC_PER_SEC) + n1) - ((s0 * PIZ_NSEC_PER_SEC) + n0) + base->time_;
        (*t) += PIZ_TIME_TRUNCATION_PATCH;
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
