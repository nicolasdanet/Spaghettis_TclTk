
/* Copyright (c) 1997-2017 Miller Puckette and others. */

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
    int         r_type;
    int         r_downsample;
    int         r_upsample;
    t_sample    r_buffer;
    int         r_allocatedSize;
    t_sample    *r_vector;
    } t_resample;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_sample *resample_vector (t_resample *x)
{
    return x->r_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        resample_init           (t_resample *x, t_symbol *type);
void        resample_free           (t_resample *x);
void        resample_setRatio       (t_resample *x, int downsample, int upsample);
int         resample_isRequired     (t_resample *x);
void        resample_toDsp          (t_resample *x, t_sample *s, int vectorSize, int resampledSize);
t_sample    *resample_fromDsp       (t_resample *x, t_sample *s, int vectorSize, int resampledSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _blockproperties {
    int         bp_switchable;
    int         bp_reblocked;
    int         bp_blockSize;
    t_float     bp_sampleRate;
    int         bp_period;
    int         bp_frequency;
    int         bp_downsample;
    int         bp_upsample;
    } t_blockproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _block {
    t_object    bk_obj;             /* Must be the first. */
    int         bk_blockSize;
    int         bk_overlap;
    int         bk_phase;
    int         bk_period;
    int         bk_frequency;
    int         bk_downsample;
    int         bk_upsample;
    int         bk_isSwitchObject;
    int         bk_isSwitchedOn;
    int         bk_isReblocked;
    int         bk_allContextLength;
    int         bk_outletEpilogLength;
    int         bk_count;
    } t_block;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_int   *block_performProlog        (t_int *w);
t_int   *block_performEpilog        (t_int *w);

t_float block_getRatio              (t_block *x);
void    block_setPerformsLength     (t_block *x, int allContextLength, int epilogLength);
void    block_getProperties         (t_block *x, int parentBlockSize, t_float parentSampleRate, t_blockproperties *properties);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_resample_h_
