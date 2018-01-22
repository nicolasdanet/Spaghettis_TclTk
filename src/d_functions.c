
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_int *perform_zero             (t_int *);
t_int *perform_scalar           (t_int *);
t_int *perform_copy             (t_int *);
t_int *perform_copyZero         (t_int *);
t_int *perform_plus             (t_int *);
t_int *perform_plusScalar       (t_int *);
t_int *perform_subtract         (t_int *);
t_int *perform_subtractScalar   (t_int *);
t_int *perform_multiply         (t_int *);
t_int *perform_multiplyScalar   (t_int *);
t_int *perform_divide           (t_int *);
t_int *perform_divideScalar     (t_int *);
t_int *perform_maximum          (t_int *);
t_int *perform_maximumScalar    (t_int *);
t_int *perform_minimum          (t_int *);
t_int *perform_minimumScalar    (t_int *);
t_int *perform_greater          (t_int *);
t_int *perform_greaterScalar    (t_int *);
t_int *perform_less             (t_int *);
t_int *perform_lessScalar       (t_int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_int *vPerform_zero            (t_int *);
t_int *vPerform_scalar          (t_int *);
t_int *vPerform_copy            (t_int *);
t_int *vPerform_copyZero        (t_int *);
t_int *vPerform_plus            (t_int *);
t_int *vPerform_plusScalar      (t_int *);
t_int *vPerform_subtract        (t_int *);
t_int *vPerform_subtractScalar  (t_int *);
t_int *vPerform_multiply        (t_int *);
t_int *vPerform_multiplyScalar  (t_int *);
t_int *vPerform_divide          (t_int *);
t_int *vPerform_divideScalar    (t_int *);
t_int *vPerform_maximum         (t_int *);
t_int *vPerform_maximumScalar   (t_int *);
t_int *vPerform_minimum         (t_int *);
t_int *vPerform_minimumScalar   (t_int *);
t_int *vPerform_greater         (t_int *);
t_int *vPerform_greaterScalar   (t_int *);
t_int *vPerform_less            (t_int *);
t_int *vPerform_lessScalar      (t_int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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

/* Aliasing. */

void dsp_addGreaterPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_greater, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_greater, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addGreaterScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_greaterScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_greaterScalar, 4, src, f, dest, n);
    }
}

/* Aliasing. */

void dsp_addLessPerform (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    if (n & 7) { dsp_add (perform_less, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_less, 4, src1, src2, dest, n);
    }
}

/* No aliasing. */

void dsp_addLessScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    if (n & 7) { dsp_add (perform_lessScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_lessScalar, 4, src, f, dest, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
