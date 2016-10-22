
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

/* ----------------------- midiin and sysexin ------------------------- */

static t_class *midiin_class, *sysexin_class;

typedef struct _midiin
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_midiin;

static void *midiin_new( void)
{
    t_midiin *x = (t_midiin *)pd_new(midiin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__midiin);
    return (x);
}

static void midiin_list(t_midiin *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_float(x->x_outlet2, atom_getFloatAtIndex(1, ac, av) + 1);
    outlet_float(x->x_outlet1, atom_getFloatAtIndex(0, ac, av));
}

static void midiin_free(t_midiin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__midiin);
}

static void *sysexin_new( void)
{
    t_midiin *x = (t_midiin *)pd_new(sysexin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__sysexin);
    return (x);
}

static void sysexin_free(t_midiin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__sysexin);
}

void midiin_setup(void)
{
    midiin_class = class_new(sym_midiin, (t_newmethod)midiin_new,
        (t_method)midiin_free, sizeof(t_midiin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(midiin_class, midiin_list);
    class_setHelpName(midiin_class, sym_midiout);

    sysexin_class = class_new(sym_sysexin, (t_newmethod)sysexin_new,
        (t_method)sysexin_free, sizeof(t_midiin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(sysexin_class, midiin_list);
    class_setHelpName(sysexin_class, sym_midiout);
}
