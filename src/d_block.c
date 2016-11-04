
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

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *block_class;                       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BLOCK_PROLOGCALL    2
#define BLOCK_EPILOGCALL    2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void block_float(t_block *x, t_float f)
{
    if (x->bk_isSwitchObject)
        x->bk_isSwitchedOn = (f != 0);
}

static void block_bang(t_block *x)
{
    if (x->bk_isSwitchObject && !x->bk_isSwitchedOn && pd_this->pd_dspChain)
    {
        t_int *ip;
        x->bk_return = 1;
        for (ip = pd_this->pd_dspChain + x->bk_chainOnset; ip; )
            ip = (*(t_perform)(*ip))(ip);
        x->bk_return = 0;
    }
    else post_error ("bang to block~ or on-state switch~ has no effect");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void block_set(t_block *x, t_float fcalcsize, t_float foverlap,
    t_float fupsample)
{
    int upsample, downsample;
    int calcsize = fcalcsize;
    int overlap = foverlap;
    int dspstate = dsp_suspend();
    int vecsize;
    if (overlap < 1)
        overlap = 1;
    if (calcsize < 0)
        calcsize = 0;    /* this means we'll get it from parent later. */

    if (fupsample <= 0)
        upsample = downsample = 1;
    else if (fupsample >= 1) {
        upsample = fupsample;
        downsample   = 1;
    }
    else
    {
        downsample = 1.0 / fupsample;
        upsample   = 1;
    }

        /* vecsize is smallest power of 2 large enough to hold calcsize */
    if (calcsize)
    {
        if ((vecsize = (1 << math_ilog2 (calcsize))) != calcsize)
            vecsize *= 2;
    }
    else vecsize = 0;
    if (vecsize && (vecsize != (1 << math_ilog2 (vecsize))))
    {
        post_error ("block~: vector size not a power of 2");
        vecsize = 64;
    }
    if (overlap != (1 << math_ilog2 (overlap)))
    {
        post_error ("block~: overlap not a power of 2");
        overlap = 1;
    }
    if (downsample != (1 << math_ilog2 (downsample)))
    {
        post_error ("block~: downsampling not a power of 2");
        downsample = 1;
    }
    if (upsample != (1 << math_ilog2 (upsample)))
    {
        post_error ("block~: upsampling not a power of 2");
        upsample = 1;
    }

    x->bk_blockSize = calcsize;
    x->bk_vectorSize = vecsize;
    x->bk_overlap = overlap;
    x->bk_upSample = upsample;
    x->bk_downSample = downsample;
    dsp_resume(dspstate);
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
        return (w + BLOCK_PROLOGCALL);        /* beginning of block is next ugen */
    }
}

t_int *block_dspEpilog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int count = x->bk_count - 1;
    if (x->bk_return)
        return (0);
    if (!x->bk_isReblocked)
        return (w + x->bk_epilogLength + BLOCK_EPILOGCALL);
    if (count)
    {
        x->bk_count = count;
        return (w - (x->bk_blockLength -
            (BLOCK_PROLOGCALL + BLOCK_EPILOGCALL)));   /* go to ugen after prolog */
    }
    else return (w + BLOCK_EPILOGCALL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *block_new(t_float fvecsize, t_float foverlap,
                       t_float fupsample)
{
    t_block *x = (t_block *)pd_new(block_class);
    x->bk_phase = 0;
    x->bk_period = 1;
    x->bk_frequency = 1;
    x->bk_isSwitchObject = 0;
    x->bk_isSwitchedOn = 1;
    block_set(x, fvecsize, foverlap, fupsample);
    return (x);
}

static void *block_newSwitch(t_float fvecsize, t_float foverlap,
                        t_float fupsample)
{
    t_block *x = (t_block *)(block_new(fvecsize, foverlap, fupsample));
    x->bk_isSwitchObject = 1;
    x->bk_isSwitchedOn = 0;
    return (x);
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
    class_addBang (c, block_bang);
    class_addFloat (c, block_float);
    
    class_addMethod (c, (t_method)block_set, sym_set, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    
    block_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
