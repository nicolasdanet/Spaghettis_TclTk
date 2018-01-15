
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

void dsp_addZeroPerform             (t_sample *s, int n);
void dsp_addScalarPerform           (t_float *f, t_sample *dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addPlusPerform             (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addSubtractPerform         (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addMultiplyPerform         (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addDividePerform           (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addMaximumPerform          (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addMinimumPerform          (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addGreaterPerform          (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void dsp_addLessPerform             (t_sample *src1, t_sample *src2, t_sample *dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addPlusScalarPerform       (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addSubtractScalarPerform   (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addMultiplyScalarPerform   (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addDivideScalarPerform     (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addMaximumScalarPerform    (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addMinimumScalarPerform    (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addGreaterScalarPerform    (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void dsp_addLessScalarPerform       (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_addCopyPerform             (PD_RESTRICTED src, PD_RESTRICTED dest, int n);
void dsp_addCopyZeroPerform         (PD_RESTRICTED src, PD_RESTRICTED dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_perform_h_
