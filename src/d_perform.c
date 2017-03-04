
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* SSE welcomed in the future. */

/* < https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_int *perform_zero (t_int *w)
{
    t_sample *s = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    
    memset (s, 0, n * sizeof (t_sample));
    
    return (w + 3);
}

static t_int *perform_scalar (t_int *w)
{
    t_float f = *(t_float *)(w[1]);
    t_sample *s = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *s++ = f; }
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* No aliasing. */

static t_int *perform_copy (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    PD_RESTRICTED s2 = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *s2 = *s1; s2++; s1++; }
    
    return (w + 4);
}

/* No aliasing. */

static t_int *perform_copyZero (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    PD_RESTRICTED s2 = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *s2 = *s1; *s1 = 0; s2++; s1++; }
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Aliasing. */

static t_int *perform_plus (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f1 = *s1++;
    t_sample f2 = *s2++;
    
    *s3++ = f1 + f2;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *perform_plusScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) { *s2 = *s1 + f; s2++; s1++; }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *perform_subtract (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f1 = *s1++;
    t_sample f2 = *s2++;
    
    *s3++ = f1 - f2;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *perform_subtractScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) { *s2 = *s1 - f; s2++; s1++; }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *perform_multiply (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f1 = *s1++;
    t_sample f2 = *s2++;
    
    *s3++ = f1 * f2;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *perform_multiplyScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) { *s2 = *s1 * f; s2++; s1++; }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *perform_divide (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f1 = *s1++;
    t_sample f2 = *s2++;
    
    if (f2) { *s3++ = f1 / f2; }
    else {
        *s3++ = 0;
    }
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *perform_divideScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    if (f) { *s2 = *s1 / f; }
    else {
        *s2 = 0;
    } 
    
    s2++; s1++;
    //
    }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *perform_maximum (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f1 = *s1++;
    t_sample f2 = *s2++;
    
    *s3++ = PD_MAX (f1, f2);
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *perform_maximumScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) { *s2 = PD_MAX (*s1, f); s2++; s1++; }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *perform_minimum (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) {
    //
    t_sample f1 = *s1++;
    t_sample f2 = *s2++;
    
    *s3++ = PD_MIN (f1, f2);
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *perform_minimumScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) { *s2 = PD_MIN (*s1, f); s2++; s1++; }
    
    return (w + 5);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *vPerform_zero (t_int *w)
{
    t_sample *s = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    
    while (n) {
    //
    s[0] = 0;
    s[1] = 0;
    s[2] = 0;
    s[3] = 0;
    s[4] = 0;
    s[5] = 0;
    s[6] = 0;
    s[7] = 0;
    n -= 8;
    s += 8;
    //
    }
    
    return (w + 3);
}

static t_int *vPerform_scalar (t_int *w)
{
    t_float f = *(t_float *)(w[1]);
    t_sample *s = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n) {
    //
    s[0] = f;
    s[1] = f;
    s[2] = f;
    s[3] = f;
    s[4] = f;
    s[5] = f;
    s[6] = f;
    s[7] = f;
    
    n -= 8;
    s += 8;
    //
    }
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* No aliasing. */

static t_int *vPerform_copy (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    PD_RESTRICTED s2 = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];
    
    s2[0] = f0;
    s2[1] = f1;
    s2[2] = f2;
    s2[3] = f3;
    s2[4] = f4;
    s2[5] = f5;
    s2[6] = f6;
    s2[7] = f7;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 4);
}

/* No aliasing. */

static t_int *vPerform_copyZero (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    PD_RESTRICTED s2 = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];
    
    s2[0] = f0;
    s2[1] = f1;
    s2[2] = f2;
    s2[3] = f3;
    s2[4] = f4;
    s2[5] = f5;
    s2[6] = f6;
    s2[7] = f7;

    s1[0] = 0;
    s1[1] = 0;
    s1[2] = 0;
    s1[3] = 0; 
    s1[4] = 0;
    s1[5] = 0;
    s1[6] = 0;
    s1[7] = 0; 
   
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Aliasing. */

