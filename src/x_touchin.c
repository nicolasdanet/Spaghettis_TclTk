
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

/* ----------------------- touchin ------------------------- */

static t_class *touchin_class;

typedef struct _touchin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_touchin;

static void *touchin_new(t_float f)
{
    t_touchin *x = (t_touchin *)pd_new(touchin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__touchin);
    return (x);
}

static void touchin_list(t_touchin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float channel = atom_getFloatAtIndex(1, argc, argv);
    if (x->x_channel)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet1, value);
    }
    else
    {
        outlet_float(x->x_outlet2, channel);
        outlet_float(x->x_outlet1, value);
    }
}

static void touchin_free(t_touchin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__touchin);
}

void touchin_setup(void)
{
    touchin_class = class_new(sym_touchin, (t_newmethod)touchin_new,
        (t_method)touchin_free, sizeof(t_touchin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(touchin_class, touchin_list);
    class_setHelpName(touchin_class, sym_midiout);
}
