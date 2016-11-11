
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

static t_int *voutlet_perform (t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *out  = x->vo_bufferWrite;
    t_sample *next = out + x->vo_hopSize;

    while (n--) { *out += *in; out++; in++; if (out == x->vo_bufferEnd) { out = x->vo_buffer; } }
    
    x->vo_bufferWrite = (next >= x->vo_bufferEnd) ? x->vo_buffer : next;
    
    return (w + 4);
}

static t_int *voutlet_performEpilog (t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *in  = x->vo_bufferRead;
    
    if (resample_needResampling (&x->vo_resample)) { out = resample_vector (&x->vo_resample); }

    while (n--) { *out = *in; *in = 0.0; out++; in++; }
    if (in == x->vo_bufferEnd) { in = x->vo_buffer; }
    
    x->vo_bufferRead = in;
    
    return (w + 4);
}

static t_int *voutlet_performEpilogWithResampling (t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    int n = (int)(w[2]);
    
    t_sample *in  = x->vo_bufferRead;
    t_sample *out = resample_vector (&x->vo_resample);

    while (n--) { *out = *in; *in = 0.0; out++; in++; }
    if (in == x->vo_bufferEnd) { in = x->vo_buffer; }
    
    x->vo_bufferRead = in;
    
    return (w + 3);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void voutlet_dsp (t_voutlet *x, t_signal **sp)
{
    if (voutlet_isSignal (x)) {
    //
    t_signal *in = sp[0];
    
    if (x->vo_copyOut) { dsp_addCopyPerform (in->s_vector, x->vo_directSignal->s_vector, in->s_vectorSize); }
    else if (x->vo_directSignal) { signal_borrow (x->vo_directSignal, in); }
    else {
        dsp_add (voutlet_perform, 3, x, in->s_vector, in->s_vectorSize);
    }
    //
    }
}

void voutlet_dspProlog (t_voutlet *x,
    t_signal **parentSignals,
    int switched,
    int reblocked,
    int blockSize,
    int phase,
    int period,
    int frequency,
    int downsample,
    int upsample)
{
    if (voutlet_isSignal (x)) {
    //
    resample_setRatio (&x->vo_resample, downsample, upsample);
    
    x->vo_copyOut = (switched && !reblocked);
    
    if (reblocked) { x->vo_directSignal = NULL; }
    else {
        PD_ASSERT (parentSignals);
        x->vo_directSignal = parentSignals[object_getIndexOfSignalOutlet (x->vo_outlet)];
    }
    //
    }
}

void voutlet_dspEpilog (t_voutlet *x,
    t_signal **parentSignals,
    int switched,
    int reblocked,
    int blockSize,
    int phase,
    int period,
    int frequency,
    int downsample,
    int upsample)
{
    if (voutlet_isSignal (x)) {
    //
    t_signal *out = NULL;
    
    resample_setRatio (&x->vo_resample, downsample, upsample);

    if (reblocked) {
    //
    int parentVectorSize;
    int parentVectorSizeResampled;
    int newBufferSize;
    int oldBufferSize;
    int newPeriod;
    int epilogPhase;
    int blockPhase;
    
    if (parentSignals) {
        out                         = parentSignals[object_getIndexOfSignalOutlet (x->vo_outlet)];
        parentVectorSize            = out->s_vectorSize;
        parentVectorSizeResampled   = parentVectorSize * upsample / downsample;
    } else {
        out                         = NULL;
        parentVectorSize            = 1;
        parentVectorSizeResampled   = 1;
    }
    
    newPeriod       = blockSize / parentVectorSizeResampled;
    newPeriod       = PD_MAX (1, newPeriod);
    epilogPhase     = phase & (newPeriod - 1);
    blockPhase      = (phase + period - 1) & (newPeriod - 1) & (-period);
    newBufferSize   = parentVectorSizeResampled;
    
    if (newBufferSize < blockSize) { newBufferSize = blockSize; }
    if (newBufferSize != (oldBufferSize = x->vo_bufferSize)) {
        t_sample *t = x->vo_buffer;
        PD_MEMORY_FREE (t);
        t = (t_sample *)PD_MEMORY_GET (newBufferSize * sizeof (t_sample));
        //memset (t, 0, newBufferSize * sizeof (t_sample));
        x->vo_bufferSize = newBufferSize;
        x->vo_bufferEnd  = t + newBufferSize;
        x->vo_buffer     = t;
    }
    
    PD_ASSERT (parentVectorSizeResampled * period <= newBufferSize);
    
    x->vo_bufferWrite = x->vo_buffer + parentVectorSizeResampled * blockPhase;
    
    if (x->vo_bufferWrite == x->vo_bufferEnd) { x->vo_bufferWrite = x->vo_buffer; }
    
    if (period == 1 && frequency > 1) { x->vo_hopSize = parentVectorSizeResampled / frequency; }
    else { 
        x->vo_hopSize = period * parentVectorSizeResampled;
    }

    if (parentSignals) {
    
        x->vo_bufferRead = x->vo_buffer + parentVectorSizeResampled * epilogPhase;
        
        if (upsample * downsample == 1) { 
            dsp_add (voutlet_performEpilog, 3, x, out->s_vector, parentVectorSizeResampled);
            
        } else {
            dsp_add (voutlet_performEpilogWithResampling, 2, x, parentVectorSizeResampled);
            resample_toDsp (&x->vo_resample, out->s_vector, parentVectorSize, parentVectorSizeResampled);
        }
    }
    //
    } else if (switched) {
    //
    if (parentSignals) {
        out = parentSignals[object_getIndexOfSignalOutlet (x->vo_outlet)];
        dsp_addZeroPerform (out->s_vector, out->s_vectorSize);
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