static t_int *vPerform_plus (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    t_sample g0 = s2[0];
    t_sample g1 = s2[1];
    t_sample g2 = s2[2];
    t_sample g3 = s2[3];
    t_sample g4 = s2[4];
    t_sample g5 = s2[5];
    t_sample g6 = s2[6];
    t_sample g7 = s2[7];

    s3[0] = f0 + g0;
    s3[1] = f1 + g1;
    s3[2] = f2 + g2;
    s3[3] = f3 + g3;
    s3[4] = f4 + g4;
    s3[5] = f5 + g5;
    s3[6] = f6 + g6;
    s3[7] = f7 + g7;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    s3 += 8;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *vPerform_plusScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    s2[0] = f0 + g;
    s2[1] = f1 + g;
    s2[2] = f2 + g;
    s2[3] = f3 + g;
    s2[4] = f4 + g;
    s2[5] = f5 + g;
    s2[6] = f6 + g;
    s2[7] = f7 + g;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *vPerform_subtract (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    t_sample g0 = s2[0];
    t_sample g1 = s2[1];
    t_sample g2 = s2[2];
    t_sample g3 = s2[3];
    t_sample g4 = s2[4];
    t_sample g5 = s2[5];
    t_sample g6 = s2[6];
    t_sample g7 = s2[7];

    s3[0] = f0 - g0;
    s3[1] = f1 - g1;
    s3[2] = f2 - g2;
    s3[3] = f3 - g3;
    s3[4] = f4 - g4;
    s3[5] = f5 - g5;
    s3[6] = f6 - g6;
    s3[7] = f7 - g7;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    s3 += 8;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *vPerform_subtractScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    s2[0] = f0 - g;
    s2[1] = f1 - g;
    s2[2] = f2 - g;
    s2[3] = f3 - g;
    s2[4] = f4 - g;
    s2[5] = f5 - g;
    s2[6] = f6 - g;
    s2[7] = f7 - g;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *vPerform_multiply (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    t_sample g0 = s2[0];
    t_sample g1 = s2[1];
    t_sample g2 = s2[2];
    t_sample g3 = s2[3];
    t_sample g4 = s2[4];
    t_sample g5 = s2[5];
    t_sample g6 = s2[6];
    t_sample g7 = s2[7];

    s3[0] = f0 * g0;
    s3[1] = f1 * g1;
    s3[2] = f2 * g2;
    s3[3] = f3 * g3;
    s3[4] = f4 * g4;
    s3[5] = f5 * g5;
    s3[6] = f6 * g6;
    s3[7] = f7 * g7;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    s3 += 8;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *vPerform_multiplyScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    s2[0] = f0 * g;
    s2[1] = f1 * g;
    s2[2] = f2 * g;
    s2[3] = f3 * g;
    s2[4] = f4 * g;
    s2[5] = f5 * g;
    s2[6] = f6 * g;
    s2[7] = f7 * g;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *vPerform_divide (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    t_sample g0 = s2[0];
    t_sample g1 = s2[1];
    t_sample g2 = s2[2];
    t_sample g3 = s2[3];
    t_sample g4 = s2[4];
    t_sample g5 = s2[5];
    t_sample g6 = s2[6];
    t_sample g7 = s2[7];

    s3[0] = g0 ? f0 / g0 : 0;
    s3[1] = g1 ? f1 / g1 : 0;
    s3[2] = g2 ? f2 / g2 : 0;
    s3[3] = g3 ? f3 / g3 : 0;
    s3[4] = g4 ? f4 / g4 : 0;
    s3[5] = g5 ? f5 / g5 : 0;
    s3[6] = g6 ? f6 / g6 : 0;
    s3[7] = g7 ? f7 / g7 : 0;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    s3 += 8;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *vPerform_divideScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    if (g) { g = (t_float)(1.0 / g); }
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    s2[0] = f0 * g;
    s2[1] = f1 * g;
    s2[2] = f2 * g;
    s2[3] = f3 * g;
    s2[4] = f4 * g;
    s2[5] = f5 * g;
    s2[6] = f6 * g;
    s2[7] = f7 * g;
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *vPerform_maximum (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    t_sample g0 = s2[0];
    t_sample g1 = s2[1];
    t_sample g2 = s2[2];
    t_sample g3 = s2[3];
    t_sample g4 = s2[4];
    t_sample g5 = s2[5];
    t_sample g6 = s2[6];
    t_sample g7 = s2[7];

    s3[0] = PD_MAX (f0, g0);
    s3[1] = PD_MAX (f1, g1);
    s3[2] = PD_MAX (f2, g2);
    s3[3] = PD_MAX (f3, g3);
    s3[4] = PD_MAX (f4, g4);
    s3[5] = PD_MAX (f5, g5);
    s3[6] = PD_MAX (f6, g6);
    s3[7] = PD_MAX (f7, g7);
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    s3 += 8;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *vPerform_maximumScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    s2[0] = PD_MAX (f0, g);
    s2[1] = PD_MAX (f1, g);
    s2[2] = PD_MAX (f2, g);
    s2[3] = PD_MAX (f3, g);
    s2[4] = PD_MAX (f4, g);
    s2[5] = PD_MAX (f5, g);
    s2[6] = PD_MAX (f6, g);
    s2[7] = PD_MAX (f7, g);
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 5);
}

/* Aliasing. */

static t_int *vPerform_minimum (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    t_sample g0 = s2[0];
    t_sample g1 = s2[1];
    t_sample g2 = s2[2];
    t_sample g3 = s2[3];
    t_sample g4 = s2[4];
    t_sample g5 = s2[5];
    t_sample g6 = s2[6];
    t_sample g7 = s2[7];

    s3[0] = PD_MIN (f0, g0);
    s3[1] = PD_MIN (f1, g1);
    s3[2] = PD_MIN (f2, g2);
    s3[3] = PD_MIN (f3, g3);
    s3[4] = PD_MIN (f4, g4);
    s3[5] = PD_MIN (f5, g5);
    s3[6] = PD_MIN (f6, g6);
    s3[7] = PD_MIN (f7, g7);
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    s3 += 8;
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *vPerform_minimumScalar (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n) {
    //
    t_sample f0 = s1[0];
    t_sample f1 = s1[1];
    t_sample f2 = s1[2];
    t_sample f3 = s1[3];
    t_sample f4 = s1[4];
    t_sample f5 = s1[5];
    t_sample f6 = s1[6];
    t_sample f7 = s1[7];

    s2[0] = PD_MIN (f0, g);
    s2[1] = PD_MIN (f1, g);
    s2[2] = PD_MIN (f2, g);
    s2[3] = PD_MIN (f3, g);
    s2[4] = PD_MIN (f4, g);
    s2[5] = PD_MIN (f5, g);
    s2[6] = PD_MIN (f6, g);
    s2[7] = PD_MIN (f7, g);
    
    n -= 8; 
    s1 += 8;
    s2 += 8;
    //
    }
    
    return (w + 5);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_addZeroPerform (t_sample *s, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_zero, 2, s, n); }
    else {
        dsp_add (vPerform_zero, 2, s, n);
    }
}

void dsp_addScalarPerform (t_float *f, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_scalar, 3, f, dest, n); }
    else {       
        dsp_add (vPerform_scalar, 3, f, dest, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* No aliasing. */

void dsp_addCopyPerform (PD_RESTRICTED src, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_copy, 3, src, dest, n); }
    else {        
        dsp_add (vPerform_copy, 3, src, dest, n);
    }
}

/* No aliasing. */

void dsp_addCopyZeroPerform (PD_RESTRICTED src, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_copyZero, 3, src, dest, n); }
    else {        
        dsp_add (vPerform_copyZero, 3, src, dest, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Aliasing. */

void dsp_addPlusPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_plus, 4, src1, src2, dest, n); }
    else {      
        dsp_add (vPerform_plus, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addPlusScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_plusScalar, 4, src, f, dest, n); }
    else {     
        dsp_add (vPerform_plusScalar, 4, src, f, dest, n);
    }
}

