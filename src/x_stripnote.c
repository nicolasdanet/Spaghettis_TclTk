
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

static t_class *stripnote_class;

typedef struct _stripnote
{
    t_object x_obj;
    t_float x_velo;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
} t_stripnote;

static void *stripnote_new(void )
{
    t_stripnote *x = (t_stripnote *)pd_new(stripnote_class);
    inlet_newFloat(&x->x_obj, &x->x_velo);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    return (x);
}
    
static void stripnote_float(t_stripnote *x, t_float f)
{
    if (!x->x_velo) return;
    outlet_float(x->x_velout, x->x_velo);
    outlet_float(x->x_pitchout, f);
}

void stripnote_setup(void)
{
    stripnote_class = class_new(sym_stripnote,
        (t_newmethod)stripnote_new, 0, sizeof(t_stripnote), 0, 0);
    class_addFloat(stripnote_class, stripnote_float);
}
