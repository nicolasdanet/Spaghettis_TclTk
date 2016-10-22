
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sysexin_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sysexin
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_sysexin;

static void sysexin_list(t_sysexin *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_float(x->x_outlet2, atom_getFloatAtIndex(1, ac, av) + 1);
    outlet_float(x->x_outlet1, atom_getFloatAtIndex(0, ac, av));
}

static void *sysexin_new( void)
{
    t_sysexin *x = (t_sysexin *)pd_new(sysexin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, sym__sysexin);
    return (x);
}

static void sysexin_free(t_sysexin *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, sym__sysexin);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void sysexin_setup(void)
{
    sysexin_class = class_new(sym_sysexin, (t_newmethod)sysexin_new,
        (t_method)sysexin_free, sizeof(t_sysexin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addList(sysexin_class, sysexin_list);
    class_setHelpName(sysexin_class, sym_midiout);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
