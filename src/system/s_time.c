
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TIME_NSEC_PER_SEC           1000000000ULL
#define TIME_USEC_PER_SEC           1000000ULL

#define TIME_FROM_1900_TO_1970      2208988800ULL

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_MAC_TIME

static mach_timebase_info_data_t time_baseInfo;       /* Static. */

static void time_initialize (void)
{
    if (time_baseInfo.denom == 0) {
        mach_timebase_info (&time_baseInfo);          /* Must be initialized first (not thread-safe). */
    }
}

void time_ctor (void)  __attribute__ ((constructor));
void time_ctor (void)  { time_initialize(); }

#endif // PD_MAC_TIME

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://graphics.stanford.edu/~seander/bithacks.html > */
/* < http://aggregate.org/MAGIC/ > */

static uint8_t time_makeSeed16Reversed8 (uint8_t v)
{
    return (v * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}

static uint16_t time_makeSeed16Reversed (uint16_t v)
{
    uint8_t hi = (uint8_t)(v >> 8);
    uint8_t lo = (uint8_t)(v & 0xff);
    
    return (((uint16_t)(time_makeSeed16Reversed8 (lo))) << 8) | (uint16_t)(time_makeSeed16Reversed8 (hi));
}

static uint64_t time_makeSeed16 (void)
{
    static uint16_t base = 0;           /* Static. */
    
    t_time t1, t2;
    uint16_t v1, v2;
    
    time_set (&t1);
    time_set (&t2);
    
    v1 = (uint16_t)t1;
    v2 = time_makeSeed16Reversed ((uint16_t)t2);
    base ^= (v1 ^ v2);
    
    return (uint64_t)base;
}

static uint64_t time_makeSeed (void)
{
    uint64_t seed, lo, hi;

    seed = time_makeSeed16();
    seed |= (time_makeSeed16() << 16);
    seed |= (time_makeSeed16() << 32);
    
    lo = PD_RAND48_UINT32 (seed);       /* To spread the bits. */
    hi = PD_RAND48_UINT32 (seed);       /* To spread the bits. */
    
    return ((hi << 32) | lo);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Do NOT fit for cryptography purpose. */

t_seed time_makeRandomSeed (void)
{
    static uint64_t seed = 0;           /* Static. */
    
    seed ^= utils_unique();
    
    PD_RAND48_NEXT (seed);
    
    #if PD_WINDOWS
        seed ^= _getpid();
    #else
        seed ^= getpid();
    #endif
    
    PD_RAND48_NEXT (seed);
    
    seed ^= time_makeSeed();
    
    return (t_seed)seed;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_MAC_TIME

void time_set (t_time *t)
{
    (*t) = mach_absolute_time();
}

/* < https://developer.apple.com/library/mac/qa/qa1398/_index.html > */ 

void time_addNanoseconds (t_time *t, t_nano ns)
{
    (*t) += ns * time_baseInfo.denom / time_baseInfo.numer;
}

t_error time_elapsedNanoseconds (const t_time *t0, const t_time *t1, t_nano *r)
{
    (*r) = 0ULL;
    
    if ((*t1) > (*t0)) {
        uint64_t elapsed = (*t1) - (*t0);
        (*r) = elapsed * time_baseInfo.numer / time_baseInfo.denom;
        return PD_ERROR_NONE;
    }

    return PD_ERROR;
}

#endif // PD_MAC_TIME

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_POSIX_TIME

void time_set (t_time *t)
{
    struct timespec time;
    
    clock_gettime (CLOCK_MONOTONIC, &time);
    
    uint64_t seconds     = (uint64_t)time.tv_sec;
    uint64_t nanoseconds = (uint64_t)time.tv_nsec;
    
    (*t) = (seconds * TIME_NSEC_PER_SEC) + nanoseconds;
}

void time_addNanoseconds (t_time *t, t_nano ns)
{
    (*t) += ns;
}

t_error time_elapsedNanoseconds (const t_time *t0, const t_time *t1, t_nano *r)
{
    (*r) = 0ULL;
    
    if ((*t1) > (*t0)) { (*r) = (*t1) - (*t0); return PD_ERROR_NONE; }

    return PD_ERROR;
}

#endif // PD_POSIX_TIME

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void nano_sleep (t_nano ns)
{
    if (ns != 0ULL) {
    //
    struct timespec t0, t1;
    struct timespec *ptrA = &t0;
    struct timespec *ptrB = &t1;
    struct timespec *temp = NULL;

    t0.tv_sec  = (time_t)(ns / TIME_NSEC_PER_SEC);
    t0.tv_nsec = (long)(ns % TIME_NSEC_PER_SEC);

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
    s = (uint64_t)tv.tv_sec + TIME_FROM_1900_TO_1970;
    f = ((uint64_t)tv.tv_usec << 32) / TIME_USEC_PER_SEC;
    // f = ((tv.tv_usec << 12) + (tv.tv_usec << 8) - ((tv.tv_usec * 1825) >> 5));

    (*stamp) = (s << 32) | f; 
}

void stamp_addNanoseconds (t_stamp *stamp, t_nano ns)
{
    uint64_t hi = ((*stamp) >> 32);
    uint64_t lo = ((*stamp) & 0xffffffffULL);
    uint64_t s = hi;
    uint64_t n = ((lo * TIME_NSEC_PER_SEC) >> 32) + ns;
        
    s += (uint64_t)(n / TIME_NSEC_PER_SEC);
    n %= TIME_NSEC_PER_SEC;

    (*stamp) = (s << 32) | ((uint64_t)((n << 32) / TIME_NSEC_PER_SEC));
}

t_error stamp_elapsedNanoseconds (const t_stamp *t0, const t_stamp *t1, t_nano *r)
{
    t_error err = PD_ERROR_NONE;

    uint64_t s0 = (*t0) >> 32;
    uint64_t n0 = (((*t0) & 0xffffffffULL) * TIME_NSEC_PER_SEC) >> 32;
    uint64_t s1 = (*t1) >> 32;
    uint64_t n1 = (((*t1) & 0xffffffffULL) * TIME_NSEC_PER_SEC) >> 32;
    
    (*r) = 0ULL;
    
    err |= (s1 < s0);                               /* < https://en.wikipedia.org/wiki/Year_2038_problem > */
    err |= (s1 == s0) && (n1 < n0);
    err |= ((*t0) == (*t1));
    
    if (!err) {
        (*r) = ((s1 * TIME_NSEC_PER_SEC) + n1) - ((s0 * TIME_NSEC_PER_SEC) + n0);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_symbol *stamp_4BitsToTag (unsigned char b)
{
    switch (b) {
        case 0  : return sym___arrobe__0;
        case 1  : return sym___arrobe__1;
        case 2  : return sym___arrobe__2;
        case 3  : return sym___arrobe__3;
        case 4  : return sym___arrobe__4;
        case 5  : return sym___arrobe__5;
        case 6  : return sym___arrobe__6;
        case 7  : return sym___arrobe__7;
        case 8  : return sym___arrobe__8;
        case 9  : return sym___arrobe__9;
        case 10 : return sym___arrobe__a;
        case 11 : return sym___arrobe__b;
        case 12 : return sym___arrobe__c;
        case 13 : return sym___arrobe__d;
        case 14 : return sym___arrobe__e;
        default : return sym___arrobe__f;
    }
}

static unsigned char stamp_tagTo4Bits (t_symbol *s)
{
    if (s == sym___arrobe__0)       { return 0x00; }
    else if (s == sym___arrobe__1)  { return 0x01; }
    else if (s == sym___arrobe__2)  { return 0x02; }
    else if (s == sym___arrobe__3)  { return 0x03; }
    else if (s == sym___arrobe__4)  { return 0x04; }
    else if (s == sym___arrobe__5)  { return 0x05; }
    else if (s == sym___arrobe__6)  { return 0x06; }
    else if (s == sym___arrobe__7)  { return 0x07; }
    else if (s == sym___arrobe__8)  { return 0x08; }
    else if (s == sym___arrobe__9)  { return 0x09; }
    else if (s == sym___arrobe__a)  { return 0x0a; }
    else if (s == sym___arrobe__b)  { return 0x0b; }
    else if (s == sym___arrobe__c)  { return 0x0c; }
    else if (s == sym___arrobe__d)  { return 0x0d; }
    else if (s == sym___arrobe__e)  { return 0x0e; }
    else if (s == sym___arrobe__f)  { return 0x0f; }
    
    return 0xff;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int stamp_isTag (t_symbol *s)
{
    return (stamp_tagTo4Bits (s) != 0xff);
}

t_error stamp_setAsTags (int argc, t_atom *argv, t_stamp *stamp)
{
    if (argc >= STAMP_TAGS_SIZE) {
    //
    int i;
    
    for (i = 0; i < STAMP_TAGS_SIZE; i++) {
    //
    unsigned char t = ((*stamp) >> (i * 4)) & 0x0f;
    SET_SYMBOL (argv + (STAMP_TAGS_SIZE - 1 - i), stamp_4BitsToTag (t));
    //
    }
    
    return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

t_error stamp_getWithTags (int argc, t_atom *argv, t_stamp *stamp)
{
    if (argc >= STAMP_TAGS_SIZE) {
    //
    t_error err = PD_ERROR_NONE;
    uint64_t t = 0;
    int i;
    
    for (i = 0; i < STAMP_TAGS_SIZE; i++) {
    //
    unsigned char b = stamp_tagTo4Bits (atom_getSymbol (argv + i));
    err |= (b == 0xff); t |= b; if (i != (STAMP_TAGS_SIZE - 1)) { t <<= 4; }
    //
    }
    
    if (!err) { (*stamp) = t; return PD_ERROR_NONE; }
    //
    }
    
    (*stamp) = 0; return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
