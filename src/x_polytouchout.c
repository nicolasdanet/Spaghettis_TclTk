
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

/* -------------------------- polytouch -------------------------- */

static t_class *polytouchout_class;

typedef struct _polytouchout
{
    t_object x_obj;
    t_float x_channel;
    t_float x_pitch;
} t_polytouchout;

static void *polytouchout_new(t_float channel)
{
    t_polytouchout *x = (t_polytouchout *)pd_new(polytouchout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    x->x_pitch = 0;
    inlet_newFloat(&x->x_obj, &x->x_pitch);
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void polytouchout_float(t_polytouchout *x, t_float n)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_polyPressure((binchan >> 4), (binchan & 15), x->x_pitch, n);
}

void polytouchout_setup(void)
{
    polytouchout_class = class_new(sym_polytouchout, 
        (t_newmethod)polytouchout_new, 0,
        sizeof(t_polytouchout), 0, A_DEFFLOAT, 0);
    class_addFloat(polytouchout_class, polytouchout_float);
    class_setHelpName(polytouchout_class, sym_midiout);
}