/* Aliasing. */

void dsp_addSubtractPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_subtract, 4, src1, src2, dest, n); }
    else {      
        dsp_add (vPerform_subtract, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addSubtractScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_subtractScalar, 4, src, f, dest, n); }
    else {     
        dsp_add (vPerform_subtractScalar, 4, src, f, dest, n);
    }
}

/* Aliasing. */

void dsp_addMultiplyPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_multiply, 4, src1, src2, dest, n); }
    else {      
        dsp_add (vPerform_multiply, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addMultiplyScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_multiplyScalar, 4, src, f, dest, n); }
    else {     
        dsp_add (vPerform_multiplyScalar, 4, src, f, dest, n);
    }
}

/* Aliasing. */

void dsp_addDividePerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_divide, 4, src1, src2, dest, n); }
    else {      
        dsp_add (vPerform_divide, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addDivideScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_divideScalar, 4, src, f, dest, n); }
    else {     
        dsp_add (vPerform_divideScalar, 4, src, f, dest, n);
    }
}

/* Aliasing. */

void dsp_addMaximumPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_maximum, 4, src1, src2, dest, n); }
    else {      
        dsp_add (vPerform_maximum, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addMaximumScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_maximumScalar, 4, src, f, dest, n); }
    else {     
        dsp_add (vPerform_maximumScalar, 4, src, f, dest, n);
    }
}

/* Aliasing. */

void dsp_addMinimumPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_minimum, 4, src1, src2, dest, n); }
    else {      
        dsp_add (vPerform_minimum, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addMinimumScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_minimumScalar, 4, src, f, dest, n); }
    else {     
        dsp_add (vPerform_minimumScalar, 4, src, f, dest, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
