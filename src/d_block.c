
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

#define PROLOGCALL 2
#define EPILOGCALL 2

extern t_pdinstance *pd_this;

/* ---------------------------- block~ ----------------------------- */

/* The "block~ object maintains the containing canvas's DSP computation,
calling it at a super- or sub-multiple of the containing canvas's
calling frequency.  The block~'s creation arguments specify block size
and overlap.  Block~ does no "dsp" computation in its own right, but it
adds prolog and epilog code before and after the canvas's unit generators.

A subcanvas need not have a block~ at all; if there's none, its
ugens are simply put on the list without any prolog or epilog code.

Block~ may be invoked as switch~, in which case it also acts to switch the
subcanvas on and off.  The overall order of scheduling for a subcanvas
is thus,

    inlet and outlet prologue code (1)
    block prologue (2)
    the objects in the subcanvas, including inlets and outlets
    block epilogue (2)
    outlet epilogue code (2)

where (1) means, "if reblocked" and  (2) means, "if reblocked or switched".

If we're reblocked, the inlet prolog and outlet epilog code takes care of
overlapping and buffering to deal with vector size changes.  If we're switched
but not reblocked, the inlet prolog is not needed, and the output epilog is
ONLY run when the block is switched off; in this case the epilog code simply
copies zeros to all signal outlets.
*/

t_class *block_class;

static void block_set(t_block *x, t_float fvecsize, t_float foverlap,
    t_float fupsample);

static void *block_new(t_float fvecsize, t_float foverlap,
                       t_float fupsample)
{
    t_block *x = (t_block *)pd_new(block_class);
    x->x_phase = 0;
    x->x_period = 1;
    x->x_frequency = 1;
    x->x_switched = 0;
    x->x_switchon = 1;
    block_set(x, fvecsize, foverlap, fupsample);
    return (x);
}

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
        if ((vecsize = (1 << ilog2(calcsize))) != calcsize)
            vecsize *= 2;
    }
    else vecsize = 0;
    if (vecsize && (vecsize != (1 << ilog2(vecsize))))
    {
        post_error ("block~: vector size not a power of 2");
        vecsize = 64;
    }
    if (overlap != (1 << ilog2(overlap)))
    {
        post_error ("block~: overlap not a power of 2");
        overlap = 1;
    }
    if (downsample != (1 << ilog2(downsample)))
    {
        post_error ("block~: downsampling not a power of 2");
        downsample = 1;
    }
    if (upsample != (1 << ilog2(upsample)))
    {
        post_error ("block~: upsampling not a power of 2");
        upsample = 1;
    }

    x->x_calcsize = calcsize;
    x->x_vecsize = vecsize;
    x->x_overlap = overlap;
    x->x_upsample = upsample;
    x->x_downsample = downsample;
    dsp_resume(dspstate);
}

static void *switch_new(t_float fvecsize, t_float foverlap,
                        t_float fupsample)
{
    t_block *x = (t_block *)(block_new(fvecsize, foverlap, fupsample));
    x->x_switched = 1;
    x->x_switchon = 0;
    return (x);
}

static void block_float(t_block *x, t_float f)
{
    if (x->x_switched)
        x->x_switchon = (f != 0);
}

static void block_bang(t_block *x)
{
    if (x->x_switched && !x->x_switchon && pd_this->pd_dspChain)
    {
        t_int *ip;
        x->x_return = 1;
        for (ip = pd_this->pd_dspChain + x->x_chainonset; ip; )
            ip = (*(t_perform)(*ip))(ip);
        x->x_return = 0;
    }
    else post_error ("bang to block~ or on-state switch~ has no effect");
}

t_int *block_prolog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int phase = x->x_phase;
        /* if we're switched off, jump past the epilog code */
    if (!x->x_switchon)
        return (w + x->x_blocklength);
    if (phase)
    {
        phase++;
        if (phase == x->x_period) phase = 0;
        x->x_phase = phase;
        return (w + x->x_blocklength);  /* skip block; jump past epilog */
    }
    else
    {
        x->x_count = x->x_frequency;
        x->x_phase = (x->x_period > 1 ? 1 : 0);
        return (w + PROLOGCALL);        /* beginning of block is next ugen */
    }
}

t_int *block_epilog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int count = x->x_count - 1;
    if (x->x_return)
        return (0);
    if (!x->x_reblock)
        return (w + x->x_epiloglength + EPILOGCALL);
    if (count)
    {
        x->x_count = count;
        return (w - (x->x_blocklength -
            (PROLOGCALL + EPILOGCALL)));   /* go to ugen after prolog */
    }
    else return (w + EPILOGCALL);
}

static void block_dsp(t_block *x, t_signal **sp)
{
    /* do nothing here */
}

void block_tilde_setup(void)
{
    block_class = class_new(sym_block__tilde__, (t_newmethod)block_new, 0,
            sizeof(t_block), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addCreator((t_newmethod)switch_new, sym_switch__tilde__,
        A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(block_class, (t_method)block_set, sym_set, 
        A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(block_class, (t_method)block_dsp, sym_dsp, A_CANT, 0);
    class_addFloat(block_class, block_float);
    class_addBang(block_class, block_bang);
}
