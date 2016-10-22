
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

/* -------------------------- midiout -------------------------- */

static t_class *midiout_class;

typedef struct _midiout
{
    t_object x_obj;
    t_float x_portno;
} t_midiout;

static void *midiout_new(t_float portno)
{
    t_midiout *x = (t_midiout *)pd_new(midiout_class);
    if (portno <= 0) portno = 1;
    x->x_portno = portno;
    inlet_newFloat(&x->x_obj, &x->x_portno);
    return (x);
}

static void midiout_float(t_midiout *x, t_float f)
{
    midi_broadcast (x->x_portno - 1, 1, f, 0, 0);
}

void midiout_setup(void)
{
    midiout_class = class_new(sym_midiout, (t_newmethod)midiout_new, 0,
        sizeof(t_midiout), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(midiout_class, midiout_float);
    class_setHelpName(midiout_class, sym_midiout);
}
