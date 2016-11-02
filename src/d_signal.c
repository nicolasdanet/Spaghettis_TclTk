
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

/* Note that signals are not freed, but cached to be recycled. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SIGNAL_SLOTS    32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_signal *signal_reusable[SIGNAL_SLOTS + 1];         /* Indexed by the bloc size (power of two). */
static t_signal *signal_reusableBorrowed;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_signal *signal_new (int blockSize, t_float sampleRate)
{
    t_signal *s  = NULL;
    t_signal **t = NULL;
    
    PD_ASSERT (math_ilog2 (blockSize) <= SIGNAL_SLOTS);
    PD_ASSERT (PD_ISPOWER2 (blockSize)); 
    PD_ABORT (!PD_ISPOWER2 (blockSize));
    
    if (!blockSize) { t = &signal_reusableBorrowed; }
    else {
        t = signal_reusable + math_ilog2 (blockSize);
    }
    
    if ((s = *t)) { *t = s->s_nextReusable; }
    else {
    //
    s = (t_signal *)PD_MEMORY_GET (sizeof (t_signal));
    
    if (blockSize) {
        s->s_vector     = (t_sample *)PD_MEMORY_GET (blockSize * sizeof (t_sample));
        s->s_isBorrowed = 0;
        
    } else {
        s->s_vector     = NULL;
        s->s_isBorrowed = 1;
    }

    s->s_nextUsed = pd_this->pd_signals;
    pd_this->pd_signals = s;
    //
    }
    
    s->s_blockSize      = blockSize;
    s->s_sampleRate     = sampleRate;
    s->s_count          = 0;
    s->s_borrowedFrom   = NULL;
    
    return s;
}

void signal_free (t_signal *s)
{
    if (s->s_isBorrowed) {

        t_signal *t = s->s_borrowedFrom;
        t->s_count--; if (!t->s_count) { signal_free (t); }
        s->s_nextReusable = signal_reusableBorrowed; signal_reusableBorrowed = s;
        
    } else {
    
        int n = math_ilog2 (s->s_blockSize);
        s->s_nextReusable = signal_reusable[n]; signal_reusable[n] = s;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void signal_borrow (t_signal *s, t_signal *toBeBorrowed)
{
    s->s_borrowedFrom = toBeBorrowed;
    s->s_blockSize    = toBeBorrowed->s_blockSize;
    s->s_vector       = toBeBorrowed->s_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void signal_clean (void)
{
    t_signal *s = NULL;
    int i;
    
    while (s = pd_this->pd_signals) {

        pd_this->pd_signals = s->s_nextUsed;
        if (!s->s_isBorrowed) { PD_MEMORY_FREE (s->s_vector); } PD_MEMORY_FREE (s);
    }
    
    for (i = 0; i <= SIGNAL_SLOTS; i++) { signal_reusable[i] = NULL; }
    
    signal_reusableBorrowed = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

