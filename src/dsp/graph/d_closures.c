
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

#include "../d_fft.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void chain_addClosure (t_chain *, t_closure *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    CLOSURES_SPACE      = 0,
    CLOSURES_VECTORS,
    CLOSURES_FFT,
    CLOSURES_RAW,
    CLOSURES_OBJECT,
    CLOSURES_BLOCK,
    CLOSURES_VINLET,
    CLOSURES_VOUTLET
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void fft_stateRelease (t_FFTState *x)
{
    if (x->s_cache) { PD_MEMORY_FREE (x->s_cache); } x->s_size = 0;
}

void fft_stateInitialize (t_FFTState *x, int n)
{
    PD_ASSERT (n >= FFT_MINIMUM && n <= FFT_MAXIMUM);
    
    if (n > x->s_size) {
    //
    fft_stateRelease (x);

    x->s_cache = (double *)PD_MEMORY_GET (n * 2 * sizeof (double));
    x->s_size  = n;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_space *space_new (void)
{
    t_space *x = (t_space *)PD_MEMORY_GET (sizeof (t_space));
    
    ((t_closure *)x)->s_type = CLOSURES_SPACE;
    
    chain_addClosure (instance_chainGetTemporary(), (t_closure *)x);
    
    return x;
}

t_sfvectors *sfvectors_new (void)
{
    t_sfvectors *x = (t_sfvectors *)PD_MEMORY_GET (sizeof (t_sfvectors));
    
    ((t_closure *)x)->s_type = CLOSURES_VECTORS;
    
    chain_addClosure (instance_chainGetTemporary(), (t_closure *)x);
    
    return x;
}

t_FFTState *fftstate_new (int n)
{
    t_FFTState *x = (t_FFTState *)PD_MEMORY_GET (sizeof (t_FFTState));
    
    ((t_closure *)x)->s_type = CLOSURES_FFT;
    
    fft_stateInitialize (x, n);
    
    chain_addClosure (instance_chainGetTemporary(), (t_closure *)x);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void garbage_newRaw (void *m)
{
    t_chain *chain = instance_chainGetCurrent();
    
    PD_ASSERT (m);
    
    if (!chain) { PD_MEMORY_FREE (m); }
    else {
    //
    t_garbage *x = (t_garbage *)PD_MEMORY_GET (sizeof (t_garbage));
    
    ((t_closure *)x)->s_type = CLOSURES_RAW;
    
    x->s_ptr = m;
    
    chain_addClosure (chain, (t_closure *)x);
    //
    }
}

void garbage_newObject (t_gobj *o)
{
    t_chain *chain = instance_chainGetCurrent();
    
    PD_ASSERT (o);
    
    if (!chain) { pd_free (cast_pd (o)); }
    else {
    //
    t_garbage *x = (t_garbage *)PD_MEMORY_GET (sizeof (t_garbage));
    
    ((t_closure *)x)->s_type = CLOSURES_OBJECT;
    
    x->s_ptr = (void *)o;
    
    if (class_hasDismissFunction (pd_class (o))) { (*class_getDismissFunction (pd_class (o))) (o); }
    
    chain_addClosure (chain, (t_closure *)x);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_blockclosure *block_newClosure (void)
{
    t_blockclosure *x = (t_blockclosure *)PD_MEMORY_GET (sizeof (t_blockclosure));
    
    ((t_closure *)x)->s_type = CLOSURES_BLOCK;
    
    chain_addClosure (instance_chainGetTemporary(), (t_closure *)x);
    
    return x;
}

t_vinletclosure *vinlet_newClosure (void)
{
    t_vinletclosure *x = (t_vinletclosure *)PD_MEMORY_GET (sizeof (t_vinletclosure));
    
    ((t_closure *)x)->s_type = CLOSURES_VINLET;
    
    chain_addClosure (instance_chainGetTemporary(), (t_closure *)x);
    
    return x;
}

t_voutletclosure *voutlet_newClosure (void)
{
    t_voutletclosure *x = (t_voutletclosure *)PD_MEMORY_GET (sizeof (t_voutletclosure));
    
    ((t_closure *)x)->s_type = CLOSURES_VOUTLET;
    
    chain_addClosure (instance_chainGetTemporary(), (t_closure *)x);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void closure_free (t_closure *x)
{
    if (x->s_type == CLOSURES_FFT) { fft_stateRelease ((t_FFTState *)x); }
    
    if (x->s_type == CLOSURES_RAW) {
        void *t = ((t_garbage *)x)->s_ptr; if (t) { PD_MEMORY_FREE (t); }
    }
    
    if (x->s_type == CLOSURES_OBJECT) {
        void *t = ((t_garbage *)x)->s_ptr; if (t) { pd_free (cast_pd (t)); }
    }
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
