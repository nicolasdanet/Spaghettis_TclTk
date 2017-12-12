
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizRandom.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizUtils.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_32BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PIZRandom *pizRandomNew (void)
{
    return genrand32_new();
}

void pizRandomFree (PIZRandom *x)
{
    genrand32_free (x);
}

double pizRandomDouble (PIZRandom *x)
{
    return genrand32_res53 (x);
}

/* < http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/efaq.html > */

long pizRandomLong (PIZRandom *x, long v)
{
    if (v <= 1) { return 0; }
    else {
    //
    long r = 0;
    long k = pizUInt32NextPower2Index ((uint32_t)v);
    
    do {
        r = (long)(genrand32_int32 (x) >> (32 - k));
    } while (r >= v);
    
    return r;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_32BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_64BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

PIZRandom *pizRandomNew (void)
{
    return genrand64_new();
}

void pizRandomFree (PIZRandom *x)
{
    genrand64_free (x);
}

double pizRandomDouble (PIZRandom *x)
{
    return genrand64_real2 (x);
}

/* < http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/efaq.html > */

long pizRandomLong (PIZRandom *x, long v)
{
    if (v <= 1) { return 0; }
    else {
    //
    long r = 0;
    long k = pizUInt64NextPower2Index ((uint64_t)v);
    
    do {
        r = (long)(genrand64_int64 (x) >> (64 - k));
    } while (r >= v);
    
    return r;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_64BIT

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
