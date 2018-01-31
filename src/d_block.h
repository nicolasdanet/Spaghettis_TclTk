
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_block_h_
#define __d_block_h_

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
    int         bp_downsample;              /* Downsampling factor. */
    int         bp_upsample;                /* Upsampling factor. */
    } t_blockproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that at most one value between downsample or upsample can be set. */
/* It is the resampling factor (i.e. 1, 2, 4 respectively below or above). */
/* Whereas the other is set to 1. */

typedef struct _block {
    t_object    bk_obj;                     /* Must be the first. */
    int         bk_blockSize;
    int         bk_overlap;
    int         bk_phase;
    int         bk_period;
    int         bk_frequency;
    int         bk_downsample;              /* Downsampling factor. */
    int         bk_upsample;                /* Upsampling factor. */
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

t_int       *block_performPrologue          (t_int *w);
t_int       *block_performEpilogue          (t_int *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float     block_getResamplingRatio        (t_block *x);
int         block_getBlockSize              (t_block *x);
void        block_getProperties             (t_block *x, t_blockproperties *properties);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_block_h_
