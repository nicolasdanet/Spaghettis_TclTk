
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
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

void *binop_new (t_class *class, t_float f)
{
    t_binop *x = (t_binop *)pd_new (class);
    
    x->bo_f1 = (t_float)0.0;
    x->bo_f2 = f;
    x->bo_outlet = outlet_new (cast_object (x), &s_float);
    
    inlet_newFloat (cast_object (x), &x->bo_f2);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopAdd_new (t_float f)
{
    return binop_new (binopAdd_class, f);
}

static void binopAdd_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 + x->bo_f2);
}

static void binopAdd_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopAdd_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopSubtract_new (t_float f)
{
    return binop_new (binopSubtract_class, f);
}

static void binopSubtract_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 - x->bo_f2);
}

static void binopSubtract_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopSubtract_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopMultiply_new (t_float f)
{
    return binop_new (binopMultiply_class, f);
}

static void binopMultiply_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 * x->bo_f2);
}

static void binopMultiply_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopMultiply_bang (x);
}
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopDivide_new (t_float f)
{
    return binop_new (binopDivide_class, f);
}

static void binopDivide_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (x->bo_f2 != 0.0 ? x->bo_f1 / x->bo_f2 : (t_float)0.0));
}

static void binopDivide_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopDivide_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopPower_new (t_float f)
{
    return binop_new (binopPower_class, f);
}

static void binopPower_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (x->bo_f1 > 0.0 ? powf (x->bo_f1, x->bo_f2) : (t_float)0.0));
}

static void binopPower_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopPower_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopMaximum_new (t_float f)
{
    return binop_new (binopMaximum_class, f);
}

static void binopMaximum_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, PD_MAX (x->bo_f1, x->bo_f2));
}

static void binopMaximum_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopMaximum_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopMinimum_new (t_float f)
{
    return binop_new (binopMinimum_class, f);
}

static void binopMinimum_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, PD_MIN (x->bo_f1, x->bo_f2));
}

static void binopMinimum_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopMinimum_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void binop1_setup (void)
{
    binopAdd_class = class_new (sym___plus__,
                            (t_newmethod)binopAdd_new,
                            NULL,
                            sizeof (t_binop),
                            CLASS_DEFAULT,
                            A_DEFFLOAT,
                            A_NULL);
                            
    binopSubtract_class = class_new (sym___minus__,
                            (t_newmethod)binopSubtract_new,
                            NULL,
                            sizeof (t_binop),
                            CLASS_DEFAULT,
                            A_DEFFLOAT,
                            A_NULL);
                            
    binopMultiply_class = class_new (sym___asterisk__,
                            (t_newmethod)binopMultiply_new,
                            NULL,
                            sizeof (t_binop),
                            CLASS_DEFAULT,
                            A_DEFFLOAT,
                            A_NULL);

    binopDivide_class = class_new (sym___slash__,
                            (t_newmethod)binopDivide_new,
                            NULL,
                            sizeof (t_binop),
                            CLASS_DEFAULT,
                            A_DEFFLOAT,
                            A_NULL);
                            
    binopPower_class = class_new (sym_pow,
                            (t_newmethod)binopPower_new,
                            NULL,
                            sizeof (t_binop),
                            CLASS_DEFAULT,
                            A_DEFFLOAT,
                            A_NULL);
                            
    binopMaximum_class = class_new (sym_max,
                            (t_newmethod)binopMaximum_new,
                            NULL,
                            sizeof (t_binop),
                            CLASS_DEFAULT,
                            A_DEFFLOAT,
                            A_NULL);
                            
    binopMinimum_class = class_new (sym_min,
                            (t_newmethod)binopMinimum_new,
                            NULL,
                            sizeof (t_binop),
                            CLASS_DEFAULT,
                            A_DEFFLOAT,
                            A_NULL);

    class_addBang (binopAdd_class,          (t_method)binopAdd_bang);
    class_addBang (binopSubtract_class,     (t_method)binopSubtract_bang);
    class_addBang (binopMultiply_class,     (t_method)binopMultiply_bang);
    class_addBang (binopDivide_class,       (t_method)binopDivide_bang);
    class_addBang (binopPower_class,        (t_method)binopPower_bang);
    class_addBang (binopMaximum_class,      (t_method)binopMaximum_bang);
    class_addBang (binopMinimum_class,      (t_method)binopMinimum_bang);
        
    class_addFloat (binopAdd_class,         (t_method)binopAdd_float);
    class_addFloat (binopSubtract_class,    (t_method)binopSubtract_float);
    class_addFloat (binopMultiply_class,    (t_method)binopMultiply_float);
    class_addFloat (binopDivide_class,      (t_method)binopDivide_float);
    class_addFloat (binopPower_class,       (t_method)binopPower_float);
    class_addFloat (binopMaximum_class,     (t_method)binopMaximum_float);
    class_addFloat (binopMinimum_class,     (t_method)binopMinimum_float);
        
    class_setHelpName (binopAdd_class,      sym_pow);
    class_setHelpName (binopSubtract_class, sym_pow);
    class_setHelpName (binopMultiply_class, sym_pow);
    class_setHelpName (binopDivide_class,   sym_pow);
    class_setHelpName (binopPower_class,    sym_pow);
    class_setHelpName (binopMaximum_class,  sym_pow);
    class_setHelpName (binopMinimum_class,  sym_pow);
}

void binop1_destroy (void)
{
    CLASS_FREE (binopAdd_class);
    CLASS_FREE (binopSubtract_class);
    CLASS_FREE (binopMultiply_class);
    CLASS_FREE (binopDivide_class);
    CLASS_FREE (binopPower_class);
    CLASS_FREE (binopMaximum_class);
    CLASS_FREE (binopMinimum_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
