/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* arithmetic: binops ala C language.  The 4 functions and relationals are
done on floats; the logical and bitwise binops convert their
inputs to int and their outputs back to float. */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include <math.h>


typedef struct _binop
{
    t_object x_obj;
    t_float x_f1;
    t_float x_f2;
} t_binop;

/* ------------------ binop1:  +, -, *, / ----------------------------- */

static void *binop1_new(t_class *floatclass, t_float f)
{
    t_binop *x = (t_binop *)pd_new(floatclass);
    outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    x->x_f1 = 0;
    x->x_f2 = f;
    return (x);
}

/* --------------------- addition ------------------------------- */

static t_class *binop1_plus_class;

static void *binop1_plus_new(t_float f)
{
    return (binop1_new(binop1_plus_class, f));
}

static void binop1_plus_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 + x->x_f2);
}

static void binop1_plus_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) + x->x_f2);
}

/* --------------------- subtraction ------------------------------- */

static t_class *binop1_minus_class;

static void *binop1_minus_new(t_float f)
{
    return (binop1_new(binop1_minus_class, f));
}

static void binop1_minus_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 - x->x_f2);
}

static void binop1_minus_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) - x->x_f2);
}

/* --------------------- multiplication ------------------------------- */

static t_class *binop1_times_class;

static void *binop1_times_new(t_float f)
{
    return (binop1_new(binop1_times_class, f));
}

static void binop1_times_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 * x->x_f2);
}

static void binop1_times_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) * x->x_f2);
}

/* --------------------- division ------------------------------- */

static t_class *binop1_div_class;

static void *binop1_div_new(t_float f)
{
    return (binop1_new(binop1_div_class, f));
}

static void binop1_div_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet,
        (x->x_f2 != 0 ? x->x_f1 / x->x_f2 : 0));
}

static void binop1_div_float(t_binop *x, t_float f)
{
    x->x_f1 = f;
    outlet_float(x->x_obj.te_outlet,
        (x->x_f2 != 0 ? x->x_f1 / x->x_f2 : 0));
}

/* ------------------------ pow -------------------------------- */

static t_class *binop1_pow_class;

static void *binop1_pow_new(t_float f)
{
    return (binop1_new(binop1_pow_class, f));
}

static void binop1_pow_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet,
        (x->x_f1 > 0 ? powf(x->x_f1, x->x_f2) : 0));
}

static void binop1_pow_float(t_binop *x, t_float f)
{
    x->x_f1 = f;
    outlet_float(x->x_obj.te_outlet,
        (x->x_f1 > 0 ? powf(x->x_f1, x->x_f2) : 0));
}

/* ------------------------ max -------------------------------- */

static t_class *binop1_max_class;

static void *binop1_max_new(t_float f)
{
    return (binop1_new(binop1_max_class, f));
}

static void binop1_max_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet,
        (x->x_f1 > x->x_f2 ? x->x_f1 : x->x_f2));
}

static void binop1_max_float(t_binop *x, t_float f)
{
    x->x_f1 = f;
    outlet_float(x->x_obj.te_outlet,
        (x->x_f1 > x->x_f2 ? x->x_f1 : x->x_f2));
}

/* ------------------------ min -------------------------------- */

static t_class *binop1_min_class;

static void *binop1_min_new(t_float f)
{
    return (binop1_new(binop1_min_class, f));
}

static void binop1_min_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet,
        (x->x_f1 < x->x_f2 ? x->x_f1 : x->x_f2));
}

static void binop1_min_float(t_binop *x, t_float f)
{
    x->x_f1 = f;
    outlet_float(x->x_obj.te_outlet,
        (x->x_f1 < x->x_f2 ? x->x_f1 : x->x_f2));
}

/* ------------------ binop2: ==, !=, >, <, >=, <=. -------------------- */

static void *binop2_new(t_class *floatclass, t_float f)
{
    t_binop *x = (t_binop *)pd_new(floatclass);
    outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    x->x_f1 = 0;
    x->x_f2 = f;
    return (x);
}

/* --------------------- == ------------------------------- */

static t_class *binop2_ee_class;

