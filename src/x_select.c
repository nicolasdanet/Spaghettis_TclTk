
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

static t_class *sel1_class;

typedef struct _sel1
{
    t_object x_obj;
    t_atom x_atom;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_sel1;

static void sel1_float(t_sel1 *x, t_float f)
{
    if (x->x_atom.a_type == A_FLOAT && f == x->x_atom.a_w.w_float)
        outlet_bang(x->x_outlet1);
    else outlet_float(x->x_outlet2, f);
}

static void sel1_symbol(t_sel1 *x, t_symbol *s)
{
    if (x->x_atom.a_type == A_SYMBOL && s == x->x_atom.a_w.w_symbol)
        outlet_bang(x->x_outlet1);
    else outlet_symbol(x->x_outlet2, s);
}

static t_class *sel2_class;

typedef struct _selectelement
{
    t_word e_w;
    t_outlet *e_outlet;
} t_selectelement;

typedef struct _sel2
{
    t_object x_obj;
    t_atomtype x_type;
    t_int x_nelement;
    t_selectelement *x_vec;
    t_outlet *x_rejectout;
} t_sel2;

static void sel2_float(t_sel2 *x, t_float f)
{
    t_selectelement *e;
    int nelement;
    if (x->x_type == A_FLOAT)
    {
        for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            if (e->e_w.w_float == f)
        {
            outlet_bang(e->e_outlet);
            return;
        }
    }
    outlet_float(x->x_rejectout, f);
}

static void sel2_symbol(t_sel2 *x, t_symbol *s)
{
    t_selectelement *e;
    int nelement;
    if (x->x_type == A_SYMBOL)
    {
        for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            if (e->e_w.w_symbol == s)
        {
            outlet_bang(e->e_outlet);
            return;
        }
    }
    outlet_symbol(x->x_rejectout, s);
}

static void sel2_free(t_sel2 *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

static void *select_new(t_symbol *s, int argc, t_atom *argv)
{
    t_atom a;
    if (argc == 0)
    {
        argc = 1;
        SET_FLOAT(&a, 0);
        argv = &a;
    }
    if (argc == 1)
    {
        t_sel1 *x = (t_sel1 *)pd_new(sel1_class);
        x->x_atom = *argv;
        x->x_outlet1 = outlet_new(&x->x_obj, &s_bang);
        if (argv->a_type == A_FLOAT)
        {
            inlet_newFloat(&x->x_obj, &x->x_atom.a_w.w_float);
            x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
        }
        else
        {
            inlet_newSymbol(&x->x_obj, &x->x_atom.a_w.w_symbol);
            x->x_outlet2 = outlet_new(&x->x_obj, &s_symbol);
        }
        return (x);
    }
    else
    {
        int n;
        t_selectelement *e;
        t_sel2 *x = (t_sel2 *)pd_new(sel2_class);
        x->x_nelement = argc;
        x->x_vec = (t_selectelement *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
        x->x_type = argv[0].a_type;
        for (n = 0, e = x->x_vec; n < argc; n++, e++)
        {
            e->e_outlet = outlet_new(&x->x_obj, &s_bang);
            if ((x->x_type = argv->a_type) == A_FLOAT)
                e->e_w.w_float = atom_getFloatAtIndex(n, argc, argv);
            else e->e_w.w_symbol = atom_getSymbolAtIndex(n, argc, argv);
        }
        x->x_rejectout = outlet_new(&x->x_obj, &s_float);
        return (x);
    }

}

void select_setup(void)
{
    sel1_class = class_new(sym_select, 0, 0,
        sizeof(t_sel1), 0, 0);
    class_addFloat(sel1_class, sel1_float);
    class_addSymbol(sel1_class, sel1_symbol);

    sel2_class = class_new(sym_select, 0, (t_method)sel2_free,
        sizeof(t_sel2), 0, 0);
    class_addFloat(sel2_class, sel2_float);
    class_addSymbol(sel2_class, sel2_symbol);

    class_addCreator((t_newmethod)select_new, sym_select,  A_GIMME, 0);
    class_addCreator((t_newmethod)select_new, sym_sel,  A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
