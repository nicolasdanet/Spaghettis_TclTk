
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizDefine.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizTimePosix.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizUtils.h"
#include "pizLCG.h"
#include "pizAtomic.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <errno.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PIZ_APPLE 

#include <mach/mach_time.h>

#endif // PIZ_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_TIME_TRUNCATION_PATCH       1ULL
#define PIZ_TIME_HAS_BEEN_PREEMPTED     10000ULL
#define PIZ_TIME_FROM_1900_TO_1970      2208988800ULL  

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static PIZUInt64 pizSeedMakeTime            (void);
static PIZUInt64 pizSeedMakeTimeBase16Bits  (void);
static PIZUInt64 pizSeedMakeConstant        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static PIZInt32Atomic pizSeedIsConstant;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PIZ_APPLE

static mach_timebase_info_data_t pizTimeBaseInfo;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void pizTimeInitialize (void)
{
    #if PIZ_APPLE
    
    if (pizTimeBaseInfo.denom == 0) {
        mach_timebase_info (&pizTimeBaseInfo);   /* Must be initialized first (not thread-safe). */
    }
    
    #endif // PIZ_APPLE
}

#if PIZ_CONSTRUCTOR

void pizTimeCtor (void)  __attribute__ ((constructor));
void pizTimeCtor (void)  { pizTimeInitialize(); }

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pizSeedConstant (PIZBool isConstant)
{
    PIZ_ATOMIC_INT32_WRITE ((PIZInt32)isConstant, &pizSeedIsConstant);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PIZUInt64 pizSeedMake (void)
{
    if (PIZ_ATOMIC_INT32_READ (&pizSeedIsConstant) == 1) { return pizSeedMakeConstant(); } 
    else {
        return pizSeedMakeTime();
    }
}

PIZUInt64 pizSeedMakeConstant (void)
{
    PIZUInt64 lo, hi, seed;
    
    seed = 0xcacafade;
    
    lo = PIZ_RAND48_UINT32 (seed);
    hi = PIZ_RAND48_UINT32 (seed);
    
    return ((hi << 32) | lo);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PIZUInt64 pizSeedMakeTime (void)
{
    PIZUInt64 seed, lo, hi;

    seed = pizSeedMakeTimeBase16Bits();
    seed |= (pizSeedMakeTimeBase16Bits() << 16);
    seed |= (pizSeedMakeTimeBase16Bits() << 32);
    
    lo = PIZ_RAND48_UINT32 (seed);   /* To spread the bits. */
    hi = PIZ_RAND48_UINT32 (seed);   /* To spread the bits. */
    
    return ((hi << 32) | lo);
}

PIZUInt64 pizSeedMakeTimeBase16Bits (void)
{
    static PIZUInt16 base = 0ULL;
    
    PIZTime t1, t2;
    PIZUInt16 v1, v2;
        
    pizTimeSet (&t1);
    PIZ_MEMORY_BARRIER;     /* Avoid out-of-thin-air compiler optimizations. */
    pizTimeSet (&t2);
    
    v1 = (PIZUInt16)t1;
    v2 = pizUInt16Reversed ((PIZUInt16)t2);
    base ^= (v1 ^ v2);
    
    return (PIZUInt64)base;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PIZ_APPLE

void pizTimeSet (PIZTime *t)
{
    (*t) = mach_absolute_time();
}

/* < https://developer.apple.com/library/mac/qa/qa1398/_index.html > */ 

void pizTimeAddNano (PIZTime *t, const PIZNano *ns)
{
    (*t) += (*ns) * pizTimeBaseInfo.denom / pizTimeBaseInfo.numer;
}

PIZError pizTimeElapsedNano (const PIZTime *t0, const PIZTime *t1, PIZNano *r)
{
    (*r) = PIZ_ZERO_NANO;
    
    if ((*t1) > (*t0)) {
        PIZUInt64 elapsed = (*t1) - (*t0); 
        (*r) = elapsed * pizTimeBaseInfo.numer / pizTimeBaseInfo.denom;
        return PIZ_GOOD;
    }

    return PIZ_ERROR;
}

#endif // PIZ_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PIZ_LINUX

void pizTimeSet (PIZTime *t)
{
    struct timespec time;
    
    clock_gettime (CLOCK_MONOTONIC, &time);
    
    PIZUInt64 seconds     = (PIZUInt64)time.tv_sec;
    PIZUInt64 nanoseconds = (PIZUInt64)time.tv_nsec;
    
    (*t) = (seconds * PIZ_NSEC_PER_SEC) + nanoseconds;
}

void pizTimeAddNano (PIZTime *t, const PIZNano *ns)
{
    (*t) += (*ns);
}

PIZError pizTimeElapsedNano (const PIZTime *t0, const PIZTime *t1, PIZNano *r)
{
    (*r) = PIZ_ZERO_NANO;
    
    if ((*t1) > (*t0)) { (*r) = (*t1) - (*t0); return PIZ_GOOD; }

    return PIZ_ERROR;
}

#endif // PIZ_LINUX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void pizTimeCopy (PIZTime *t, const PIZTime *toCopy)
{
    (*t) = (*toCopy);
}

PIZUInt64 pizTimeAsUInt64 (PIZTime *t)
{
    return (*t);
}

void pizTimeWithUInt64 (PIZTime *t, PIZUInt64 n)
{
    (*t = n);
}

PIZBool pizTimeIsEqual (PIZTime *t1, PIZTime *t2)
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

PIZUInt64 pizNanoAsUInt64 (PIZNano *ns)
{
    return (*ns);
}

PIZBool pizNanoIsLessThan (PIZNano *t1, PIZNano *t2)
{
    return ((*t1) < (*t2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pizStampSet (PIZStamp *stamp)
{
    PIZUInt64 s, f;
    struct timeval tv;
    
    gettimeofday (&tv, NULL);
    s = (PIZUInt64)tv.tv_sec + PIZ_TIME_FROM_1900_TO_1970;
    f = ((PIZUInt64)tv.tv_usec << 32) / PIZ_USEC_PER_SEC;
    // f = ((tv.tv_usec << 12) + (tv.tv_usec << 8) - ((tv.tv_usec * 1825) >> 5));

    (*stamp) = (s << 32) | f; 
}

void pizStampCopy (PIZStamp *stamp, const PIZStamp *toCopy)
{
    (*stamp) = (*toCopy);
}

void pizStampAddNano (PIZStamp *stamp, const PIZNano *ns)
{
    PIZUInt64 hi = ((*stamp) >> 32);
    PIZUInt64 lo = ((*stamp) & 0xffffffffULL);
    PIZUInt64 s = hi;
    PIZUInt64 n = ((lo * PIZ_NSEC_PER_SEC) >> 32) + (*ns);
        
    s += (PIZUInt64)(n / PIZ_NSEC_PER_SEC);
    n %= PIZ_NSEC_PER_SEC;

    (*stamp) = (s << 32) | ((PIZUInt64)((n << 32) / PIZ_NSEC_PER_SEC));
}

PIZError pizStampElapsedNano (const PIZStamp *t0, const PIZStamp *t1, PIZNano *r)
{
    PIZError err = PIZ_GOOD;

    PIZUInt64 s0 = (*t0) >> 32;
    PIZUInt64 n0 = (((*t0) & 0xffffffffULL) * PIZ_NSEC_PER_SEC) >> 32;
    PIZUInt64 s1 = (*t1) >> 32;
    PIZUInt64 n1 = (((*t1) & 0xffffffffULL) * PIZ_NSEC_PER_SEC) >> 32;
    
    (*r) = PIZ_ZERO_NANO;
    
    err |= (s1 < s0);                               /* < https://en.wikipedia.org/wiki/Year_2038_problem > */
    err |= (s1 == s0) && (n1 < n0);
    err |= ((*t0) == (*t1));
    
    if (!err) {
        (*r) = ((s1 * PIZ_NSEC_PER_SEC) + n1) - ((s0 * PIZ_NSEC_PER_SEC) + n0);
    }
    
    return err;
}

PIZUInt64 pizStampAsUInt64 (PIZStamp *stamp)
{
    return (*stamp);
}

void pizStampWithUInt64 (PIZStamp *stamp, PIZUInt64 n)
{
    (*stamp = n);
}

PIZBool pizStampIsEqual (PIZStamp *t1, PIZStamp *t2)
{
    return ((*t1) == (*t2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PIZError pizBaseInit (PIZBase *base)
{
    PIZTime t;
    PIZNano ns;
    PIZError err = PIZ_ERROR;
    
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

PIZError pizBaseTimeToStamp (const PIZBase *base, const PIZTime *t, PIZStamp *stamp)
{
    PIZUInt64 f, s;
    PIZNano elapsed, n;
    PIZError err = PIZ_ERROR;
    
    (*stamp) = PIZ_ZERO_STAMP;
    
    if (!(err = pizTimeElapsedNano (&base->time_, t, &elapsed))) {
        n = elapsed + ((PIZUInt64)base->tv_.tv_usec * PIZ_NSEC_PER_USEC);
        s = (PIZUInt64)base->tv_.tv_sec + PIZ_TIME_FROM_1900_TO_1970 + (PIZUInt64)(n / PIZ_NSEC_PER_SEC);
        f = (PIZUInt64)(((n % PIZ_NSEC_PER_SEC) << 32) / PIZ_NSEC_PER_SEC);
        
        (*stamp) = (s << 32) | f;
    }
    
    return err;
}

PIZError pizBaseStampToTime (const PIZBase *base, const PIZStamp *stamp, PIZTime *t)
{
    PIZError err = PIZ_ERROR;
    
    (*t) = PIZ_ZERO_TIME;
    
    if ((*stamp) != PIZ_ZERO_STAMP) {
    //
    PIZUInt64 hi = ((*stamp) >> 32);
    PIZUInt64 lo = ((*stamp) & 0xffffffffULL);
    PIZUInt64 s0 = (PIZUInt64)base->tv_.tv_sec + PIZ_TIME_FROM_1900_TO_1970;
    PIZUInt64 n0 = (PIZUInt64)base->tv_.tv_usec * PIZ_NSEC_PER_USEC;
    PIZUInt64 s1 = hi;
    PIZUInt64 n1 = ((lo * PIZ_NSEC_PER_SEC) >> 32);
    
    err = PIZ_GOOD;
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
