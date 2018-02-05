
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

t_class *block_class;                       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BLOCK_PROLOGUE  2
#define BLOCK_EPILOGUE  2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://lists.puredata.info/pipermail/pd-dev/2016-11/020878.html > */
/* < https://lists.puredata.info/pipermail/pd-list/2005-07/029490.html > */
/* < https://www.mail-archive.com/pd-list@iem.at/msg60031.html > */

t_float block_getResamplingRatio (t_block *x)
{
    return ((t_float)x->bk_upsample / (t_float)x->bk_downsample);
}

int block_getBlockSize (t_block *x)
{
    return x->bk_blockSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void block_setProperties (t_block *x, t_blockproperties *p)
{
    int parentBlockSize         = p->bp_blockSize;
    int isTopPatch              = p->bp_reblocked;
    t_float parentSampleRate    = p->bp_sampleRate;
    
    int reblocked        = isTopPatch;
    int blockSize        = (x->bk_blockSize > 0) ? x->bk_blockSize : parentBlockSize;
    int overlap          = PD_MIN (x->bk_overlap, blockSize);
    int downsample       = PD_MIN (x->bk_downsample, parentBlockSize);
    int upsample         = x->bk_upsample;
    int period           = PD_MAX (1, ((blockSize * downsample) / (parentBlockSize * overlap * upsample)));
    int frequency        = PD_MAX (1, ((parentBlockSize * overlap * upsample) / (blockSize * downsample)));
    t_float sampleRate   = parentSampleRate * overlap * (upsample / downsample);
    int switchable       = x->bk_switchable;
    
    PD_ASSERT (PD_IS_POWER_2 (period));
    PD_ASSERT (PD_IS_POWER_2 (frequency));
    PD_ASSERT (period == 1 || frequency == 1);
        
    reblocked |= (overlap    != 1);
    reblocked |= (blockSize  != parentBlockSize);
    reblocked |= (downsample != 1);
    reblocked |= (upsample   != 1);
    
    x->bk_reblocked      = reblocked;
    x->bk_phase          = (int)(instance_getDspPhase() & (t_phase)(period - 1));
    x->bk_period         = period;
    x->bk_frequency      = frequency;

    p->bp_blockSize      = blockSize;
    p->bp_overlap        = overlap;
    p->bp_downsample     = downsample;
    p->bp_upsample       = upsample;
    p->bp_switchable     = switchable;
    p->bp_reblocked      = reblocked;
    p->bp_sampleRate     = sampleRate;
    p->bp_period         = period;
    p->bp_frequency      = frequency;
}

void block_setLengthInDspChain (t_block *x, int context, int epilogue)
{
    x->bk_contextLength  = context;
    x->bk_epilogueLength = epilogue;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void block_setProceed (t_block *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float f1      = atom_getFloatAtIndex (0, argc, argv);
    t_float f2      = atom_getFloatAtIndex (1, argc, argv);
    t_float f3      = atom_getFloatAtIndex (2, argc, argv);
    int blockSize   = (int)PD_MAX (0.0, f1);
    int overlap     = (int)PD_MAX (1.0, f2);
    int upsample    = 1;
    int downsample  = 1;
    
    if (blockSize && !PD_IS_POWER_2 (blockSize)) { blockSize = (int)PD_NEXT_POWER_2 (blockSize); }
    if (!PD_IS_POWER_2 (overlap)) { overlap = (int)PD_NEXT_POWER_2 (overlap); }
    
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
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void block_set (t_block *x, t_symbol *s, int argc, t_atom *argv)
{
    int oldState = dsp_suspend();
    
    block_setProceed (x, s, argc, argv);
    
    dsp_resume (oldState);
}

static void block_float (t_block *x, t_float f)
{
    if (x->bk_switchable) { x->bk_switchedOn = (f != 0.0); }
}

static void block_dsp (t_block *x, t_signal **sp)
{

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Perform the context only one time (the first) over the period. */
/* By-pass it the rest of the times (or either all the time if it is switched). */
/* Notice that with a period of 1 (i.e NOT or smaller reblocked) it is triggered each time. */

t_int *block_performPrologue (t_int *w)
{
    t_block *x = (t_block *)w[1];
    
    if (x->bk_switchedOn) {
    //
    if (x->bk_phase) { x->bk_phase++; if (x->bk_phase == x->bk_period) { x->bk_phase = 0; } }
    else {
        x->bk_count = x->bk_frequency;
        x->bk_phase = x->bk_period > 1 ? 1 : 0;

        return (w + BLOCK_PROLOGUE);
    }
    //
    }
    
    /* Go to the outlet epilogue (to zero the signal out). */
    
    return (w + x->bk_contextLength);
}

/* Perform the context several time (according to the frequency set above). */
/* It is required for instance if the block size of a context is smaller than its parent's one. */

t_int *block_performEpilogue (t_int *w)
{
    t_block *x = (t_block *)w[1];
    
    if (x->bk_reblocked) {
    //
    if (x->bk_count - 1) {
        x->bk_count--; return (w - (x->bk_contextLength - (BLOCK_PROLOGUE + BLOCK_EPILOGUE)));
    } else {
        return (w + BLOCK_EPILOGUE);
    }
    //
    }
    
    return (w + BLOCK_EPILOGUE + x->bk_epilogueLength);   /* By-pass the outlets epilogue. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *block_new (t_symbol *s, int argc, t_atom *argv)
{
    t_block *x = (t_block *)pd_new (block_class);
    
    x->bk_phase      = 0;
    x->bk_period     = 1;
    x->bk_frequency  = 1;
    x->bk_switchable = 0;
    x->bk_switchedOn = 1;
    
    block_setProceed (x, s, argc, argv);
    
    return x;
}

static void *block_newSwitch (t_symbol *s, int argc, t_atom *argv)
{
    t_block *x = (t_block *)block_new (s, argc, argv);
    
    x->bk_switchable = 1;
    x->bk_switchedOn = 0;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void block_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_block__tilde__,
            (t_newmethod)block_new,
            NULL,
            sizeof (t_block),
            CLASS_DEFAULT, 
            A_GIMME,
            A_NULL);
            
    class_addCreator ((t_newmethod)block_newSwitch, sym_switch__tilde__, A_GIMME, A_NULL);
            
    class_addDSP (c, (t_method)block_dsp);
    class_addFloat (c, (t_method)block_float);
    
    class_addMethod (c, (t_method)block_set, sym_set, A_GIMME, A_NULL);
    
    block_class = c;
}

void block_tilde_destroy (void)
{
    class_free (block_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
