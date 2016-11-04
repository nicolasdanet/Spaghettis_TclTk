
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_int dsp_done (t_int *w)
{
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_add (t_perform f, int n, ...)
{
    int size = pd_this->pd_dspChainSize + n + 1;
    
    size_t newSize = sizeof (t_int) * size;
    size_t oldSize = sizeof (t_int) * pd_this->pd_dspChainSize;
    
    pd_this->pd_dspChain = PD_MEMORY_RESIZE (pd_this->pd_dspChain, oldSize, newSize);
    
    {
    //
    int i;
    va_list ap;

    pd_this->pd_dspChain[pd_this->pd_dspChainSize - 1] = (t_int)f;

    va_start (ap, n);
    
    for (i = 0; i < n; i++) { 
        pd_this->pd_dspChain[pd_this->pd_dspChainSize + i] = va_arg (ap, t_int);
    }
    
    va_end (ap);
    //
    }
    
    pd_this->pd_dspChain[size - 1] = (t_int)dsp_done;
    pd_this->pd_dspChainSize = size;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dsp_start (void)
{
    t_glist *glist;

    ugen_dspInitialize();
    
    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) { canvas_dspPerform (glist, 1, NULL); }
    
    pd_this->pd_dspState = 1;
}


static void dsp_stop (void)
{
    PD_ASSERT (pd_this->pd_dspState);
    
    ugen_dspRelease();
    
    pd_this->pd_dspState = 0;
}

static void dsp_notify (int n)
{
    sys_vGui ("set ::var(isDsp) %d\n", n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_state (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int n = (int)atom_getFloatAtIndex (0, argc, argv);
    
    if (n != pd_this->pd_dspState) {
    //
    if (n) { if (audio_startDSP() == PD_ERROR_NONE) { dsp_start(); } }
    else {
        dsp_stop(); audio_stopDSP();
    }
    
    dsp_notify (pd_this->pd_dspState);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_update (void)
{
    if (pd_this->pd_dspState) { dsp_start(); }
}

int dsp_suspend (void)
{
    int oldState = pd_this->pd_dspState;
    
    if (oldState) { dsp_stop(); }
    
    return oldState;
}

void dsp_resume (int oldState)
{
    if (oldState) { dsp_start(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
