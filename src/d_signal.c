
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

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_signal *signal_new (int vectorSize, t_float sampleRate)
{
    PD_ASSERT (PD_ISPOWER2 (vectorSize)); 
    PD_ABORT (!PD_ISPOWER2 (vectorSize));
    
    t_signal *s = (t_signal *)PD_MEMORY_GET (sizeof (t_signal));
    
    s->s_sampleRate         = sampleRate;
    s->s_isVectorBorrowed   = (vectorSize == 0);
    s->s_vectorSize         = vectorSize;
    s->s_vector             = vectorSize ? (t_sample *)PD_MEMORY_GET (vectorSize * sizeof (t_sample)) : NULL;
    s->s_borrowedFrom       = NULL;
    s->s_next               = pd_this->pd_signals;
    
    pd_this->pd_signals = s;
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void signal_borrow (t_signal *s, t_signal *toBeBorrowed)
{
    s->s_borrowedFrom = toBeBorrowed;
    s->s_vectorSize   = toBeBorrowed->s_vectorSize;
    s->s_vector       = toBeBorrowed->s_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void signal_clean (void)
{
    t_signal *s = NULL;
    
    while (s = pd_this->pd_signals) {
    //
    pd_this->pd_signals = s->s_next;
    
    if (!s->s_isVectorBorrowed) { PD_MEMORY_FREE (s->s_vector); }
    
    PD_MEMORY_FREE (s);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

