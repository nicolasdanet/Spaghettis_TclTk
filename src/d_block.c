
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
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *block_class;                       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BLOCK_PROLOG    2
#define BLOCK_EPILOG    2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float block_getRatio (t_block *x)
{
    return ((t_float)x->bk_upsample / (t_float)x->bk_downsample);
}

void block_getParameters (t_block *x, 
    int *a,
    int *b,
    int *c,
    t_float *d,
    int *e,
    int *f,
    int *g,
    int *h,
    int parentBlockSize,
    t_float parentSampleRate)
{
    int switchable      = *a;
    int reblocked       = *b;
    int blockSize       = *c;
    t_float sampleRate  = *d;
    int period          = *e;
    int frequency       = *f;
    int downsample      = *g;
    int upsample        = *h;
    
    int overlap = x->bk_overlap;
    unsigned long phase = ugen_getPhase();
    
    if (x->bk_blockSize > 0) { blockSize = x->bk_blockSize; } 
        
    overlap     = PD_MIN (overlap, blockSize);
    downsample  = PD_MIN (x->bk_downsample, parentBlockSize);
    upsample    = x->bk_upsample;
    period      = PD_MAX (1, ((blockSize * downsample) / (parentBlockSize * overlap * upsample)));
    frequency   = PD_MAX (1, ((parentBlockSize * overlap * upsample) / (blockSize * downsample)));
    sampleRate  = parentSampleRate * overlap * (upsample / downsample);
    switchable  = x->bk_isSwitchObject;
    
    PD_ASSERT (PD_IS_POWER_2 (period));
    PD_ASSERT (PD_IS_POWER_2 (frequency));
    PD_ASSERT (period == 1 || frequency == 1);
        
    reblocked |= (overlap != 1);
    reblocked |= (blockSize != parentBlockSize);
    reblocked |= (downsample != 1);
    reblocked |= (upsample != 1);
    
    x->bk_phase       = (int)(phase & (unsigned long)(period - 1));
    x->bk_period      = period;
    x->bk_frequency   = frequency;
    x->bk_isReblocked = reblocked;

    *a = switchable;
    *b = reblocked;
    *c = blockSize;
    *d = sampleRate;
    *e = period;
    *f = frequency;
    *g = downsample;
    *h = upsample;
}

void block_setPerformLength (t_block *x, int context, int epilog)
{
    x->bk_allContextLength = context;
    x->bk_outletEpilogLength = epilog;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void block_float (t_block *x, t_float f)
{
    if (x->bk_isSwitchObject) { x->bk_isSwitchedOn = (f != 0.0); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void block_set (t_block *x, t_float f1, t_float f2, t_float f3)
{
    int blockSize   = PD_MAX (0.0, f1);
    int overlap     = PD_MAX (1.0, f2);
    int upsample    = 1;
    int downsample  = 1;
    int oldState    = dsp_suspend();
    
    if (blockSize && !PD_IS_POWER_2 (blockSize)) { blockSize = PD_NEXT_POWER_2 (blockSize); }
    if (!PD_IS_POWER_2 (overlap)) { overlap = PD_NEXT_POWER_2 (overlap); }
    
    if (f3 > 0.0) {
    //
    if (f3 >= 1.0) { upsample = (int)f3; downsample = 1; }
    else {
        upsample = 1; downsample = (int)(1.0 / f3);
    }
        
    if (!PD_IS_POWER_2 (downsample) || !PD_IS_POWER_2 (upsample)) {
        warning_invalid (sym_block__tilde__, sym_resampling);
        downsample = 1; upsample = 1;
    }
    //
    }

    x->bk_blockSize  = blockSize;
    x->bk_overlap    = overlap;
    x->bk_downsample = downsample;
    x->bk_upsample   = upsample;
    
    dsp_resume (oldState);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void block_dsp (t_block *x, t_signal **sp)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Perform the context only one time (the first) over the period. */
/* By-pass it the rest of the times (or either all the time if it is switched). */

t_int *block_performProlog (t_int *w)
{
    t_block *x = (t_block *)w[1];
    
    if (x->bk_isSwitchedOn) {
    //
    if (x->bk_phase) { x->bk_phase++; if (x->bk_phase == x->bk_period) { x->bk_phase = 0; } }
    else {
        x->bk_count = x->bk_frequency;
        x->bk_phase = x->bk_period > 1 ? 1 : 0;

        return (w + BLOCK_PROLOG);
    }
    //
    }
    
    return (w + x->bk_allContextLength);    /* Go to the outlet epilog. */
}

/* Perform the context several time according to the frequency. */

t_int *block_performEpilog (t_int *w)
{
    t_block *x = (t_block *)w[1];
    
    if (x->bk_isReblocked) {
    //
    if (x->bk_count - 1) {
        x->bk_count--; return (w - (x->bk_allContextLength - (BLOCK_PROLOG + BLOCK_EPILOG)));
    } else {
        return (w + BLOCK_EPILOG);
    }
    //
    }
    
    return (w + BLOCK_EPILOG + x->bk_outletEpilogLength);   /* By-pass the outlet epilog. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *block_new (t_float blockSize, t_float overlap, t_float upsample)
{
    t_block *x = (t_block *)pd_new (block_class);
    
    x->bk_phase             = 0;
    x->bk_period            = 1;
    x->bk_frequency         = 1;
    x->bk_isSwitchObject    = 0;
    x->bk_isSwitchedOn      = 1;
    
    block_set (x, blockSize, overlap, upsample);
    
    return x;
}

static void *block_newSwitch (t_float blockSize, t_float overlap, t_float upsample)
{
    t_block *x = block_new (blockSize, overlap, upsample);
    
    x->bk_isSwitchObject    = 1;
    x->bk_isSwitchedOn      = 0;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void block_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_block__tilde__,
            (t_newmethod)block_new,
            NULL,
            sizeof (t_block),
            CLASS_DEFAULT, 
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addCreator ((t_newmethod)block_newSwitch,
            sym_switch__tilde__, 
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addDSP (c, block_dsp);
    class_addFloat (c, block_float);
    
    class_addMethod (c, (t_method)block_set, sym_set, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    
    block_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
