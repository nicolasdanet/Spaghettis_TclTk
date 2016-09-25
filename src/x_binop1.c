
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *binopAdd_class;             /* Shared. */
static t_class *binopSubtract_class;        /* Shared. */
static t_class *binopMultiply_class;        /* Shared. */
static t_class *binopDivide_class;          /* Shared. */
static t_class *binopPower_class;           /* Shared. */
static t_class *binopMaximum_class;         /* Shared. */
static t_class *binopMinimum_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void *binop_new (t_class *floatclass, t_float f)
{
    t_binop *x = (t_binop *)pd_new(floatclass);
    outlet_new(&x->bo_obj, &s_float);
    inlet_newFloat(&x->bo_obj, &x->bo_f2);
    x->bo_f1 = 0;
    x->bo_f2 = f;
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopAdd_new(t_float f)
{
    return (binop_new(binopAdd_class, f));
}

static void binopAdd_bang(t_binop *x)
{
    outlet_float(x->bo_obj.te_outlet, x->bo_f1 + x->bo_f2);
}

static void binopAdd_float(t_binop *x, t_float f)
{
    outlet_float(x->bo_obj.te_outlet, (x->bo_f1 = f) + x->bo_f2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopSubtract_new(t_float f)
{
    return (binop_new(binopSubtract_class, f));
}

static void binopSubtract_bang(t_binop *x)
{
    outlet_float(x->bo_obj.te_outlet, x->bo_f1 - x->bo_f2);
}

static void binopSubtract_float(t_binop *x, t_float f)
{
    outlet_float(x->bo_obj.te_outlet, (x->bo_f1 = f) - x->bo_f2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopMultiply_new(t_float f)
{
    return (binop_new(binopMultiply_class, f));
}

static void binopMultiply_bang(t_binop *x)
{
    outlet_float(x->bo_obj.te_outlet, x->bo_f1 * x->bo_f2);
}

static void binopMultiply_float(t_binop *x, t_float f)
{
    outlet_float(x->bo_obj.te_outlet, (x->bo_f1 = f) * x->bo_f2);
}
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopDivide_new(t_float f)
{
    return (binop_new(binopDivide_class, f));
}

static void binopDivide_bang(t_binop *x)
{
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f2 != 0 ? x->bo_f1 / x->bo_f2 : 0));
}

static void binopDivide_float(t_binop *x, t_float f)
{
    x->bo_f1 = f;
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f2 != 0 ? x->bo_f1 / x->bo_f2 : 0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopPower_new(t_float f)
{
    return (binop_new(binopPower_class, f));
}

static void binopPower_bang(t_binop *x)
{
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f1 > 0 ? powf(x->bo_f1, x->bo_f2) : 0));
}

static void binopPower_float(t_binop *x, t_float f)
{
    x->bo_f1 = f;
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f1 > 0 ? powf(x->bo_f1, x->bo_f2) : 0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopMaximum_new(t_float f)
{
    return (binop_new(binopMaximum_class, f));
}

static void binopMaximum_bang(t_binop *x)
{
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f1 > x->bo_f2 ? x->bo_f1 : x->bo_f2));
}

static void binopMaximum_float(t_binop *x, t_float f)
{
    x->bo_f1 = f;
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f1 > x->bo_f2 ? x->bo_f1 : x->bo_f2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopMinimum_new(t_float f)
{
    return (binop_new(binopMinimum_class, f));
}

static void binopMinimum_bang(t_binop *x)
{
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f1 < x->bo_f2 ? x->bo_f1 : x->bo_f2));
}

static void binopMinimum_float(t_binop *x, t_float f)
{
    x->bo_f1 = f;
    outlet_float(x->bo_obj.te_outlet,
        (x->bo_f1 < x->bo_f2 ? x->bo_f1 : x->bo_f2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void binop1_setup (void)
{
    t_symbol *binop1_sym = sym_pow;
    t_symbol *binop23_sym = sym___ampersand____ampersand__;

    binopAdd_class = class_new (sym___plus__, (t_newmethod)binopAdd_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binopAdd_class, binopAdd_bang);
    class_addFloat(binopAdd_class, (t_method)binopAdd_float);
    class_setHelpName(binopAdd_class, binop1_sym);
    
    binopSubtract_class = class_new(sym___minus__,
        (t_newmethod)binopSubtract_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binopSubtract_class, binopSubtract_bang);
    class_addFloat(binopSubtract_class, (t_method)binopSubtract_float);
    class_setHelpName(binopSubtract_class, binop1_sym);

    binopMultiply_class = class_new(sym___asterisk__,
        (t_newmethod)binopMultiply_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binopMultiply_class, binopMultiply_bang);
    class_addFloat(binopMultiply_class, (t_method)binopMultiply_float);
    class_setHelpName(binopMultiply_class, binop1_sym);

    binopDivide_class = class_new (sym___slash__,
        (t_newmethod)binopDivide_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binopDivide_class, binopDivide_bang);
    class_addFloat(binopDivide_class, (t_method)binopDivide_float);
    class_setHelpName(binopDivide_class, binop1_sym);

    binopPower_class = class_new(sym_pow,
        (t_newmethod)binopPower_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binopPower_class, binopPower_bang);
    class_addFloat(binopPower_class, (t_method)binopPower_float);
    class_setHelpName(binopPower_class, binop1_sym);

    binopMaximum_class = class_new (sym_max,
        (t_newmethod)binopMaximum_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binopMaximum_class, binopMaximum_bang);
    class_addFloat(binopMaximum_class, (t_method)binopMaximum_float);
    class_setHelpName(binopMaximum_class, binop1_sym);

    binopMinimum_class = class_new (sym_min,
        (t_newmethod)binopMinimum_new, 0,
        sizeof(t_binop), 0, A_DEFFLOAT, 0);
    class_addBang(binopMinimum_class, binopMinimum_bang);
    class_addFloat(binopMinimum_class, (t_method)binopMinimum_float);
    class_setHelpName(binopMinimum_class, binop1_sym);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
