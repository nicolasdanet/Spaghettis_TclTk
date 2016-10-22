
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
#include "s_midi.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* -------------------------- bendout -------------------------- */

static t_class *bendout_class;

typedef struct _bendout
{
    t_object x_obj;
    t_float x_channel;
} t_bendout;

static void *bendout_new(t_float channel)
{
    t_bendout *x = (t_bendout *)pd_new(bendout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void bendout_float(t_bendout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    int n = (int)f +  8192;
    if (binchan < 0)
        binchan = 0;
    outmidi_pitchBend((binchan >> 4), (binchan & 15), n);
}

void bendout_setup(void)
{
    bendout_class = class_new(sym_bendout, (t_newmethod)bendout_new, 0,
        sizeof(t_bendout), 0, A_DEFFLOAT, 0);
    class_addFloat(bendout_class, bendout_float);
    class_setHelpName(bendout_class, sym_midiout);
}
