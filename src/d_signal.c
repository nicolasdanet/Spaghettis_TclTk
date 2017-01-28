
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_signal *signal_new (int vectorSize, t_float sampleRate)
{
    PD_ASSERT (PD_IS_POWER_2 (vectorSize)); 
    PD_ABORT (!PD_IS_POWER_2 (vectorSize));
    
    t_signal *s = (t_signal *)PD_MEMORY_GET (sizeof (t_signal));
    
    s->s_sampleRate = sampleRate;
    s->s_vectorSize = vectorSize;
    s->s_vector     = (t_sample *)PD_MEMORY_GET (vectorSize * sizeof (t_sample));
    s->s_unused     = NULL;
    s->s_next       = pd_this->pd_signals;
    
    pd_this->pd_signals = s;
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int signal_isCompatibleWith (t_signal *s1, t_signal *s2)
{
    return (s1->s_vectorSize == s2->s_vectorSize && s1->s_sampleRate == s2->s_sampleRate);
}

t_signal *signal_borrow (t_signal *s, t_signal *toBeBorrowed)
{
    PD_ASSERT (s->s_hasBorrowed == 0);
    
    s->s_hasBorrowed    = 1;
    s->s_unused         = s->s_vector;
    s->s_vectorSize     = toBeBorrowed->s_vectorSize;
    s->s_vector         = toBeBorrowed->s_vector;
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void signal_clean (void)
{
    t_signal *s = NULL;
    
    while ((s = pd_this->pd_signals)) {
    //
    pd_this->pd_signals = s->s_next;
    
    if (!s->s_hasBorrowed) { PD_MEMORY_FREE (s->s_vector); }
    else {
    //
    PD_ASSERT (s->s_unused); PD_MEMORY_FREE (s->s_unused);
    //
    }
    
    PD_MEMORY_FREE (s);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

