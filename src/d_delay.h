
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_delay_h_
#define __d_delay_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct delwrite_tilde_control {
    int                         c_phase;
    int                         c_size;
    t_sample                    *c_vector;
    } t_delwrite_tilde_control;

typedef struct _delwrite_tilde {
    t_object                    dw_obj;                     /* Must be the first. */
    t_float                     dw_f;
    t_float                     dw_delayLineInMilliseconds;
    int                         dw_buildIdentifier;
    int                         dw_buildIdentifierForMasterVectorSize;
    int                         dw_masterVectorSize;
    t_delwrite_tilde_control    dw_space;
    t_symbol                    *dw_name;
    } t_delwrite_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DELAY_EXTRA_SAMPLES     4       /* Required for 4-points interpolation. */
#define DELAY_ROUND_SAMPLES     4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void delwrite_tilde_setMasterVectorSize (t_delwrite_tilde *x, int vectorSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_delay_h_
