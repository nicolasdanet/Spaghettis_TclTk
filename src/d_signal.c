
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

#define SIGNAL_SLOTS    32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_signal *signal_reusable[SIGNAL_SLOTS + 1];         /* Queued by the bloc size (power of two). */
static t_signal *signal_reusableBorrowed;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_signal *signal_new (int n, t_float sr)
{
    int logn, n2, vecsize = 0;
    t_signal *ret, **whichlist;
    t_sample *fp;
    logn = math_ilog2 (n);
    if (n)
    {
        if ((vecsize = (1<<logn)) != n)
            vecsize *= 2;
        if (logn > SIGNAL_SLOTS) { PD_BUG; }
        whichlist = signal_reusable + logn;
    }
    else
        whichlist = &signal_reusableBorrowed;

        /* first try to reclaim one from the free list */
    if (ret = *whichlist)
        *whichlist = ret->s_nextReusable;
    else
    {
            /* LATER figure out what to do for out-of-space here! */
        ret = (t_signal *)PD_MEMORY_GET(sizeof *ret);
        if (n)
        {
            ret->s_vector = (t_sample *)PD_MEMORY_GET(vecsize * sizeof (*ret->s_vector));
            ret->s_isBorrowed = 0;
        }
        else
        {
            ret->s_vector = 0;
            ret->s_isBorrowed = 1;
        }
        ret->s_nextUsed = pd_this->pd_signals;
        pd_this->pd_signals = ret;
    }
    ret->s_blockSize = n;
    ret->s_vectorSize = vecsize;
    ret->s_sampleRate = sr;
    ret->s_count = 0;
    ret->s_borrowedFrom = 0;
    return (ret);
}

    /* mark the signal "reusable." */
void signal_free(t_signal *sig)
{
    int logn = math_ilog2 (sig->s_vectorSize);
#if 1
    t_signal *s5;
    for (s5 = signal_reusableBorrowed; s5; s5 = s5->s_nextReusable)
    {
        if (s5 == sig)
        {
            PD_BUG;
            return;
        }
    }
    for (s5 = signal_reusable[logn]; s5; s5 = s5->s_nextReusable)
    {
        if (s5 == sig)
        {
            PD_BUG;
            return;
        }
    }
#endif
    if (sig->s_isBorrowed)
    {
            /* if the signal is borrowed, decrement the borrowed-from signal's
                reference count, possibly marking it reusable too */
        t_signal *s2 = sig->s_borrowedFrom;
        if ((s2 == sig) || !s2) { PD_BUG; }
        s2->s_count--;
        if (!s2->s_count)
            signal_free(s2);
        sig->s_nextReusable = signal_reusableBorrowed;
        signal_reusableBorrowed = sig;
    }
    else
    {
            /* if it's a real signal (not borrowed), put it on the free list
                so we can reuse it. */
        if (signal_reusable[logn] == sig) { PD_BUG; }
        sig->s_nextReusable = signal_reusable[logn];
        signal_reusable[logn] = sig;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void signal_setborrowed(t_signal *sig, t_signal *sig2)
{
    if (!sig->s_isBorrowed || sig->s_borrowedFrom) { PD_BUG; }
    if (sig == sig2) { PD_BUG; }
    sig->s_borrowedFrom = sig2;
    sig->s_vector = sig2->s_vector;
    sig->s_blockSize = sig2->s_blockSize;
    sig->s_vectorSize = sig2->s_vectorSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void signal_release(void)
{
    t_signal **svec, *sig, *sig2;
    int i;
    while (sig = pd_this->pd_signals)
    {
        pd_this->pd_signals = sig->s_nextUsed;
        if (!sig->s_isBorrowed)
            PD_MEMORY_FREE(sig->s_vector);
        PD_MEMORY_FREE(sig);
    }
    for (i = 0; i <= SIGNAL_SLOTS; i++)
        signal_reusable[i] = 0;
    signal_reusableBorrowed = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

