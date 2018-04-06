
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

t_int *perform_zero                 (t_int *);
t_int *perform_scalar               (t_int *);
t_int *perform_copy                 (t_int *);
t_int *perform_copyZero             (t_int *);
t_int *perform_squareRoot           (t_int *);
t_int *perform_inverseSquareRoot    (t_int *);
t_int *perform_plusAliased          (t_int *);
t_int *perform_plusScalar           (t_int *);
t_int *perform_subtractAliased      (t_int *);
t_int *perform_subtractScalar       (t_int *);
t_int *perform_multiplyAliased      (t_int *);
t_int *perform_multiplyScalar       (t_int *);
t_int *perform_divideAliased        (t_int *);
t_int *perform_divideScalar         (t_int *);
t_int *perform_maximumAliased       (t_int *);
t_int *perform_maximumScalar        (t_int *);
t_int *perform_minimumAliased       (t_int *);
t_int *perform_minimumScalar        (t_int *);
t_int *perform_greaterAliased       (t_int *);
t_int *perform_greaterScalar        (t_int *);
t_int *perform_lessAliased          (t_int *);
t_int *perform_lessScalar           (t_int *);
t_int *perform_magnitude            (t_int *);
t_int *perform_inverseMagnitude     (t_int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_int *vPerform_zero                (t_int *);
t_int *vPerform_scalar              (t_int *);
t_int *vPerform_copy                (t_int *);
t_int *vPerform_copyZero            (t_int *);
t_int *vPerform_squareRoot          (t_int *);
t_int *vPerform_inverseSquareRoot   (t_int *);
t_int *vPerform_plusAliased         (t_int *);
t_int *vPerform_plusScalar          (t_int *);
t_int *vPerform_subtractAliased     (t_int *);
t_int *vPerform_subtractScalar      (t_int *);
t_int *vPerform_multiplyAliased     (t_int *);
t_int *vPerform_multiplyScalar      (t_int *);
t_int *vPerform_divideAliased       (t_int *);
t_int *vPerform_divideScalar        (t_int *);
t_int *vPerform_maximumAliased      (t_int *);
t_int *vPerform_maximumScalar       (t_int *);
t_int *vPerform_minimumAliased      (t_int *);
t_int *vPerform_minimumScalar       (t_int *);
t_int *vPerform_greaterAliased      (t_int *);
t_int *vPerform_greaterScalar       (t_int *);
t_int *vPerform_lessAliased         (t_int *);
t_int *vPerform_lessScalar          (t_int *);
t_int *vPerform_magnitude           (t_int *);
t_int *vPerform_inverseMagnitude    (t_int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addZeroPerform (PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_zero, 2, dest, n); }
    else {
        dsp_add (vPerform_zero, 2, dest, n);
    }
}

void dsp_addScalarPerform (t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_scalar, 3, f, dest, n); }
    else {
        dsp_add (vPerform_scalar, 3, f, dest, n);
    }
}

void dsp_addCopyPerform (PD_RESTRICTED src, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_copy, 3, src, dest, n); }
    else {
        dsp_add (vPerform_copy, 3, src, dest, n);
    }
}

void dsp_addCopyZeroPerform (PD_RESTRICTED src, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_copyZero, 3, src, dest, n); }
    else {
        dsp_add (vPerform_copyZero, 3, src, dest, n);
    }
}

void dsp_addSquareRootPerform (PD_RESTRICTED src, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_squareRoot, 3, src, dest, n); }
    else {
        dsp_add (vPerform_squareRoot, 3, src, dest, n);
    }
}

void dsp_addInverseSquareRootPerform (PD_RESTRICTED src, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_inverseSquareRoot, 3, src, dest, n); }
    else {
        dsp_add (vPerform_inverseSquareRoot, 3, src, dest, n);
    }
}

void dsp_addPlusPerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_plusAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_plusAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addPlusScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_plusScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_plusScalar, 4, src, f, dest, n);
    }
}

void dsp_addSubtractPerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_subtractAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_subtractAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addSubtractScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_subtractScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_subtractScalar, 4, src, f, dest, n);
    }
}

void dsp_addMultiplyPerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_multiplyAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_multiplyAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addMultiplyScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));

    if (n & 7) { dsp_add (perform_multiplyScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_multiplyScalar, 4, src, f, dest, n);
    }
}

void dsp_addDividePerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_divideAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_divideAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addDivideScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_divideScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_divideScalar, 4, src, f, dest, n);
    }
}

void dsp_addMaximumPerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_maximumAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_maximumAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addMaximumScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_maximumScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_maximumScalar, 4, src, f, dest, n);
    }
}

void dsp_addMinimumPerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_minimumAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_minimumAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addMinimumScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_minimumScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_minimumScalar, 4, src, f, dest, n);
    }
}

void dsp_addGreaterPerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_greaterAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_greaterAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addGreaterScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_greaterScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_greaterScalar, 4, src, f, dest, n);
    }
}

void dsp_addLessPerformAliased (t_sample *src1, t_sample *src2, t_sample *dest, int n)
{
    PD_ASSERT (n > 0);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_lessAliased, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_lessAliased, 4, src1, src2, dest, n);
    }
}

void dsp_addLessScalarPerform (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_lessScalar, 4, src, f, dest, n); }
    else {
        dsp_add (vPerform_lessScalar, 4, src, f, dest, n);
    }
}

void dsp_addMagnitudePerform (PD_RESTRICTED src1, PD_RESTRICTED src2, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src1 != dest);
    PD_ASSERT (src2 != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_magnitude, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_magnitude, 4, src1, src2, dest, n);
    }
}

void dsp_addInverseMagnitudePerform (PD_RESTRICTED src1, PD_RESTRICTED src2, PD_RESTRICTED dest, int n)
{
    PD_ASSERT (n > 0);
    PD_ASSERT (src1 != dest);
    PD_ASSERT (src2 != dest);
    
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src1));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (src2));
    PD_ASSERT (!PD_MALLOC_ALIGNED || PD_IS_ALIGNED_16 (dest));
    
    if (n & 7) { dsp_add (perform_inverseMagnitude, 4, src1, src2, dest, n); }
    else {
        dsp_add (vPerform_inverseMagnitude, 4, src1, src2, dest, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
