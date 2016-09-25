/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* arithmetic: binops ala C language.  The 4 functions and relationals are
done on floats; the logical and bitwise binops convert their
inputs to int and their outputs back to float. */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ------------- binop3: &, |, &&, ||, <<, >>, %, mod, div ------------------ */

static void *binop3_new(t_class *fixclass, t_float f)
{
    t_binop *x = (t_binop *)pd_new(fixclass);
    outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    x->x_f1 = 0;
    x->x_f2 = f;
    return (x);
}

/* --------------------------- & ---------------------------- */

static t_class *binop3_ba_class;

static void *binop3_ba_new(t_float f)
{
    return (binop3_new(binop3_ba_class, f));
}



/* --------------------------- && ---------------------------- */

static t_class *binop3_la_class;

static void *binop3_la_new(t_float f)
{
    return (binop3_new(binop3_la_class, f));
}



/* --------------------------- | ---------------------------- */

static t_class *binop3_bo_class;

static void *binop3_bo_new(t_float f)
{
    return (binop3_new(binop3_bo_class, f));
}



/* --------------------------- || ---------------------------- */

static t_class *binop3_lo_class;

static void *binop3_lo_new(t_float f)
{
    return (binop3_new(binop3_lo_class, f));
}



/* --------------------------- << ---------------------------- */

static t_class *binop3_ls_class;

static void *binop3_ls_new(t_float f)
{
    return (binop3_new(binop3_ls_class, f));
}



/* --------------------------- >> ---------------------------- */

static t_class *binop3_rs_class;

static void *binop3_rs_new(t_float f)
{
    return (binop3_new(binop3_rs_class, f));
}

/* --------------------------- % ---------------------------- */

static t_class *binop3_pc_class;

static void *binop3_pc_new(t_float f)
{
    return (binop3_new(binop3_pc_class, f));
}



/* --------------------------- mod ---------------------------- */

static t_class *binop3_mod_class;

static void *binop3_mod_new(t_float f)
{
    return (binop3_new(binop3_mod_class, f));
}

static void binop3_mod_bang(t_binop *x)
{
    int n2 = x->x_f2, result;
    if (n2 < 0) n2 = -n2;
    else if (!n2) n2 = 1;
    result = ((int)(x->x_f1)) % n2;
    if (result < 0) result += n2;
    outlet_float(x->x_obj.te_outlet, (t_float)result);
}

static void binop3_mod_float(t_binop *x, t_float f)
{
    x->x_f1 = f;
    binop3_mod_bang(x);
}

/* --------------------------- div ---------------------------- */

static t_class *binop3_div_class;

static void *binop3_div_new(t_float f)
{
    return (binop3_new(binop3_div_class, f));
}

static void binop3_div_bang(t_binop *x)
{
    int n1 = x->x_f1, n2 = x->x_f2, result;
    if (n2 < 0) n2 = -n2;
    else if (!n2) n2 = 1;
    if (n1 < 0) n1 -= (n2-1);
    result = n1 / n2;
    outlet_float(x->x_obj.te_outlet, (t_float)result);
}

static void binop3_div_float(t_binop *x, t_float f)
{
    x->x_f1 = f;
    binop3_div_bang(x);
}

void binop3_setup(void)
{
    t_symbol *binop1_sym = sym_pow;
    t_symbol *binop23_sym = sym___ampersand____ampersand__;

    binop3_ba_class = class_new (sym___ampersand__, (t_newmethod)binop3_ba_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_ba_class, binop2_ba_bang);
    class_addFloat(binop3_ba_class, (t_method)binop2_ba_float);
    class_setHelpName(binop3_ba_class, binop23_sym);

    binop3_la_class = class_new(sym___ampersand____ampersand__, (t_newmethod)binop3_la_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_la_class, binop2_la_bang);
    class_addFloat(binop3_la_class, (t_method)binop2_la_float);
    class_setHelpName(binop3_la_class, binop23_sym);

    binop3_bo_class = class_new (sym___bar__, (t_newmethod)binop3_bo_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_bo_class, binop2_bo_bang);
    class_addFloat(binop3_bo_class, (t_method)binop2_bo_float);
    class_setHelpName(binop3_bo_class, binop23_sym);

    binop3_lo_class = class_new (sym___bar____bar__, (t_newmethod)binop3_lo_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_lo_class, binop2_lo_bang);
    class_addFloat(binop3_lo_class, (t_method)binop2_lo_float);
    class_setHelpName(binop3_lo_class, binop23_sym);

    binop3_ls_class = class_new(sym___less____less__, (t_newmethod)binop3_ls_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_ls_class, binop2_ls_bang);
    class_addFloat(binop3_ls_class, (t_method)binop2_ls_float);
    class_setHelpName(binop3_ls_class, binop23_sym);

    binop3_rs_class = class_new(sym___greater____greater__, (t_newmethod)binop3_rs_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_rs_class, binop2_rs_bang);
    class_addFloat(binop3_rs_class, (t_method)binop2_rs_float);
    class_setHelpName(binop3_rs_class, binop23_sym);

    binop3_pc_class = class_new(sym___percent__, (t_newmethod)binop3_pc_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_pc_class, binop2_pc_bang);
    class_addFloat(binop3_pc_class, (t_method)binop2_pc_float);
    class_setHelpName(binop3_pc_class, binop23_sym);

    binop3_mod_class = class_new(sym_mod, (t_newmethod)binop3_mod_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_mod_class, binop3_mod_bang);
    class_addFloat(binop3_mod_class, (t_method)binop3_mod_float);
    class_setHelpName(binop3_mod_class, binop23_sym);

    binop3_div_class = class_new (sym_div, (t_newmethod)binop3_div_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop3_div_class, binop3_div_bang);
    class_addFloat(binop3_div_class, (t_method)binop3_div_float);
    class_setHelpName(binop3_div_class, binop23_sym);
}


