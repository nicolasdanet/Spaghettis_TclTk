
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

/* -------------------------- ctlout -------------------------- */

static t_class *ctlout_class;

typedef struct _ctlout
{
    t_object x_obj;
    t_float x_ctl;
    t_float x_channel;
} t_ctlout;

static void *ctlout_new(t_float ctl, t_float channel)
{
    t_ctlout *x = (t_ctlout *)pd_new(ctlout_class);
    x->x_ctl = ctl;
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_ctl);
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void ctlout_float(t_ctlout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_controlChange((binchan >> 4),
        (binchan & 15), (int)(x->x_ctl), (int)f);
}

void ctlout_setup(void)
{
    ctlout_class = class_new(sym_ctlout, (t_newmethod)ctlout_new, 0,
        sizeof(t_ctlout), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(ctlout_class, ctlout_float);
    class_setHelpName(ctlout_class, sym_midiout);
}