static void *binop2_ee_new(t_float f)
{
    return (binop2_new(binop2_ee_class, f));
}

static void binop2_ee_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 == x->x_f2);
}

static void binop2_ee_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) == x->x_f2);
}

/* --------------------- != ------------------------------- */

static t_class *binop2_ne_class;

static void *binop2_ne_new(t_float f)
{
    return (binop2_new(binop2_ne_class, f));
}

static void binop2_ne_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 != x->x_f2);
}

static void binop2_ne_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) != x->x_f2);
}

/* --------------------- > ------------------------------- */

static t_class *binop2_gt_class;

static void *binop2_gt_new(t_float f)
{
    return (binop2_new(binop2_gt_class, f));
}

static void binop2_gt_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 > x->x_f2);
}

static void binop2_gt_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) > x->x_f2);
}

/* --------------------- < ------------------------------- */

static t_class *binop2_lt_class;

static void *binop2_lt_new(t_float f)
{
    return (binop2_new(binop2_lt_class, f));
}

static void binop2_lt_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 < x->x_f2);
}

static void binop2_lt_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) < x->x_f2);
}

/* --------------------- >= ------------------------------- */

static t_class *binop2_ge_class;

static void *binop2_ge_new(t_float f)
{
    return (binop2_new(binop2_ge_class, f));
}

static void binop2_ge_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 >= x->x_f2);
}

static void binop2_ge_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) >= x->x_f2);
}

/* --------------------- <= ------------------------------- */

static t_class *binop2_le_class;

static void *binop2_le_new(t_float f)
{
    return (binop2_new(binop2_le_class, f));
}

static void binop2_le_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f1 <= x->x_f2);
}

static void binop2_le_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (x->x_f1 = f) <= x->x_f2);
}

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

static void binop2_ba_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) & (int)(x->x_f2));
}

static void binop2_ba_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) & (int)(x->x_f2));
}

/* --------------------------- && ---------------------------- */

static t_class *binop3_la_class;

static void *binop3_la_new(t_float f)
{
    return (binop3_new(binop3_la_class, f));
}

static void binop2_la_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) && (int)(x->x_f2));
}

static void binop2_la_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) && (int)(x->x_f2));
}

/* --------------------------- | ---------------------------- */

static t_class *binop3_bo_class;

static void *binop3_bo_new(t_float f)
{
    return (binop3_new(binop3_bo_class, f));
}

static void binop2_bo_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) | (int)(x->x_f2));
}

static void binop2_bo_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) | (int)(x->x_f2));
}

/* --------------------------- || ---------------------------- */

static t_class *binop3_lo_class;

static void *binop3_lo_new(t_float f)
{
    return (binop3_new(binop3_lo_class, f));
}

static void binop2_lo_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) || (int)(x->x_f2));
}

static void binop2_lo_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) || (int)(x->x_f2));
}

/* --------------------------- << ---------------------------- */

static t_class *binop3_ls_class;

static void *binop3_ls_new(t_float f)
{
    return (binop3_new(binop3_ls_class, f));
}

static void binop2_ls_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) << (int)(x->x_f2));
}

static void binop2_ls_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) << (int)(x->x_f2));
}

/* --------------------------- >> ---------------------------- */

static t_class *binop3_rs_class;

static void *binop3_rs_new(t_float f)
{
    return (binop3_new(binop3_rs_class, f));
}

static void binop2_rs_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) >> (int)(x->x_f2));
}

static void binop2_rs_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) >> (int)(x->x_f2));
}

/* --------------------------- % ---------------------------- */

static t_class *binop3_pc_class;

static void *binop3_pc_new(t_float f)
{
    return (binop3_new(binop3_pc_class, f));
}

static void binop2_pc_bang(t_binop *x)
{
    int n2 = x->x_f2;
        /* apparently "%" raises an exception for INT_MIN and -1 */
    if (n2 == -1)
        outlet_float(x->x_obj.te_outlet, 0);
    else outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) % (n2 ? n2 : 1));
}

