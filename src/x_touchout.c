
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

/* -------------------------- touch -------------------------- */

static t_class *touchout_class;

typedef struct _touchout
{
    t_object x_obj;
    t_float x_channel;
} t_touchout;

static void *touchout_new(t_float channel)
{
    t_touchout *x = (t_touchout *)pd_new(touchout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void touchout_float(t_touchout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_afterTouch((binchan >> 4), (binchan & 15), (int)f);
}

void touchout_setup(void)
{
    touchout_class = class_new(sym_touchout, (t_newmethod)touchout_new, 0,
        sizeof(t_touchout), 0, A_DEFFLOAT, 0);
    class_addFloat(touchout_class, touchout_float);
    class_setHelpName(touchout_class, sym_midiout);
}
