
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_perform_h_
#define __d_perform_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addZeroPerform                     (PD_RESTRICTED dest, int n);
void dsp_addScalarPerform                   (t_float *f, PD_RESTRICTED dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addCopyPerform                     (PD_RESTRICTED src, PD_RESTRICTED dest, int n);
void dsp_addCopyZeroPerform                 (PD_RESTRICTED src, PD_RESTRICTED dest, int n);
void dsp_addInverseSquareRootPerform        (PD_RESTRICTED src, PD_RESTRICTED dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addPlusPerformAliased              (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addSubtractPerformAliased          (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addMultiplyPerformAliased          (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addDividePerformAliased            (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addMaximumPerformAliased           (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addMinimumPerformAliased           (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addGreaterPerformAliased           (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addLessPerformAliased              (t_sample *src1, t_sample *src2, t_sample *dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addPlusScalarPerform               (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addSubtractScalarPerform           (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addMultiplyScalarPerform           (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addDivideScalarPerform             (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addMaximumScalarPerform            (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addMinimumScalarPerform            (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addGreaterScalarPerform            (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addLessScalarPerform               (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_perform_h_
