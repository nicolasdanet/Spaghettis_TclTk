
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
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void dsp_state (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int n = (int)atom_getFloatAtIndex (0, argc, argv);
    
    if (n != instance_getDspState()) {
    //
    if (n) { if (audio_start() == PD_ERROR_NONE) { instance_dspStart(); } }
    else {
        instance_dspStop(); audio_stop();
    }
    
    sys_vGui ("set ::var(isDsp) %d\n", instance_getDspState());     // --
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
    dsp_resume (dsp_suspend());
}

int dsp_suspend (void)
{
    int n = instance_getDspState(); if (n) { instance_dspStop(); } return n;
}

void dsp_resume (int n)
{
    if (n) { instance_dspStart(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
