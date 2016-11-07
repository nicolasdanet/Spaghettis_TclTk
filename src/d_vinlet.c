
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_int *vinlet_perform (t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *in = x->vi_bufferRead;

    while (n--) { *out++ = *in++; }
    if (in == x->vi_bufferEnd) { in = x->vi_buffer; }
    
    x->vi_bufferRead = in;
    
    return (w + 4);
}

static t_int *vinlet_performProlog (t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vinlet_dsp (t_vinlet *x, t_signal **sp)
{
    if (x->vi_buffer) {
    //
    t_signal *out = sp[0];
            
    if (x->vi_directSignal) { signal_borrow (out, x->vi_directSignal); }
    else {
        dsp_add (vinlet_perform, 3, x, out->s_vector, out->s_vectorSize);
        x->vi_bufferRead = x->vi_buffer;
    }
    //
    }
}

void vinlet_dspProlog (t_vinlet *x,
    t_signal **parentSignals,
    int vectorSize,
    int phase,
    int period,
    int frequency,
    int downSample,
    int upSample,
    int reblock,
    int switched)
{
    if (x->vi_buffer) {
    //
    x->vi_resampling.r_downSample = downSample;
    x->vi_resampling.r_upSample   = upSample;

    if (!reblock) { x->vi_directSignal = parentSignals[object_getIndexOfSignalInlet (x->vi_inlet)]; }
    else {
    //
    t_signal *signalIn = NULL;
    
    int newBufferSize;
    int oldBufferSize;
    int parentVectorSize;
    int parentVectorSizeResampled;

    int prologPhase = (phase - 1) & (period - 1);
    
    if (parentSignals) {
        signalIn                    = parentSignals[object_getIndexOfSignalInlet (x->vi_inlet)];
        parentVectorSize            = signalIn->s_vectorSize;
        parentVectorSizeResampled   = parentVectorSize * upSample / downSample;
        
    } else {
        signalIn                    = NULL;
        parentVectorSize            = 1;
        parentVectorSizeResampled   = 1;
    }

    newBufferSize = parentVectorSizeResampled;
    
    if (newBufferSize < vectorSize) { newBufferSize = vectorSize; }
    if (newBufferSize != (oldBufferSize = x->vi_bufferSize)) {
        t_sample *t = x->vi_buffer;
        PD_MEMORY_FREE (t);
        t = (t_sample *)PD_MEMORY_GET (newBufferSize * sizeof (t_sample));
        memset ((char *)t, 0, newBufferSize * sizeof (t_sample));
        x->vi_bufferSize = newBufferSize;
        x->vi_bufferEnd  = t + newBufferSize;
        x->vi_buffer     = t;
    }
    
    if (!parentSignals) { memset ((char *)x->vi_buffer, 0, newBufferSize * sizeof (t_sample)); }
    else {
    //
    x->vi_hopSize = period * parentVectorSizeResampled;
    x->vi_bufferWrite = x->vi_bufferEnd - (x->vi_hopSize - prologPhase * parentVectorSizeResampled);

    if (upSample * downSample == 1) {
        dsp_add (vinlet_performProlog, 3, x, signalIn->s_vector, parentVectorSizeResampled);
        
    } else {
        resample_fromDsp (&x->vi_resampling, 
            signalIn->s_vector,
            parentVectorSize,
            parentVectorSizeResampled,
            (x->vi_resampling.r_type == 3) ? 0 : x->vi_resampling.r_type);
            
        dsp_add (vinlet_performProlog, 3, x, x->vi_resampling.r_vector, parentVectorSizeResampled);
    }

    /* Free signal with a zero reference count. */
        
    if (!signalIn->s_count) { signal_free (signalIn); }
    //
    }
    
    x->vi_directSignal = NULL;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
