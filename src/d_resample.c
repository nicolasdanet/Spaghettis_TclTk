
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
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define RESAMPLE_ZERO       0
#define RESAMPLE_HOLD       1
#define RESAMPLE_LINEAR     2
#define RESAMPLE_DEFAULT    3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int resample_setAllocateVectorIfRequired (t_resample *x, t_sample *s, int size, int resampledSize)
{
    if (size == resampledSize) {
    
        if (x->r_vector) { PD_MEMORY_FREE (x->r_vector); x->r_vectorSize = 0; x->r_vector = NULL; }
        
        x->r_vector = s;
        
        return 0;
        
    } else if (x->r_vectorSize != resampledSize) {
    
        size_t oldSize  = sizeof (t_sample) * x->r_vectorSize;
        size_t newSize  = sizeof (t_sample) * resampledSize;
        x->r_vectorSize = resampledSize;
        x->r_vector     = (t_sample *)PD_MEMORY_RESIZE (x->r_vector, oldSize, newSize);  
    }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void resample_addResampling (t_resample *x,
    t_sample *in,
    int inSize,
    t_sample *out,
    int outSize,
    int type)
{
    PD_ASSERT (inSize != outSize);

    if (inSize > outSize) {
    //
    PD_ASSERT (!(inSize % outSize));
        
    dsp_add (perform_downsampling, 4, in, out, inSize / outSize, inSize);
    //
    } else {
    //
    PD_ASSERT (!(outSize % inSize));
    
    int t = outSize / inSize;
    
    switch (type) {
    //
    case RESAMPLE_DEFAULT : PD_BUG;
    case RESAMPLE_ZERO    : dsp_add (perform_upsamplingZero,    4, in, out, t, inSize); break;
    case RESAMPLE_HOLD    : dsp_add (perform_upsamplingHold,    4, in, out, t, inSize); break;
    case RESAMPLE_LINEAR  : dsp_add (perform_upsamplingLinear,  5, &x->r_buffer, in, out, t, inSize); break;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void resample_init (t_resample *x, t_symbol *type)
{
    x->r_type = RESAMPLE_DEFAULT;
    
    if (type == sym_hold)   { x->r_type = RESAMPLE_HOLD;    }
    if (type == sym_linear) { x->r_type = RESAMPLE_LINEAR;  }
    if (type == sym_pad)    { x->r_type = RESAMPLE_ZERO;    }
    
    #if PD_WITH_LEGACY
    
    if (type == sym_lin)    { x->r_type = RESAMPLE_LINEAR;  }
    
    #endif

    x->r_downsample = 1;
    x->r_upsample   = 1;
    x->r_vectorSize = 0;
    x->r_vector     = NULL;
}

void resample_free (t_resample *x)
{
    if (x->r_vector) { PD_MEMORY_FREE (x->r_vector); x->r_vectorSize = 0; x->r_vector = NULL; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void resample_setRatio (t_resample *x, int downsample, int upsample)
{
    x->r_downsample = downsample;
    x->r_upsample   = upsample;
}

int resample_isRequired (t_resample *x)
{
    return (x->r_downsample != x->r_upsample);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_sample *resample_fromDsp (t_resample *x, t_sample *s, int size, int resampledSize)
{
    if (resample_setAllocateVectorIfRequired (x, s, size, resampledSize)) {
        resample_addResampling (x,
            s,
            size,
            x->r_vector,
            x->r_vectorSize,
            (x->r_type != RESAMPLE_DEFAULT) ? x->r_type : RESAMPLE_ZERO);
    }
    
    return x->r_vector;
}

void resample_toDsp (t_resample *x, t_sample *s, int size, int resampledSize)
{
    if (resample_setAllocateVectorIfRequired (x, s, size, resampledSize)) {
        resample_addResampling (x,
            x->r_vector,
            x->r_vectorSize,
            s,
            size,
            (x->r_type != RESAMPLE_DEFAULT) ? x->r_type : RESAMPLE_HOLD);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
