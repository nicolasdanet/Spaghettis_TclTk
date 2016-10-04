
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

static t_symbol *keyname_sym;
static t_class *keyname_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _keyname
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_keyname;

static void *keyname_new( void)
{
    t_keyname *x = (t_keyname *)pd_new(keyname_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_symbol);
    pd_bind(&x->x_obj.te_g.g_pd, keyname_sym);
    return (x);
}

static void keyname_list(t_keyname *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_symbol(x->x_outlet2, atom_getSymbolAtIndex(1, ac, av));
    outlet_float(x->x_outlet1, atom_getFloatAtIndex(0, ac, av));
}

static void keyname_free(t_keyname *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, keyname_sym);
}

void keyname_setup(void)
{
    keyname_class = class_new(sym_keyname,
        (t_newmethod)keyname_new, (t_method)keyname_free,
        sizeof(t_keyname), CLASS_NOINLET, 0);
    class_addList(keyname_class, keyname_list);
    keyname_sym = sym__keyname;
    class_setHelpName(keyname_class, sym_key);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
