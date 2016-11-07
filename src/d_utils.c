
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

static t_int *utils_performZero (t_int *w)
{
    t_sample *s = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    
    while (n--) { *s++ = 0; }
    
    return (w + 3);
}

static t_int *utils_performCopy (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *s2++ = *s1++; }
    
    return (w + 4);
}

static t_int *utils_performPlus (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    t_sample *s3 = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    while (n--) { *s3++ = *s1++ + *s2++; }
    
    return (w + 5);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* < https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions > */

static t_int *utils_vPerformZero (t_int *w)
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

static t_int *utils_vPerformCopy (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
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

static t_int *utils_vPerformPlus(t_int *w)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_addZeroPerform (t_sample *s, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (utils_performZero, 2, s, n); }
    else {
        dsp_add (utils_vPerformZero, 2, s, n);
    }
}

void dsp_addCopyPerform (t_sample *src, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (utils_performCopy, 3, src, dest, n); }
    else {        
        dsp_add (utils_vPerformCopy, 3, src, dest, n);
    }
}

void dsp_addPlusPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (utils_performPlus, 4, src1, src2, dest, n); }
    else {      
        dsp_add (utils_vPerformPlus, 4, src1, src2, dest, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
