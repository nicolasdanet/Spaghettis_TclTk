
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_dsp_h_
#define __d_dsp_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _signal {
    int                 s_blockSize;
    int                 s_vectorSize;
    t_sample            *s_vector;
    t_float             s_sampleRate;
    int                 s_count;
    int                 s_isBorrowed;
    struct _signal      *s_borrowedFrom;
    struct _signal      *s_nextFree;
    struct _signal      *s_nextUsed;
    };

typedef struct _resample {
    int                 r_type;
    int                 r_downSample;
    int                 r_upSample;
    int                 r_vectorSize;
    int                 r_coefficientsSize;
    int                 r_bufferSize;
    t_sample            *r_vector;
    t_sample            *r_coefficients;
    t_sample            *r_buffer;
    } t_resample;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_int *(*t_perform)(t_int *args);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    dsp_addZeroPerform (t_sample *s, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    signal_setborrowed  (t_signal *sig, t_signal *sig2);
void    signal_makereusable (t_signal *sig);

t_int   *plus_perform       (t_int *args);
t_int   *copy_perform       (t_int *args);

void    dsp_add_plus        (t_sample *in1, t_sample *in2, t_sample *out, int n);
void    dsp_add_copy        (t_sample *in, t_sample *out, int n);
void    dsp_add_scalarcopy  (t_float *in, t_sample *out, int n);

void    dsp_add             (t_perform f, int n, ...);
void    pd_fft              (t_float *buffer, int npoints, int inverse);
int     ilog2               (int n);

void    mayer_fht           (t_sample *fz, int n);
void    mayer_fft           (int n, t_sample *real, t_sample *imag);
void    mayer_ifft          (int n, t_sample *real, t_sample *imag);
void    mayer_realfft       (int n, t_sample *real);
void    mayer_realifft      (int n, t_sample *real);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    resample_init       (t_resample *x, t_symbol *type);
void    resample_free       (t_resample *x);
void    resample_dsp        (t_resample *x, t_sample *in, int insize, t_sample *out, int outsize, int m);
void    resamplefrom_dsp    (t_resample *x, t_sample *in, int insize, int outsize, int m);
void    resampleto_dsp      (t_resample *x, t_sample *out, int insize, int outsize, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float q8_sqrt             (t_float);
t_float q8_rsqrt            (t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dsp_h_
