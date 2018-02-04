
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Write to buffer. */
/* Notice that samples are accumulated (required in case of overlap). */

static t_int *voutlet_perform (t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *out  = x->vo_bufferWrite;
    t_sample *next = out + x->vo_hopSize;
    
    while (n--) { *out += *in; out++; in++; if (out == x->vo_bufferEnd) { out = x->vo_buffer; } }
    
    x->vo_bufferWrite = (next >= x->vo_bufferEnd) ? x->vo_buffer : next;
    
    return (w + 4);
}

/* Read from buffer. */
/* Notice that samples are zeroed (same reason as above). */

static t_int *voutlet_performEpilogue (t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *in = x->vo_bufferRead;
    
    if (out == NULL) { out = resample_vector (&x->vo_resample); }

    while (n--) { *out = *in; *in = (t_sample)0.0; out++; in++; }
    if (in == x->vo_bufferEnd) { in = x->vo_buffer; }
    
    x->vo_bufferRead = in;
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void voutlet_dspPrologue (t_voutlet *x, t_signal **signals, t_blockproperties *p)
{
    if (voutlet_isSignal (x)) {
    //
    if (p->bp_reblocked)  { x->vo_directSignal = NULL; }
    else {
    //
    if (p->bp_switchable) { x->vo_copyOut = 1; }
    
    PD_ASSERT (signals);
    
    x->vo_directSignal = signals[outlet_getIndexAsSignal (x->vo_outlet)];
    //
    }
    //
    }
}

void voutlet_dsp (t_voutlet *x, t_signal **sp)
{
    if (voutlet_isSignal (x)) {
    //
    t_signal *in = sp[0];
    
    if (x->vo_copyOut) {    /* Note that the switch off is proceeded by the "block~" object. */
    
        dsp_addCopyPerform (in->s_vector, x->vo_directSignal->s_vector, in->s_vectorSize);
        
        PD_ASSERT (in->s_vectorSize == x->vo_directSignal->s_vectorSize);
        
    } else {
        if (x->vo_directSignal) { signal_borrow (x->vo_directSignal, in); }     /* By-pass the outlet. */
        else {
            dsp_add (voutlet_perform, 3, x, in->s_vector, in->s_vectorSize);    /* Reblocked. */
        }
    }
    //
    }
}

void voutlet_dspEpilogue (t_voutlet *x, t_signal **signals, t_blockproperties *p)
{
    if (voutlet_isSignal (x)) {
    //
    if (p->bp_reblocked) {
    //
    t_signal *s = NULL;
    int parentVectorSize = 1;
    int vectorSize = 1;
    int bufferSize;
    
    resample_setRatio (&x->vo_resample, p->bp_downsample, p->bp_upsample);
    
    if (signals) {
        s = signals[outlet_getIndexAsSignal (x->vo_outlet)];
        parentVectorSize = s->s_vectorSize;
        vectorSize = parentVectorSize * p->bp_upsample / p->bp_downsample;
    }
    
    bufferSize = PD_MAX (p->bp_blockSize, vectorSize);
    
    if (bufferSize != x->vo_bufferSize) {
        PD_MEMORY_FREE (x->vo_buffer);
        x->vo_bufferSize = bufferSize;
        x->vo_buffer     = (t_sample *)PD_MEMORY_GET (x->vo_bufferSize * sizeof (t_sample));
        x->vo_bufferEnd  = x->vo_buffer + bufferSize;
    }
    
    {
    
    t_phase phase   = instance_getDspPhase();
    int period      = p->bp_period;
    int bigPeriod   = PD_MAX (1, (int)(p->bp_blockSize / vectorSize));
    int phaseRead   = (int)((phase) & (t_phase)(bigPeriod - 1));
    int phaseWrite  = (int)((phase + period - 1) & (t_phase)(- period) & (t_phase)(bigPeriod - 1));
    
    /* Variable above is next multiple of the hop size (modulo the window period). */
    /* < http://stackoverflow.com/a/1766566 > */
    /* Note that ~(n - 1) is equal to (-n) for power of 2 (assume two's complement). */
    
    x->vo_bufferRead  = x->vo_buffer + (vectorSize * phaseRead);
    x->vo_bufferWrite = x->vo_buffer + (vectorSize * phaseWrite);
    
    }
    
    if (x->vo_bufferWrite == x->vo_bufferEnd)     { x->vo_bufferWrite = x->vo_buffer; }
    if (p->bp_period == 1 && p->bp_frequency > 1) { x->vo_hopSize = vectorSize / p->bp_frequency; }
    else { 
        x->vo_hopSize = p->bp_period * vectorSize;
    }
    
    if (signals) {
    //
    if (resample_isRequired (&x->vo_resample)) { dsp_add (voutlet_performEpilogue, 3, x, NULL, vectorSize); }
    else {
        dsp_add (voutlet_performEpilogue, 3, x, s->s_vector, vectorSize);
    }
    
    /* Note that the resampled vector can be reallocated in function below. */
    /* Thus it can NOT be used above. */
    /* Fetch it later in the perform method. */

    if (resample_isRequired (&x->vo_resample)) { 
        resample_getBuffer (&x->vo_resample, s->s_vector, parentVectorSize, vectorSize);
    }
    //
    }
    //
    } else if (p->bp_switchable) {
        if (signals) {
            t_signal *s = signals[outlet_getIndexAsSignal (x->vo_outlet)];
            dsp_addZeroPerform (s->s_vector, s->s_vectorSize);
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
