
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_resample_h_
#define __d_resample_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _resample {
    int         r_type;             /* Type of upsampling method. */
    int         r_downsample;       /* Downsampling factor. */
    int         r_upsample;         /* Upsampling factor. */
    t_sample    r_buffer;           /* Temporary for interpolation method. */
    int         r_vectorSize;       /* Size of resampled vector signal. */
    t_sample    *r_vector;          /* Resampled vector signal. */
    } t_resample;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        resample_init           (t_resample *x, t_symbol *type);
void        resample_free           (t_resample *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        resample_setRatio       (t_resample *x, int downsample, int upsample);
int         resample_isRequired     (t_resample *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        resample_getBuffer      (t_resample *x, t_sample *s, int vectorSize, int resampledSize);
t_sample    *resample_setBuffer     (t_resample *x, t_sample *s, int vectorSize, int resampledSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_sample *resample_vector (t_resample *x)
{
    return x->r_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_resample_h_
