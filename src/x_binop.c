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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

void binop2_ba_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) & (int)(x->x_f2));
}

void binop2_ba_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) & (int)(x->x_f2));
}

void binop2_la_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) && (int)(x->x_f2));
}

void binop2_la_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) && (int)(x->x_f2));
}

void binop2_bo_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) | (int)(x->x_f2));
}

void binop2_bo_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) | (int)(x->x_f2));
}

void binop2_lo_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) || (int)(x->x_f2));
}

void binop2_lo_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) || (int)(x->x_f2));
}

void binop2_ls_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) << (int)(x->x_f2));
}

void binop2_ls_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) << (int)(x->x_f2));
}

void binop2_rs_bang(t_binop *x)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) >> (int)(x->x_f2));
}

void binop2_rs_float(t_binop *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) >> (int)(x->x_f2));
}

void binop2_pc_bang(t_binop *x)
{
    int n2 = x->x_f2;
        /* apparently "%" raises an exception for INT_MIN and -1 */
    if (n2 == -1)
        outlet_float(x->x_obj.te_outlet, 0);
    else outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1)) % (n2 ? n2 : 1));
}

void binop2_pc_float(t_binop *x, t_float f)
{
    int n2 = x->x_f2;
    if (n2 == -1)
        outlet_float(x->x_obj.te_outlet, 0);
    else outlet_float(x->x_obj.te_outlet, ((int)(x->x_f1 = f)) % (n2 ? n2 : 1));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
}


