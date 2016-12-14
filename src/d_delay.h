
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_delay_h_
#define __d_delay_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct delwrite_tilde_control {
    int                         c_phase;
    int                         c_size;
    t_sample                    *c_vector;
    } t_delwrite_tilde_control;

typedef struct _delwrite_tilde {
    t_object                    dw_obj;                     /* Must be the first. */
    t_float                     dw_f;
    t_float                     dw_delayInMilliseconds;
    int                         dw_buildIdentifier;
    int                         dw_buildIdentifierForMasterVectorSize;
    int                         dw_masterVectorSize;
    t_delwrite_tilde_control    dw_space;
    t_symbol                    *dw_name;
    } t_delwrite_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DELAY_EXTRA_SAMPLES     4
#define DELAY_ROUND_SAMPLES     4
#define DELAY_BLOCK_SIZE        64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void delwrite_tilde_setMasterVectorSize (t_delwrite_tilde *x, int vectorSize);
void delwrite_tilde_updateDelayLine     (t_delwrite_tilde *x, t_float sampleRate);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_delay_h_
