
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------------------- notein ------------------------- */

static t_class *notein_class;

typedef struct _notein
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    t_outlet *x_outlet3;
} t_notein;

static void *notein_new(t_float f)
{
    t_notein *x = (t_notein *)pd_new(notein_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__notein);
    return (x);
}

static void notein_list(t_notein *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float pitch = atom_getFloatAtIndex(0, argc, argv);
    t_float velo = atom_getFloatAtIndex(1, argc, argv);
    t_float channel = atom_getFloatAtIndex(2, argc, argv);
    if (x->x_channel != 0)
    {
        if (channel != x->x_channel) return;
        outlet_float(x->x_outlet2, velo);
        outlet_float(x->x_outlet1, pitch);
    }
    else
    {
        outlet_float(x->x_outlet3, channel);
        outlet_float(x->x_outlet2, velo);
        outlet_float(x->x_outlet1, pitch);
    }
}

static void notein_free(t_notein *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__notein);
}

void notein_setup(void)
{
    notein_class = class_new(sym_notein, (t_newmethod)notein_new,
        (t_method)notein_free, sizeof(t_notein), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(notein_class, notein_list);
    class_setHelpName(notein_class, sym_midiout);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------------------- ctlin ------------------------- */

static t_class *ctlin_class;

typedef struct _ctlin
{
    t_object x_obj;
    t_float x_channel;
    t_float x_ctlno;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    t_outlet *x_outlet3;
} t_ctlin;

static void *ctlin_new(t_symbol *s, int argc, t_atom *argv)
{
    int ctlno, channel;
    t_ctlin *x = (t_ctlin *)pd_new(ctlin_class);
    if (!argc) ctlno = -1;
    else ctlno = atom_getFloatAtIndex(0, argc, argv);
    channel = atom_getFloatAtIndex(1, argc, argv);
    x->x_channel = channel;
    x->x_ctlno = ctlno;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (!channel)
    {
        if (x->x_ctlno < 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
        x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
    }
    pd_bind(&x->x_obj.te_g.g_pd, sym__ctlin);
    return (x);
}

static void ctlin_list(t_ctlin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float ctlnumber = atom_getFloatAtIndex(0, argc, argv);
    t_float value = atom_getFloatAtIndex(1, argc, argv);
    t_float channel = atom_getFloatAtIndex(2, argc, argv);
    if (x->x_ctlno >= 0 && x->x_ctlno != ctlnumber) return;
    if (x->x_channel > 0  && x->x_channel != channel) return;
    if (x->x_channel == 0) outlet_float(x->x_outlet3, channel);
    if (x->x_ctlno < 0) outlet_float(x->x_outlet2, ctlnumber);
    outlet_float(x->x_outlet1, value);
}

static void ctlin_free(t_ctlin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__ctlin);
}

void ctlin_setup(void)
{
    ctlin_class = class_new(sym_ctlin, (t_newmethod)ctlin_new, 
        (t_method)ctlin_free, sizeof(t_ctlin),
            CLASS_NOINLET, A_GIMME, 0);
    class_addList(ctlin_class, ctlin_list);
    class_setHelpName(ctlin_class, sym_midiout);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------------------- pgmin ------------------------- */

static t_class *pgmin_class;

typedef struct _pgmin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_pgmin;

static void *pgmin_new(t_float f)
{
    t_pgmin *x = (t_pgmin *)pd_new(pgmin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__pgmin);
    return (x);
}

static void pgmin_list(t_pgmin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float channel = atom_getFloatAtIndex(1, argc, argv);
    if (x->x_channel != 0)
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

static void pgmin_free(t_pgmin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__pgmin);
}

void pgmin_setup(void)
{
    pgmin_class = class_new(sym_pgmin, (t_newmethod)pgmin_new,
        (t_method)pgmin_free, sizeof(t_pgmin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(pgmin_class, pgmin_list);
    class_setHelpName(pgmin_class, sym_midiout);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------------------- bendin ------------------------- */

static t_class *bendin_class;

typedef struct _bendin
{
    t_object x_obj;
    t_float x_channel;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_bendin;

static void *bendin_new(t_float f)
{
    t_bendin *x = (t_bendin *)pd_new(bendin_class);
    x->x_channel = f;
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    if (f == 0) x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__bendin);
    return (x);
}

static void bendin_list(t_bendin *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float value = atom_getFloatAtIndex(0, argc, argv);
    t_float channel = atom_getFloatAtIndex(1, argc, argv);
    if (x->x_channel != 0)
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

static void bendin_free(t_bendin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__bendin);
}

void bendin_setup(void)
{
    bendin_class = class_new(sym_bendin, (t_newmethod)bendin_new,
        (t_method)bendin_free, sizeof(t_bendin), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(bendin_class, bendin_list);
    class_setHelpName(bendin_class, sym_midiout);
}

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
