
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

typedef struct delwritectl
{
    int c_n;
    t_sample *c_vec;
    int c_phase;
} t_delwritectl;

typedef struct _sigdelwrite
{
    t_object x_obj;
    t_symbol *x_sym;
    t_float x_deltime;  /* delay in msec (added by Mathieu Bouchard) */
    t_delwritectl x_cspace;
    int x_sortno;   /* DSP sort number at which this was last put on chain */
    int x_rsortno;  /* DSP sort # for first delread or write in chain */
    int x_vecsize;  /* vector size for delread~ to use */
    t_float x_f;
} t_sigdelwrite;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sigdelwrite_checkvecsize   (t_sigdelwrite *x, int vecsize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_delay_h_
