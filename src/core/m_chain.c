
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_chain *chain_new (void)
{
    t_chain *x = (t_chain *)PD_MEMORY_GET (sizeof (t_chain));
    
    return x;
}

void chain_free (t_chain *x)
{
    chain_release (x);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_int chain_done (t_int *dummy)
{
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_append (t_chain *x, t_perform f, int n, ...)
{
    int size = x->dsp_size + n + 1;
    
    PD_ASSERT (x->dsp_chain != NULL);
    
    size_t newSize = sizeof (t_int) * size;
    size_t oldSize = sizeof (t_int) * x->dsp_size;
    
    x->dsp_chain = (t_int *)PD_MEMORY_RESIZE (x->dsp_chain, oldSize, newSize);
    
    {
    //
    int i;
    va_list ap;

    x->dsp_chain[x->dsp_size - 1] = (t_int)f;

    va_start (ap, n);
    
    for (i = 0; i < n; i++) { x->dsp_chain[x->dsp_size + i] = va_arg (ap, t_int); }
    
    va_end (ap);
    //
    }
    
    x->dsp_chain[size - 1] = (t_int)chain_done; x->dsp_size = size;
}

void chain_tick (t_chain *x)
{
    t_int *t = x->dsp_chain;

    if (t) {
    //
    while (t) { t = (*(t_perform)(*t))(t); }
    
    x->dsp_phase++;
        
    // PD_LOG ("#");
    // PD_LOG_NUMBER (x->dsp_phase);
    // PD_LOG ("#");
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_addSignal (t_chain *x, t_signal *s)
{
    s->s_next = x->dsp_signals; x->dsp_signals = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_initialize (t_chain *x)
{
    x->dsp_identifier = utils_unique();
    x->dsp_size       = 1;
    x->dsp_chain      = (t_int *)PD_MEMORY_GET (sizeof (t_int));
    x->dsp_chain[0]   = (t_int)chain_done;
}

void chain_release (t_chain *x)
{
    t_signal *s = NULL;
    
    if (x->dsp_chain != NULL) { PD_MEMORY_FREE (x->dsp_chain); x->dsp_chain = NULL; }
    
    while ((s = x->dsp_signals)) {
    //
    x->dsp_signals = s->s_next;
    
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
