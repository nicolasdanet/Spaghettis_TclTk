
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

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
    x->pd_dspState          = 0;
    x->pd_dspChainSize      = 0;
    x->pd_dspChain          = NULL;
    x->pd_clocks            = NULL;
    x->pd_signals           = NULL;
    x->pd_roots             = NULL;
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
#pragma mark -

void instance_addToRoots (t_glist *glist)
{
    glist->gl_next = pd_this->pd_roots; pd_this->pd_roots = glist;
}

void instance_removeFromRoots (t_glist *glist)
{
    if (glist == pd_this->pd_roots) { pd_this->pd_roots = glist->gl_next; }
    else {
        t_glist *z = NULL;
        for (z = pd_this->pd_roots; z->gl_next != glist; z = z->gl_next) { }
        z->gl_next = glist->gl_next;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
