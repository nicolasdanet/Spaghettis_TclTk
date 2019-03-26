
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _chain {
    t_pd        x_pd;               /* Must be the first. */
    t_id        x_identifier;
    t_phase     x_phase;
    int         x_quantum;
    int         x_size;
    t_int       *x_chain;
    t_signal    *x_signals;
    t_closure   *x_closures;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int instance_isChainSafeToDelete (t_chain *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *chain_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void closure_free (t_closure *);

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

t_id chain_getIdentifier (t_chain *x)
{
    return x->x_identifier;
}

int chain_getSize (t_chain *x)
{
    return x->x_size;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_append (t_chain *x, t_perform f, int n, ...)
{
    int size = x->x_size + n + 1;
    
    PD_ASSERT (x->x_chain != NULL);
    
    size_t newSize = sizeof (t_int) * size;
    size_t oldSize = sizeof (t_int) * x->x_size;
    
    x->x_chain = (t_int *)PD_MEMORY_RESIZE (x->x_chain, oldSize, newSize);
    
    {
    //
    int i;
    va_list ap;

    x->x_chain[x->x_size - 1] = (t_int)f;

    va_start (ap, n);
    
    for (i = 0; i < n; i++) { x->x_chain[x->x_size + i] = va_arg (ap, t_int); }
    
    va_end (ap);
    //
    }
    
    x->x_chain[size - 1] = (t_int)chain_done; x->x_size = size;
}

void chain_tick (t_chain *x)
{
    t_int *t = x->x_chain;

    if (t) {
    //
    // PD_LOG ("#");
    // PD_LOG_NUMBER (x->x_phase);
    // PD_LOG ("#");
    
    while (t) { t = (*(t_perform)(*t))(t); }
    
    x->x_phase++;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_addSignal (t_chain *x, t_signal *s)
{
    s->s_next = x->x_signals; x->x_signals = s;
}

void chain_addClosure (t_chain *x, t_closure *s)
{
    s->s_next = x->x_closures; x->x_closures = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_setQuantum (t_chain *x, int n)
{
    if (n >= INTERNAL_BLOCKSIZE) {
    //
    int t = n / INTERNAL_BLOCKSIZE; PD_ASSERT (PD_IS_POWER_2 (n));
    
    x->x_quantum = PD_MAX (x->x_quantum, t);
    //
    }
}

int chain_hasQuantumRemaining (t_chain *x)
{
    int k = ((x->x_phase % x->x_quantum) != 0); return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_chain *chain_new (void)
{
    t_chain *x = (t_chain *)pd_new (chain_class);
    
    x->x_identifier = utils_unique();
    x->x_size       = 1;
    x->x_chain      = (t_int *)PD_MEMORY_GET (sizeof (t_int));
    x->x_chain[0]   = (t_int)chain_done;
    
    chain_setQuantum (x, INTERNAL_BLOCKSIZE);
    
    return x;
}

static void chain_free (t_chain *x)
{
    t_signal *signal   = NULL;
    t_closure *closure = NULL;
    
    while ((signal = x->x_signals)) {
    //
    x->x_signals = signal->s_next;
    
    if (!signal->s_hasBorrowed) { PD_MEMORY_FREE (signal->s_vector); }
    else {
        PD_MEMORY_FREE (signal->s_unused);
    }
    
    PD_MEMORY_FREE (signal);
    //
    }
    
    while ((closure = x->x_closures)) { x->x_closures = closure->s_next; closure_free (closure); }
    
    PD_MEMORY_FREE (x->x_chain);
}

void chain_release (t_chain *x)
{
    instance_autoreleaseRegister (cast_pd (x));
}

static void chain_autorelease (t_chain *x)
{
    if (instance_isChainSafeToDelete (x)) { instance_autoreleaseProceed (cast_pd (x)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_chain,
            NULL,
            (t_method)chain_free,
            sizeof (t_chain),
            CLASS_INVISIBLE,
            A_NULL);
    
    class_addAutorelease (c, (t_method)chain_autorelease);
    
    chain_class = c;
}

void chain_destroy (void)
{
    class_free (chain_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------