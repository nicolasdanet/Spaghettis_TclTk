
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

/*----------------------- midiclkin--(midi F8 message )---------------------*/

static t_class *midiclkin_class;

typedef struct _midiclkin
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_midiclkin;

static void *midiclkin_new(t_float f)
{
    t_midiclkin *x = (t_midiclkin *)pd_new(midiclkin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__midiclkin);
    return (x);
}

static void midiclkin_list(t_midiclkin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float count = atom_getFloatAtIndex(1, argc, argv);
    outlet_float(x->x_outlet2, count);
    outlet_float(x->x_outlet1, value);
}

static void midiclkin_free(t_midiclkin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__midiclkin);
}

void midiclkin_setup(void)
{
    midiclkin_class = class_new(sym_midiclkin, 
        (t_newmethod)midiclkin_new, (t_method)midiclkin_free, 
            sizeof(t_midiclkin), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(midiclkin_class, midiclkin_list);
        class_setHelpName(midiclkin_class, sym_midiout);
}
