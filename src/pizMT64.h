
/*
    Mersenne Twister PRNG. 

    This is a 64-bit version of Mersenne Twister pseudorandom number generator.

    "Tables of 64-bit Mersenne Twisters", 
        T. Nishimura, 
        ACM Transactions on Modeling and Computer Simulation.

    "Mersenne Twister a 623-dimensionally equidistributed uniform pseudorandom number generator",
        M. Matsumoto and T. Nishimura, 
        ACM Transactions on Modeling and Computer Simulation.

*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt64.html > */
 
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PIZ_MT64_H
#define PIZ_MT64_H

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _PIZRandom {
    PIZUInt64   mt_[312];
    int         mti_;
    } MTState64;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

MTState64   *genrand64_new          (void);
MTState64   *genrand64_newByArray   (long argc, PIZUInt64 *argv);

void        genrand64_free  (MTState64 *x);
PIZUInt64   genrand64_int64 (MTState64 *x);     // -- Random number on [0, 2 ^ 64 - 1] interval.
double      genrand64_real2 (MTState64 *x);     // -- Random number on [0, 1) interval.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PIZ_MT64_H
