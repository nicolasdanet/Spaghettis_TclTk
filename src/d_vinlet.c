
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Write to buffer. */

static t_int *vinlet_performProlog (t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *out = x->vi_bufferWrite;
    
    if (out == x->vi_bufferEnd) {
        t_sample *f1 = x->vi_buffer;
        t_sample *f2 = x->vi_buffer + x->vi_hopSize;
        int shift    = x->vi_bufferSize - x->vi_hopSize;
        out -= x->vi_hopSize;
        while (shift--) { *f1++ = *f2++; }
    }

    while (n--) { *out++ = *in++; }
    
    x->vi_bufferWrite = out;
    
    return (w + 4);
}

/* Read from buffer. */

static t_int *vinlet_perform (t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *in = x->vi_bufferRead;

    while (n--) { *out++ = *in++; }
    if (in == x->vi_bufferEnd) { in = x->vi_buffer; }
    
    x->vi_bufferRead = in;
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vinlet_dspProlog (t_vinlet *x, t_signal **signals, t_blockproperties *p)
{
    if (vinlet_isSignal (x)) {
    //
    if (!p->bp_reblocked) {     /* Vector sizes are equal thus no buffering is required. */
    
        PD_ASSERT (signals); x->vi_directSignal = signals[inlet_getIndexAsSignal (x->vi_inlet)]; 

    } else {                    /* Buffering required. */
    //
    t_signal *s = NULL;
    int parentVectorSize = 1;
    int vectorSize = 1;
    int bufferSize;

    resample_setRatio (&x->vi_resample, p->bp_downsample, p->bp_upsample);
    
    x->vi_directSignal = NULL;
    
    if (signals) {
        s = signals[inlet_getIndexAsSignal (x->vi_inlet)];
        parentVectorSize = s->s_vectorSize;
        vectorSize = parentVectorSize * p->bp_upsample / p->bp_downsample;
    }

    bufferSize = PD_MAX (p->bp_blockSize, vectorSize);
    
    if (bufferSize != x->vi_bufferSize) {
        PD_MEMORY_FREE (x->vi_buffer);
        x->vi_bufferSize = bufferSize;
        x->vi_buffer     = (t_sample *)PD_MEMORY_GET (x->vi_bufferSize * sizeof (t_sample));
        x->vi_bufferEnd  = x->vi_buffer + x->vi_bufferSize;
    }
    
    if (!signals) { memset (x->vi_buffer, 0, x->vi_bufferSize * sizeof (t_sample)); }
    else {
    //
    t_sample *t = NULL;
    int phase = (int)((ugen_getPhase() - 1) & (t_phase)(p->bp_period - 1));
    
    x->vi_hopSize     = p->bp_period * vectorSize;
    x->vi_bufferWrite = x->vi_bufferEnd - (x->vi_hopSize - (phase * vectorSize));

    PD_ASSERT (x->vi_hopSize <= x->vi_bufferSize);
    
    if (!resample_isRequired (&x->vi_resample)) { t = s->s_vector; }    /* Original signal. */
    else {
        t = resample_fromDsp (&x->vi_resample, s->s_vector, parentVectorSize, vectorSize);  /* Resampled. */
    }

    dsp_add (vinlet_performProlog, 3, x, t, vectorSize);
    //
    }
    //
    }
    //
    }
}

void vinlet_dsp (t_vinlet *x, t_signal **sp)
{
    if (vinlet_isSignal (x)) {
    //
    t_signal *out = sp[0];
            
    if (x->vi_directSignal) { signal_borrow (out, x->vi_directSignal); }    /* By-pass the inlet. */
    else {
    //
    /* No phase required. */ 
    /* Submultiple read is always completed at each tick. */
    
    x->vi_bufferRead = x->vi_buffer;
    
    dsp_add (vinlet_perform, 3, x, out->s_vector, out->s_vectorSize);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
