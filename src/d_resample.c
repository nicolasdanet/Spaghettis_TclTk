
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define RESAMPLE_ZERO       0
#define RESAMPLE_HOLD       1
#define RESAMPLE_LINEAR     2
#define RESAMPLE_DEFAULT    3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *resample_performDownsampling (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    PD_RESTRICTED s2 = (t_sample *)(w[2]);
    int down = (int)(w[3]);
    int size = (int)(w[4]);
    int n = size / down;

    while (n--) { *s2 = *s1; s2++; s1 += down; }

    return (w + 5);
}

/* No aliasing. */

static t_int *resample_performUpsamplingZero (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    PD_RESTRICTED s2 = (t_sample *)(w[2]);
    int up = (int)(w[3]);
    int size = (int)(w[4]);
    
    memset (s2, 0, size * up * sizeof (t_sample));
    
    while (size--) { *s2 = *s1; s2 += up; s1++; }

    return (w + 5);
}

/* No aliasing. */

static t_int *resample_performUpsamplingHold (t_int *w)
{
    PD_RESTRICTED s1 = (t_sample *)(w[1]);
    PD_RESTRICTED s2 = (t_sample *)(w[2]);
    int up = (int)(w[3]);
    int size = (int)(w[4]);
    
    int i = up;
  
    while (i--) {
    //
    int n = size;
    t_sample *t2 = s2 + i;
    t_sample *t1 = s1;
    while (n--) { *t2 = *t1; t2 += up; t1++; }
    //
    }
    
    return (w + 5);
}

/* No aliasing. */

static t_int *resample_performUpsamplingLinear (t_int *w)
{
    PD_RESTRICTED t  = (t_sample *)(w[1]);
    PD_RESTRICTED s1 = (t_sample *)(w[2]);
    PD_RESTRICTED s2 = (t_sample *)(w[3]);
    int up = (int)(w[4]);
    int size = (int)(w[5]);
    int length = size * up;
    
    t_sample a = *t;
    t_sample b = *s1;
    int n;
    
    for (n = 0; n < length; n++) {
    //
    t_sample f = (t_sample)(n + 1) / up;
    int i = (int)f;
    t_sample fractional = f - (t_sample)i;
    
    if (fractional == 0.0) { fractional = (t_sample)1.0; }
    *s2++ = (t_sample)(fractional * b + (1.0 - fractional) * a);
    
    if (i < size) { b = *(s1 + i); }
    if (i != 0)   { a = *(s1 + i - 1); }
    //
    }

    *t = a;
  
    return (w + 6);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static void resample_addResampling (t_resample *x,
    t_sample *in,
    int inSize,
    t_sample *out,
    int outSize,
    int type)
{
    PD_ASSERT (inSize != outSize);
    PD_ASSERT (in != out);
    
    #if PD_WITH_DEBUG
    
    {
        t_float f0 = (t_float)x->r_upsample / (t_float)x->r_downsample;
        t_float f1 = (t_float)inSize / (t_float)outSize;
        t_float f2 = (t_float)outSize / (t_float)inSize;
    
        PD_ASSERT (f0 == f1 || f0 == f2);
    }
    
    #endif
    
    if (inSize > outSize) {
    //
    PD_ASSERT (!(inSize % outSize));
        
    dsp_add (resample_performDownsampling, 4, in, out, inSize / outSize, inSize);
    //
    } else {
    //
    PD_ASSERT (!(outSize % inSize));
    
    int t = outSize / inSize;
    
    switch (type) {
    //
    case RESAMPLE_DEFAULT : PD_BUG;
    case RESAMPLE_ZERO    : dsp_add (resample_performUpsamplingZero,    4, in, out, t, inSize); break;
    case RESAMPLE_HOLD    : dsp_add (resample_performUpsamplingHold,    4, in, out, t, inSize); break;
    case RESAMPLE_LINEAR  : dsp_add (resample_performUpsamplingLinear,  5, &x->r_buffer, in, out, t, inSize);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
// MARK: -

t_sample *resample_setBuffer (t_resample *x, t_sample *s, int size, int resampledSize)
{
    if (resample_setAllocateVectorIfRequired (x, s, size, resampledSize)) {
        resample_addResampling (x,
            s,
            size,
            x->r_vector,
            x->r_vectorSize,
            (x->r_type != RESAMPLE_DEFAULT) ? x->r_type : RESAMPLE_HOLD);
    }
    
    return x->r_vector;
}

void resample_getBuffer (t_resample *x, t_sample *s, int size, int resampledSize)
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
