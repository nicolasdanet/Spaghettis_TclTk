
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
    if (vinlet_isSignal (x)) {
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
    t_signal **signals,
    int switchable,
    int reblocked,
    int blockSize,
    int phase,
    int period,
    int frequency,
    int downsample,
    int upsample)
{
    if (vinlet_isSignal (x)) {
    //
    resample_setRatio (&x->vi_resample, downsample, upsample);
    
    if (reblocked) {
    //
    t_signal *s = NULL;
    int parentVectorSize = 1;
    int bufferSize, vectorSize = 1;

    x->vi_directSignal = NULL;
    
    if (signals) {
        s = signals[object_getIndexOfSignalInlet (x->vi_inlet)];
        parentVectorSize = s->s_vectorSize;
        vectorSize = parentVectorSize * upsample / downsample;
    }

    bufferSize = PD_MAX (blockSize, vectorSize);
    
    if (bufferSize != x->vi_bufferSize) {
        PD_MEMORY_FREE (x->vi_buffer);
        x->vi_bufferSize = bufferSize;
        x->vi_buffer     = (t_sample *)PD_MEMORY_GET (x->vi_bufferSize * sizeof (t_sample));
        x->vi_bufferEnd  = x->vi_buffer + x->vi_bufferSize;
    }
    
    if (!signals) { memset (x->vi_buffer, 0, x->vi_bufferSize * sizeof (t_sample)); }
    else {

        t_sample *t = NULL;
        
        x->vi_hopSize = period * vectorSize;
        x->vi_bufferWrite = x->vi_bufferEnd - (x->vi_hopSize - (((phase - 1) & (period - 1)) * vectorSize));

        if (!resample_needResampling (&x->vi_resample)) { t = s->s_vector; }
        else {
        //
        resample_fromDsp (&x->vi_resample, s->s_vector, parentVectorSize, vectorSize); 
        t = resample_vector (&x->vi_resample);
        //
        }

        dsp_add (vinlet_performProlog, 3, x, t, vectorSize);
        
        if (!s->s_count) { signal_free (s); }   /* ??? */
    }
    //
    } else { PD_ASSERT (signals); x->vi_directSignal = signals[object_getIndexOfSignalInlet (x->vi_inlet)]; }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
