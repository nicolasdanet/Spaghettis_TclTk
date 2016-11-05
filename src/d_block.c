
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

static void block_float (t_block *x, t_float f)
{
    if (x->bk_isSwitchObject) { x->bk_isSwitchedOn = (f != 0.0); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void block_set (t_block *x, t_float f1, t_float f2, t_float f3)
{
    int blockSize   = PD_MAX (0.0, f1);
    int overlap     = PD_MAX (1.0, f2);
    int upSample    = 1;
    int downSample  = 1;
    int oldState    = dsp_suspend();
    
    if (blockSize && !PD_ISPOWER2 (blockSize)) { blockSize = PD_NEXTPOWER2 (blockSize); }
    if (!PD_ISPOWER2 (overlap)) { overlap = PD_NEXTPOWER2 (overlap); }
    
    if (f3 > 0.0) {
    //
    if (f3 >= 1.0) { upSample = (int)f3; downSample = 1; }
    else {
        upSample = 1; downSample = (int)(1.0 / f3);
    }
        
    if (!PD_ISPOWER2 (downSample) || !PD_ISPOWER2 (upSample)) {
        warning_invalid (sym_block__tilde__, sym_resampling);
        downSample = 1; upSample = 1;
    }
    //
    }

    x->bk_blockSize  = blockSize;
    x->bk_overlap    = overlap;
    x->bk_upSample   = upSample;
    x->bk_downSample = downSample;
    
    dsp_resume (oldState);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void block_dsp(t_block *x, t_signal **sp)
{
    /* do nothing here */
}

t_int *block_dspProlog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int phase = x->bk_phase;
        /* if we're switched off, jump past the epilog code */
    if (!x->bk_isSwitchedOn)
        return (w + x->bk_blockLength);
    if (phase)
    {
        phase++;
        if (phase == x->bk_period) phase = 0;
        x->bk_phase = phase;
        return (w + x->bk_blockLength);  /* skip block; jump past epilog */
    }
    else
    {
        x->bk_count = x->bk_frequency;
        x->bk_phase = (x->bk_period > 1 ? 1 : 0);
        return (w + BLOCK_PROLOG);        /* beginning of block is next ugen */
    }
}

t_int *block_dspEpilog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int count = x->bk_count - 1;
    if (!x->bk_isReblocked)
        return (w + x->bk_epilogLength + BLOCK_EPILOG);
    if (count)
    {
        x->bk_count = count;
        return (w - (x->bk_blockLength -
            (BLOCK_PROLOG + BLOCK_EPILOG)));   /* go to ugen after prolog */
    }
    else return (w + BLOCK_EPILOG);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *block_new (t_float blockSize, t_float overlap, t_float upSample)
{
    t_block *x = (t_block *)pd_new (block_class);
    
    x->bk_phase             = 0;
    x->bk_period            = 1;
    x->bk_frequency         = 1;
    x->bk_isSwitchObject    = 0;
    x->bk_isSwitchedOn      = 1;
    
    block_set (x, blockSize, overlap, upSample);
    
    return x;
}

static void *block_newSwitch (t_float blockSize, t_float overlap, t_float upSample)
{
    t_block *x = block_new (blockSize, overlap, upSample);
    
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
