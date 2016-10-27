
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

#define MAXLOGSIG           32

extern t_pdinstance *pd_this;

static int ugen_loud;


    /* list of signals which can be reused, sorted by buffer size */
static t_signal *signal_freelist[MAXLOGSIG+1];
    /* list of reusable "borrowed" signals (which don't own sample buffers) */
static t_signal *signal_freeborrowed;

    /* call this when DSP is stopped to free all the signals */
void signal_cleanup(void)
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
    for (i = 0; i <= MAXLOGSIG; i++)
        signal_freelist[i] = 0;
    signal_freeborrowed = 0;
}

    /* mark the signal "reusable." */
void signal_makereusable(t_signal *sig)
{
    int logn = ilog2(sig->s_vectorSize);
#if 1
    t_signal *s5;
    for (s5 = signal_freeborrowed; s5; s5 = s5->s_nextFree)
    {
        if (s5 == sig)
        {
            PD_BUG;
            return;
        }
    }
    for (s5 = signal_freelist[logn]; s5; s5 = s5->s_nextFree)
    {
        if (s5 == sig)
        {
            PD_BUG;
            return;
        }
    }
#endif
    if (ugen_loud) post("free %lx: %d", sig, sig->s_isBorrowed);
    if (sig->s_isBorrowed)
    {
            /* if the signal is borrowed, decrement the borrowed-from signal's
                reference count, possibly marking it reusable too */
        t_signal *s2 = sig->s_borrowedFrom;
        if ((s2 == sig) || !s2) { PD_BUG; }
        s2->s_count--;
        if (!s2->s_count)
            signal_makereusable(s2);
        sig->s_nextFree = signal_freeborrowed;
        signal_freeborrowed = sig;
    }
    else
    {
            /* if it's a real signal (not borrowed), put it on the free list
                so we can reuse it. */
        if (signal_freelist[logn] == sig) { PD_BUG; }
        sig->s_nextFree = signal_freelist[logn];
        signal_freelist[logn] = sig;
    }
}

    /* reclaim or make an audio signal.  If n is zero, return a "borrowed"
    signal whose buffer and size will be obtained later via
    signal_setborrowed(). */

t_signal *signal_new(int n, t_float sr)
{
    int logn, n2, vecsize = 0;
    t_signal *ret, **whichlist;
    t_sample *fp;
    logn = ilog2(n);
    if (n)
    {
        if ((vecsize = (1<<logn)) != n)
            vecsize *= 2;
        if (logn > MAXLOGSIG) { PD_BUG; }
        whichlist = signal_freelist + logn;
    }
    else
        whichlist = &signal_freeborrowed;

        /* first try to reclaim one from the free list */
    if (ret = *whichlist)
        *whichlist = ret->s_nextFree;
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
    if (ugen_loud) post("new %lx: %d", ret, ret->s_isBorrowed);
    return (ret);
}

t_signal *signal_newlike(const t_signal *sig)
{
    return (signal_new(sig->s_blockSize, sig->s_sampleRate));
}

void signal_setborrowed(t_signal *sig, t_signal *sig2)
{
    if (!sig->s_isBorrowed || sig->s_borrowedFrom) { PD_BUG; }
    if (sig == sig2) { PD_BUG; }
    sig->s_borrowedFrom = sig2;
    sig->s_vector = sig2->s_vector;
    sig->s_blockSize = sig2->s_blockSize;
    sig->s_vectorSize = sig2->s_vectorSize;
}

int signal_compatible(t_signal *s1, t_signal *s2)
{
    return (s1->s_blockSize == s2->s_blockSize && s1->s_sampleRate == s2->s_sampleRate);
}
