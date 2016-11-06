
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

static t_int *zero_perform (t_int *w)
{
    t_sample *s = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    while (n--) { *s++ = 0; }
    return (w + 3);
}

static t_int *copy_perform (t_int *w)
{
    t_sample *s1 = (t_sample *)(w[1]);
    t_sample *s2 = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--) { *s2++ = *s1++; }
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *zero_vPerform (t_int *w)
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

static t_int *copy_vPerform(t_int *w)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_addZeroPerform (t_sample *s, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (zero_perform, 2, s, n); }
    else {
        dsp_add (zero_vPerform, 2, s, n);
    }
}

void dsp_addCopyPerform (t_sample *src, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (copy_perform, 3, src, dest, n); }
    else {        
        dsp_add (copy_vPerform, 3, src, dest, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
