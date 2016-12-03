
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
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
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in1++ * *in2++; 
    return (w+5);
}

/* No aliasing. */

static t_int *perform_multiplyScalar (t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float f = *(t_float *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--) *out++ = *in++ * f; 
    return (w+5);
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
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
        t_sample f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
        t_sample f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

        t_sample g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
        t_sample g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];

        out[0] = f0 * g0; out[1] = f1 * g1; out[2] = f2 * g2; out[3] = f3 * g3;
        out[4] = f4 * g4; out[5] = f5 * g5; out[6] = f6 * g6; out[7] = f7 * g7;
    }
    return (w+5);
}

/* No aliasing. */

static t_int *vPerform_multiplyScalar (t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_float g = *(t_float *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    for (; n; n -= 8, in += 8, out += 8)
    {
        t_sample f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
        t_sample f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

        out[0] = f0 * g; out[1] = f1 * g; out[2] = f2 * g; out[3] = f3 * g;
        out[4] = f4 * g; out[5] = f5 * g; out[6] = f6 * g; out[7] = f7 * g;
    }
    return (w+5);
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
