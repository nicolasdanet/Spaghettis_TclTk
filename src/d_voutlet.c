
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
    t_voutlet *x  = (t_voutlet *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *in = x->vo_bufferRead;
    
    if (out == NULL) { out = resample_vector (&x->vo_resample); }

    while (n--) { *out = *in; *in = 0.0; out++; in++; }
    if (in == x->vo_bufferEnd) { in = x->vo_buffer; }
    
    x->vo_bufferRead = in;
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void voutlet_dsp (t_voutlet *x, t_signal **sp)
{
    if (voutlet_isSignal (x)) {
    //
    t_signal *in = sp[0];
    
    /* Note that the switch is proceeded by the block object. */
    
    if (x->vo_copyOut) { dsp_addCopyPerform (in->s_vector, x->vo_directSignal->s_vector, in->s_vectorSize); }
    else if (x->vo_directSignal) { signal_borrow (x->vo_directSignal, in); }
    else {
        dsp_add (voutlet_perform, 3, x, in->s_vector, in->s_vectorSize);
    }
    //
    }
}

void voutlet_dspProlog (t_voutlet *x,
    t_signal **signals,
    int switchable,
    int reblocked,
    int blockSize,
    int period,
    int frequency,
    int downsample,
    int upsample)
{
    if (voutlet_isSignal (x)) {
    //
    resample_setRatio (&x->vo_resample, downsample, upsample);
    
    if (reblocked) { x->vo_directSignal = NULL; }
    else {
        if (switchable) { x->vo_copyOut = 1; }
        PD_ASSERT (signals); x->vo_directSignal = signals[object_getIndexOfSignalOutlet (x->vo_outlet)];
    }
    //
    }
}

void voutlet_dspEpilog (t_voutlet *x,
    t_signal **signals,
    int switchable,
    int reblocked,
    int blockSize,
    int period,
    int frequency,
    int downsample,
    int upsample)
{
    if (voutlet_isSignal (x)) {
    //
    resample_setRatio (&x->vo_resample, downsample, upsample);

    if (reblocked) {
    //
    int phase = ugen_getPhase();
    int parentVectorSize = 1;
    int bufferSize, vectorSize = 1;
    int phaseEpilog;
    int phaseBlock;
    int bigPeriod;
    
    t_signal *s = NULL;
    
    if (signals) {
        s = signals[object_getIndexOfSignalOutlet (x->vo_outlet)];
        parentVectorSize = s->s_vectorSize;
        vectorSize = parentVectorSize * upsample / downsample;
    }
    
    bigPeriod   = PD_MAX (1, (int)(blockSize / vectorSize));
    phaseEpilog = (phase) & (bigPeriod - 1);
    phaseBlock  = (phase + period - 1) & (bigPeriod - 1) & (- period);
    
    bufferSize  = PD_MAX (blockSize, vectorSize);
    
    if (bufferSize != x->vo_bufferSize) {
        PD_MEMORY_FREE (x->vo_buffer);
        x->vo_bufferSize = bufferSize;
        x->vo_buffer     = (t_sample *)PD_MEMORY_GET (x->vo_bufferSize * sizeof (t_sample));
        x->vo_bufferEnd  = x->vo_buffer + bufferSize;
    }
    
    x->vo_bufferWrite = x->vo_buffer + (vectorSize * phaseBlock);
    x->vo_bufferRead  = x->vo_buffer + (vectorSize * phaseEpilog);
    
    if (x->vo_bufferWrite == x->vo_bufferEnd) { x->vo_bufferWrite = x->vo_buffer; }
    if (period == 1 && frequency > 1) { x->vo_hopSize = vectorSize / frequency; }
    else { 
        x->vo_hopSize = period * vectorSize;
    }
    
    if (signals) {
    //
    if (resample_isRequired (&x->vo_resample)) { dsp_add (voutlet_performEpilog, 3, x, NULL, vectorSize); } 
    else {
        dsp_add (voutlet_performEpilog, 3, x, s->s_vector, vectorSize);
    }
        
    if (resample_isRequired (&x->vo_resample)) { 
        resample_toDsp (&x->vo_resample, s->s_vector, parentVectorSize, vectorSize);
    }
    //
    }
    //
    } else if (switchable && signals) {
        t_signal *s = signals[object_getIndexOfSignalOutlet (x->vo_outlet)];
        dsp_addZeroPerform (s->s_vector, s->s_vectorSize);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
