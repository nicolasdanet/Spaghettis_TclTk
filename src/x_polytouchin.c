
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

/* ----------------------- polytouchin ------------------------- */

static t_class *polytouchin_class;

typedef struct _polytouchin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    t_outlet *x_outlet3;
} t_polytouchin;

static void *polytouchin_new(t_float f)
{
    t_polytouchin *x = (t_polytouchin *)pd_new(polytouchin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__polytouchin);
    return (x);
}

static void polytouchin_list(t_polytouchin *x, t_symbol *s, int argc,
    t_atom *argv)
{
    t_float pitch = atom_getFloatAtIndex(0, argc, argv);
    t_float value = atom_getFloatAtIndex(1, argc, argv);
    t_float channel = atom_getFloatAtIndex(2, argc, argv);
    if (x->x_channel != 0)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet2, pitch);
        outlet_float(x->x_outlet1, value);
    }
    else
    {
        outlet_float(x->x_outlet3, channel);
        outlet_float(x->x_outlet2, pitch);
        outlet_float(x->x_outlet1, value);
    }
}

static void polytouchin_free(t_polytouchin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__polytouchin);
}

void polytouchin_setup(void)
{
    polytouchin_class = class_new(sym_polytouchin,
        (t_newmethod)polytouchin_new, (t_method)polytouchin_free,
        sizeof(t_polytouchin), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(polytouchin_class, polytouchin_list);
    class_setHelpName(polytouchin_class, sym_midiout);
}
