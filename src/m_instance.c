
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pdinstance *pd_this;  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_pdinstance *pdinstance_new()
{
    t_pdinstance *x = (t_pdinstance *)PD_MEMORY_GET (sizeof (t_pdinstance));
    
    x->pd_systime           = 0.0;
    x->pd_state             = 0;
    x->pd_chainSize         = 0;
    x->pd_chain             = NULL;
    x->pd_clocks            = NULL;
    x->pd_signals           = NULL;
    x->pd_canvases          = NULL;
    x->sym_midiin           = gensym ("#midiin");
    x->sym_sysexin          = gensym ("#sysexin");
    x->sym_notein           = gensym ("#notein");
    x->sym_ctlin            = gensym ("#ctlin");
    x->sym_pgmin            = gensym ("#pgmin");
    x->sym_bendin           = gensym ("#bendin");
    x->sym_touchin          = gensym ("#touchin");
    x->sym_polytouchin      = gensym ("#polytouchin");
    x->sym_midiclkin        = gensym ("#midiclkin");
    x->sym_midirealtimein   = gensym ("#midirealtimein");
    
    return x;
}

static pdinstance_free (t_pdinstance *x)
{
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_initialize (void)
{
    pd_this = pdinstance_new();
}

void instance_release (void)
{
    pdinstance_free (pd_this);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