static void binop2_pc_float(t_binop *x, t_float f)
{
    int n2 = x->x_f2;
    if (n2 == -1)
        outlet_float(x->x_obj.te_outlet, 0);
    else outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) % (n2 ? n2 : 1));
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

void binop_setup(void)
{
    t_symbol *binop1_sym = sym_pow;
    t_symbol *binop23_sym = sym___ampersand____ampersand__;

    binop1_plus_class = class_new (sym___plus__, (t_newmethod)binop1_plus_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop1_plus_class, binop1_plus_bang);
    class_addFloat(binop1_plus_class, (t_method)binop1_plus_float);
    class_setHelpName(binop1_plus_class, binop1_sym);
    
    binop1_minus_class = class_new(sym___minus__,
        (t_newmethod)binop1_minus_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop1_minus_class, binop1_minus_bang);
    class_addFloat(binop1_minus_class, (t_method)binop1_minus_float);
    class_setHelpName(binop1_minus_class, binop1_sym);

    binop1_times_class = class_new(sym___asterisk__,
        (t_newmethod)binop1_times_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop1_times_class, binop1_times_bang);
    class_addFloat(binop1_times_class, (t_method)binop1_times_float);
    class_setHelpName(binop1_times_class, binop1_sym);

    binop1_div_class = class_new (sym___slash__,
        (t_newmethod)binop1_div_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop1_div_class, binop1_div_bang);
    class_addFloat(binop1_div_class, (t_method)binop1_div_float);
    class_setHelpName(binop1_div_class, binop1_sym);

    binop1_pow_class = class_new(sym_pow,
        (t_newmethod)binop1_pow_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop1_pow_class, binop1_pow_bang);
    class_addFloat(binop1_pow_class, (t_method)binop1_pow_float);
    class_setHelpName(binop1_pow_class, binop1_sym);

    binop1_max_class = class_new (sym_max,
        (t_newmethod)binop1_max_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop1_max_class, binop1_max_bang);
    class_addFloat(binop1_max_class, (t_method)binop1_max_float);
    class_setHelpName(binop1_max_class, binop1_sym);

    binop1_min_class = class_new (sym_min,
        (t_newmethod)binop1_min_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop1_min_class, binop1_min_bang);
    class_addFloat(binop1_min_class, (t_method)binop1_min_float);
    class_setHelpName(binop1_min_class, binop1_sym);

        /* ------------------ binop2 ----------------------- */

    binop2_ee_class = class_new(sym___equals____equals__, (t_newmethod)binop2_ee_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop2_ee_class, binop2_ee_bang);
    class_addFloat(binop2_ee_class, (t_method)binop2_ee_float);
    class_setHelpName(binop2_ee_class, binop23_sym);

    binop2_ne_class = class_new(sym___exclamation____equals__, (t_newmethod)binop2_ne_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop2_ne_class, binop2_ne_bang);
    class_addFloat(binop2_ne_class, (t_method)binop2_ne_float);
    class_setHelpName(binop2_ne_class, binop23_sym);

    binop2_gt_class = class_new(sym___greater__, (t_newmethod)binop2_gt_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop2_gt_class, binop2_gt_bang);
    class_addFloat(binop2_gt_class, (t_method)binop2_gt_float);
    class_setHelpName(binop2_gt_class, binop23_sym);

    binop2_lt_class = class_new(sym___less__, (t_newmethod)binop2_lt_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop2_lt_class, binop2_lt_bang);
    class_addFloat(binop2_lt_class, (t_method)binop2_lt_float);
    class_setHelpName(binop2_lt_class, binop23_sym);

    binop2_ge_class = class_new(sym___greater____equals__, (t_newmethod)binop2_ge_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop2_ge_class, binop2_ge_bang);
    class_addFloat(binop2_ge_class, (t_method)binop2_ge_float);
    class_setHelpName(binop2_ge_class, binop23_sym);

    binop2_le_class = class_new (sym___less____equals__, (t_newmethod)binop2_le_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binop2_le_class, binop2_le_bang);
    class_addFloat(binop2_le_class, (t_method)binop2_le_float);
    class_setHelpName(binop2_le_class, binop23_sym);

        /* ------------------ binop3 ----------------------- */

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


