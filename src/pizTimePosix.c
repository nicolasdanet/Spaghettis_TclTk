
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

#define PIZ_NSEC_PER_USEC               1000ULL
#define PIZ_NSEC_PER_SEC                1000000000ULL
#define PIZ_USEC_PER_SEC                1000000ULL

#define PD_ZERO_TIME                    0ULL
#define PD_ZERO_NANO                    0ULL
#define PD_ZERO_STAMP                   0ULL

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

static uint8_t pizUInt8Reversed (uint8_t v)
{
    return (v * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}

static uint16_t pizUInt16Reversed (uint16_t v)
{
    uint8_t hi = (uint8_t)(v >> 8);
    uint8_t lo = (uint8_t)(v & 0xff);
    
    return (((uint16_t)(pizUInt8Reversed (lo))) << 8) | (uint16_t)(pizUInt8Reversed (hi));
}

uint64_t pizSeedMakeTimeBase16Bits (void)
{
    static uint16_t base = 0ULL;
    
    t_time t1, t2;
    uint16_t v1, v2;
    
    time_set (&t1);
    PIZ_MEMORY_BARRIER;     /* Avoid out-of-thin-air compiler optimizations. */
    time_set (&t2);
    
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

/* Do NOT fit for cryptography purpose. */

uint64_t time_makeRandomSeed (void)
{
    static uint64_t seed = 0;
    
    t_rawcast64 z;
    
    z.z_d = clock_getRealTimeInSeconds();
    
    seed ^= utils_unique();             PD_RAND48_NEXT (seed);
    seed ^= z.z_i[PD_RAWCAST64_LSB];    PD_RAND48_NEXT (seed);
    seed ^= z.z_i[PD_RAWCAST64_MSB];    PD_RAND48_NEXT (seed);
    
    #if PD_WINDOWS
        seed ^= _getpid();
    #else
        seed ^= getpid();
    #endif
    
    PD_RAND48_NEXT (seed);
    
    return seed;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_APPLE

void time_set (t_time *t)
{
    (*t) = mach_absolute_time();
}

/* < https://developer.apple.com/library/mac/qa/qa1398/_index.html > */ 

void time_addNanoseconds (t_time *t, const t_nano *ns)
{
    (*t) += (*ns) * pizTimeBaseInfo.denom / pizTimeBaseInfo.numer;
}

t_error time_elapsedNanoseconds (const t_time *t0, const t_time *t1, t_nano *r)
{
    (*r) = PD_ZERO_NANO;
    
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

void time_set (t_time *t)
{
    struct timespec time;
    
    clock_gettime (CLOCK_MONOTONIC, &time);
    
    uint64_t seconds     = (uint64_t)time.tv_sec;
    uint64_t nanoseconds = (uint64_t)time.tv_nsec;
    
    (*t) = (seconds * PIZ_NSEC_PER_SEC) + nanoseconds;
}

void time_addNanoseconds (t_time *t, const t_nano *ns)
{
    (*t) += (*ns);
}

t_error time_elapsedNanoseconds (const t_time *t0, const t_time *t1, t_nano *r)
{
    (*r) = PD_ZERO_NANO;
    
    if ((*t1) > (*t0)) { (*r) = (*t1) - (*t0); return PD_ERROR_NONE; }

    return PD_ERROR;
}

#endif // PD_LINUX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void nano_sleep (t_nano *ns)
{
    if ((*ns) != PD_ZERO_NANO) {
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void stamp_set (t_stamp *stamp)
{
    uint64_t s, f;
    struct timeval tv;
    
    gettimeofday (&tv, NULL);
    s = (uint64_t)tv.tv_sec + PIZ_TIME_FROM_1900_TO_1970;
    f = ((uint64_t)tv.tv_usec << 32) / PIZ_USEC_PER_SEC;
    // f = ((tv.tv_usec << 12) + (tv.tv_usec << 8) - ((tv.tv_usec * 1825) >> 5));

    (*stamp) = (s << 32) | f; 
}

void stamp_addNanoseconds (t_stamp *stamp, const t_nano *ns)
{
    uint64_t hi = ((*stamp) >> 32);
    uint64_t lo = ((*stamp) & 0xffffffffULL);
    uint64_t s = hi;
    uint64_t n = ((lo * PIZ_NSEC_PER_SEC) >> 32) + (*ns);
        
    s += (uint64_t)(n / PIZ_NSEC_PER_SEC);
    n %= PIZ_NSEC_PER_SEC;

    (*stamp) = (s << 32) | ((uint64_t)((n << 32) / PIZ_NSEC_PER_SEC));
}

t_error stamp_elapsedNanoseconds (const t_stamp *t0, const t_stamp *t1, t_nano *r)
{
    t_error err = PD_ERROR_NONE;

    uint64_t s0 = (*t0) >> 32;
    uint64_t n0 = (((*t0) & 0xffffffffULL) * PIZ_NSEC_PER_SEC) >> 32;
    uint64_t s1 = (*t1) >> 32;
    uint64_t n1 = (((*t1) & 0xffffffffULL) * PIZ_NSEC_PER_SEC) >> 32;
    
    (*r) = PD_ZERO_NANO;
    
    err |= (s1 < s0);                               /* < https://en.wikipedia.org/wiki/Year_2038_problem > */
    err |= (s1 == s0) && (n1 < n0);
    err |= ((*t0) == (*t1));
    
    if (!err) {
        (*r) = ((s1 * PIZ_NSEC_PER_SEC) + n1) - ((s0 * PIZ_NSEC_PER_SEC) + n0);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error timebase_init (t_timebase *base)
{
    t_time t;
    t_nano ns;
    t_error err = PD_ERROR;
    
    do {
    
    time_set (&t);
    PIZ_MEMORY_BARRIER;                                 /* Avoid out-of-thin-air compiler optimizations. */
    err = (gettimeofday (&base->tv_, NULL) != 0);
    PIZ_MEMORY_BARRIER;
    time_set (&base->time_);
    time_elapsedNanoseconds (&t, &base->time_, &ns);
    
    } while (ns > PIZ_TIME_HAS_BEEN_PREEMPTED);
    
    return err;
}

t_error timebase_timeToStamp (const t_timebase *base, const t_time *t, t_stamp *stamp)
{
    uint64_t f, s;
    t_nano elapsed, n;
    t_error err = PD_ERROR;
    
    (*stamp) = PD_ZERO_STAMP;
    
    if (!(err = time_elapsedNanoseconds (&base->time_, t, &elapsed))) {
        n = elapsed + ((uint64_t)base->tv_.tv_usec * PIZ_NSEC_PER_USEC);
        s = (uint64_t)base->tv_.tv_sec + PIZ_TIME_FROM_1900_TO_1970 + (uint64_t)(n / PIZ_NSEC_PER_SEC);
        f = (uint64_t)(((n % PIZ_NSEC_PER_SEC) << 32) / PIZ_NSEC_PER_SEC);
        
        (*stamp) = (s << 32) | f;
    }
    
    return err;
}

t_error timebase_stampToTime (const t_timebase *base, const t_stamp *stamp, t_time *t)
{
    t_error err = PD_ERROR;
    
    (*t) = PD_ZERO_TIME;
    
    if ((*stamp) != PD_ZERO_STAMP) {
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
