
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *spigot_class;

typedef struct _spigot
{
    t_object x_obj;
    t_float x_state;
} t_spigot;

static void *spigot_new(t_float f)
{
    t_spigot *x = (t_spigot *)pd_new(spigot_class);
    inlet_newFloat(&x->x_obj, &x->x_state);
    outlet_new(&x->x_obj, 0);
    x->x_state = f;
    return (x);
}

static void spigot_bang(t_spigot *x)
{
    if (x->x_state != 0) outlet_bang(x->x_obj.te_outlet);
}

static void spigot_pointer(t_spigot *x, t_gpointer *gp)
{
    if (x->x_state != 0) outlet_pointer(x->x_obj.te_outlet, gp);
}

static void spigot_float(t_spigot *x, t_float f)
{
    if (x->x_state != 0) outlet_float(x->x_obj.te_outlet, f);
}

static void spigot_symbol(t_spigot *x, t_symbol *s)
{
    if (x->x_state != 0) outlet_symbol(x->x_obj.te_outlet, s);
}

static void spigot_list(t_spigot *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_state != 0) outlet_list(x->x_obj.te_outlet, argc, argv);
}

static void spigot_anything(t_spigot *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_state != 0) outlet_anything(x->x_obj.te_outlet, s, argc, argv);
}

void spigot_setup(void)
{
    spigot_class = class_new(sym_spigot, (t_newmethod)spigot_new, 0,
        sizeof(t_spigot), 0, A_DEFFLOAT, 0);
    class_addBang(spigot_class, spigot_bang);
    class_addPointer(spigot_class, spigot_pointer);
    class_addFloat(spigot_class, spigot_float);
    class_addSymbol(spigot_class, spigot_symbol);
    class_addList(spigot_class, spigot_list);
    class_addAnything(spigot_class, spigot_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
