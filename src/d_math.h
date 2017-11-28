
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_math_h_
#define __d_math_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Assumed IEEE 754 floating-point format. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that a float can be factorized into two floats. */
/* For one keep the mantissa and set the exponent to zero (i.e 0x7f with the bias). */
/* For the other keep the exponent and set the mantissa to zero. */
/* Thus the rsqrt is approximated by the product of two (with fast lookup) rsqrt. */

/* < https://en.wikipedia.org/wiki/Fast_inverse_square_root#Newton.27s_method > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Benchmark required. */

/* < http://assemblyrequired.crashworks.org/timing-square-root/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define RSQRT_MANTISSA_SIZE         1024
#define RSQRT_EXPONENTIAL_SIZE      256

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_float rsqrt_tableMantissa[];
extern t_float rsqrt_tableExponential[];

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_float rsqrt_fast (t_float f)
{
    t_rawcast32 z;

    z.z_f = f;
    
    if (z.z_f <= 0.0) { return (t_float)0.0; }
    else {
    //
    int e = (z.z_i >> 23) & (RSQRT_EXPONENTIAL_SIZE - 1);
    int m = (z.z_i >> 13) & (RSQRT_MANTISSA_SIZE - 1);
    t_float g = rsqrt_tableExponential[e] * rsqrt_tableMantissa[m];
    
    return (t_float)(1.5 * g - 0.5 * g * g * g * z.z_f);
    //
    }
}

/* Compute the square root multiplying the value with its reversed square root. */

static inline t_float sqrt_fast (t_float f)
{
    t_rawcast32 z;

    z.z_f = f;
        
    if (z.z_f <= 0.0) { return (t_float)0.0; }
    else {
    //
    int e = (z.z_i >> 23) & (RSQRT_EXPONENTIAL_SIZE - 1);
    int m = (z.z_i >> 13) & (RSQRT_MANTISSA_SIZE - 1);
    t_float g = rsqrt_tableExponential[e] * rsqrt_tableMantissa[m];
    
    return (t_float)(z.z_f * (1.5 * g - 0.5 * g * g * g * z.z_f));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_math_h_
