
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_signal *signal_new (t_float sampleRate, int vectorSize, int overlap)
{
    PD_ASSERT (PD_IS_POWER_2 (vectorSize)); 
    PD_ABORT (!PD_IS_POWER_2 (vectorSize));
    
    t_signal *s = (t_signal *)PD_MEMORY_GET (sizeof (t_signal));
    
    PD_ASSERT (sampleRate > 0);
    PD_ASSERT (overlap    > 0);
    
    s->s_sampleRate = sampleRate;
    s->s_vectorSize = vectorSize;
    s->s_overlap    = overlap;
    s->s_vector     = (t_sample *)PD_MEMORY_GET (vectorSize * sizeof (t_sample));
    s->s_unused     = NULL;
    
    instance_signalAdd (s);
    
    return s;
}

t_signal *signal_newWithSignal (t_signal *s)
{
    return signal_new (s->s_sampleRate, s->s_vectorSize, s->s_overlap);
}

t_signal *signal_newWithContext (t_dspcontext *c)
{
    return signal_new (c->dc_sampleRate, c->dc_blockSize, c->dc_overlap);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int signal_isCompatibleWith (t_signal *s1, t_signal *s2)
{
    int a = (s1->s_vectorSize == s2->s_vectorSize);
    int b = (s1->s_sampleRate == s2->s_sampleRate);
    int c = (s1->s_overlap    == s2->s_overlap);
    
    PD_ASSERT (a);
    PD_ASSERT (b);
    PD_ASSERT (c);
    
    return (a && b && c);
}

void signal_borrow (t_signal *s, t_signal *toBeBorrowed)
{
    PD_ASSERT (s->s_hasBorrowed == 0);
    PD_ASSERT (signal_isCompatibleWith (s, toBeBorrowed));
    
    s->s_hasBorrowed = 1;
    s->s_unused      = s->s_vector;
    s->s_vector      = toBeBorrowed->s_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float signal_getSampleRate (t_signal *s)
{
    return s->s_sampleRate;
}

int signal_getVectorSize (t_signal *s)
{
    return s->s_vectorSize;
}

int signal_getOverlap (t_signal *s)
{
    return s->s_overlap;
}

t_sample *signal_getVector (t_signal *s)
{
    return s->s_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

