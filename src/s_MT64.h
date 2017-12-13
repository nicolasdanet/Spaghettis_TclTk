
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

#ifndef __s_MT64_h_
#define __s_MT64_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _MTState64 {
    uint64_t    mt_[312];
    int         mti_;
    } MTState64;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

MTState64   *genrand64_new          (void);
MTState64   *genrand64_newByArray   (long argc, uint64_t *argv);

void        genrand64_free  (MTState64 *x);
uint64_t    genrand64_int64 (MTState64 *x);     // -- Random number on [0, 2 ^ 64 - 1] interval.
double      genrand64_real2 (MTState64 *x);     // -- Random number on [0, 1) interval.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_MT64_h_
