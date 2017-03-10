
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dsp_start (void)
{
    t_glist *glist;

    ugen_dspInitialize();
    
    for (glist = instance_getRoots(); glist; glist = glist->gl_next) { canvas_dspProceed (glist, 1, NULL); }
    
    instance_setDspState (1);
}


static void dsp_stop (void)
{
    PD_ASSERT (instance_getDspState());
    
    ugen_dspRelease();
    
    instance_setDspState (0);
}

static void dsp_notify (int n)
{
    sys_vGui ("set ::var(isDsp) %d\n", n);  // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_state (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int n = (int)atom_getFloatAtIndex (0, argc, argv);
    
    if (n != instance_getDspState()) {
    //
    if (n) { if (audio_start() == PD_ERROR_NONE) { dsp_start(); } }
    else {
        dsp_stop(); audio_stop();
    }
    
    dsp_notify (instance_getDspState());
    //
    }
    //
    }
}

int dsp_isRunning (void)
{
    return (instance_getDspState() != 0);
}   

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_update (void)
{
    dsp_resume (dsp_suspend());
}

int dsp_suspend (void)
{
    int oldState = instance_getDspState();
    
    if (oldState) { dsp_stop(); }
    
    return oldState;
}

void dsp_resume (int oldState)
{
    if (oldState) { dsp_start(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
