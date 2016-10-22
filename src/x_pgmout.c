
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

/* -------------------------- pgmout -------------------------- */

static t_class *pgmout_class;

typedef struct _pgmout
{
    t_object x_obj;
    t_float x_channel;
} t_pgmout;

static void *pgmout_new(t_float channel)
{
    t_pgmout *x = (t_pgmout *)pd_new(pgmout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    inlet_newFloat(&x->x_obj, &x->x_channel);
    return (x);
}

static void pgmout_float(t_pgmout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    int n = f - 1;
    if (binchan < 0)
        binchan = 0;
    if (n < 0) n = 0;
    else if (n > 127) n = 127;
    outmidi_programChange((binchan >> 4),
        (binchan & 15), n);
}

void pgmout_setup(void)
{
    pgmout_class = class_new(sym_pgmout, (t_newmethod)pgmout_new, 0,
        sizeof(t_pgmout), 0, A_DEFFLOAT, 0);
    class_addFloat(pgmout_class, pgmout_float);
    class_setHelpName(pgmout_class, sym_midiout);
}
