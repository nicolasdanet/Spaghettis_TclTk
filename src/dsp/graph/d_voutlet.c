
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Write to buffer. */
/* Notice that samples are accumulated (required in case of overlap). */

static t_int *voutlet_perform (t_int *w)
{
    t_voutletclosure *c = (t_voutletclosure *)(w[1]);
    PD_RESTRICTED in    = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *out  = c->s_bufferWrite;
    t_sample *next = out + c->s_hopSize;
    
    // PD_LOG ("W");
    // PD_LOG_NUMBER (out - c->s_buffer);
    // PD_LOG ("/");
    // PD_LOG_NUMBER (n);
    
    while (n--) { *out += *in; out++; in++; if (out == c->s_bufferEnd) { out = c->s_buffer; } }
    
    c->s_bufferWrite = (next >= c->s_bufferEnd) ? c->s_buffer : next;
    
    return (w + 4);
}

/* Read from buffer. */
/* Notice that samples are zeroed (same reason as above). */

static t_int *voutlet_performEpilogue (t_int *w)
{
    t_voutletclosure *c = (t_voutletclosure *)(w[1]);

    t_sample *in  = c->s_bufferRead;
    t_sample *out = c->s_out;
    int n         = c->s_outSize;
    
    // PD_LOG ("E");
    // PD_LOG_NUMBER (in - c->s_buffer);
    // PD_LOG ("/");
    // PD_LOG_NUMBER (n);
    
    while (n--) { *out = *in; *in = 0.0; out++; in++; }
    
    if (in == c->s_bufferEnd) { in = c->s_buffer; }
    
    c->s_bufferRead = in;
    
    return (w + 2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void voutlet_dspPrologue (t_voutlet *x, t_signal **signals, t_blockproperties *p)
{
    PD_ASSERT (x->vo_closure == NULL);
    
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
    
    t_voutletclosure *c = x->vo_closure = voutlet_newClosure (cast_gobj (x));
    
    if (x->vo_copyOut) {    /* Note that the switch off is proceeded by the "block~" object. */
    
        dsp_addCopyPerform (in->s_vector, x->vo_directSignal->s_vector, in->s_vectorSize);
        
        PD_ASSERT (in->s_vectorSize == x->vo_directSignal->s_vectorSize);
        
    } else {
        if (x->vo_directSignal) { signal_borrow (x->vo_directSignal, in); }     /* By-pass the outlet. */
        else {
            dsp_add (voutlet_perform, 3, c, in->s_vector, in->s_vectorSize);    /* Reblocked. */
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
    
    resample_set (&x->vo_resample, p->bp_downsample, p->bp_upsample);
    
    t_voutletclosure *c = x->vo_closure;
        
    if (signals) {
        s = signals[outlet_getIndexAsSignal (x->vo_outlet)];
        parentVectorSize = s->s_vectorSize;
        vectorSize = parentVectorSize * p->bp_upsample / p->bp_downsample;
    }
    
    int bufferSize = PD_MAX (p->bp_blockSize, vectorSize);
    
    if (bufferSize != x->vo_bufferSize) {
        garbage_newRaw ((void *)x->vo_buffer);
        x->vo_bufferSize = bufferSize;
        x->vo_buffer     = (t_sample *)PD_MEMORY_GET (x->vo_bufferSize * sizeof (t_sample));
    }
    
    c->s_buffer      = x->vo_buffer;
    c->s_bufferRead  = x->vo_buffer;
    c->s_bufferWrite = x->vo_buffer;
    c->s_bufferSize  = x->vo_bufferSize;
    c->s_bufferEnd   = x->vo_buffer + x->vo_bufferSize;
    
    if (p->bp_period == 1 && p->bp_frequency > 1) { c->s_hopSize = vectorSize / p->bp_frequency; }
    else { 
        c->s_hopSize = p->bp_period * vectorSize;
    }
    
    // PD_LOG ("OUTLET SIZE");
    // PD_LOG_NUMBER (c->s_bufferSize);
    // PD_LOG ("OUTLET HOP");
    // PD_LOG_NUMBER (c->s_hopSize);
    
    if (signals) {
    //
    dsp_add (voutlet_performEpilogue, 1, c);    /* Must be before resampling below. */
    
    c->s_outSize = vectorSize;
    c->s_out     = s->s_vector;
    
    if (resample_isRequired (&x->vo_resample)) { 
        c->s_out = resample_getBufferOutlet (&x->vo_resample, s->s_vector, parentVectorSize, vectorSize);
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
    
    x->vo_closure = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
