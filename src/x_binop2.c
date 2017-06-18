
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *binopEquals_class;              /* Shared. */
static t_class *binopNotEquals_class;           /* Shared. */
static t_class *binopGreater_class;             /* Shared. */
static t_class *binopLess_class;                /* Shared. */
static t_class *binopGreaterEquals_class;       /* Shared. */
static t_class *binopLessEquals_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *binopEquals_new (t_float f)
{
    return binop_new (binopEquals_class, f);
}

static void binopEquals_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 == x->bo_f2);
}

static void binopEquals_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopEquals_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *binopNotEquals_new (t_float f)
{
    return binop_new (binopNotEquals_class, f);
}

static void binopNotEquals_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 != x->bo_f2);
}

static void binopNotEquals_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopNotEquals_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *binopGreater_new (t_float f)
{
    return binop_new (binopGreater_class, f);
}

static void binopGreater_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 > x->bo_f2);
}

static void binopGreater_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopGreater_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *binopLess_new (t_float f)
{
    return binop_new (binopLess_class, f);
}

static void binopLess_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 < x->bo_f2);
}

static void binopLess_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopLess_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *binopGreaterEquals_new (t_float f)
{
    return binop_new (binopGreaterEquals_class, f);
}

static void binopGreaterEquals_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 >= x->bo_f2);
}

static void binopGreaterEquals_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopGreaterEquals_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *binopLessEquals_new (t_float f)
{
    return binop_new (binopLessEquals_class, f);
}

static void binopLessEquals_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, x->bo_f1 <= x->bo_f2);
}

static void binopLessEquals_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopLessEquals_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void binop2_setup (void)
{
    binopEquals_class = class_new (sym___equals____equals__,
                                (t_newmethod)binopEquals_new,
                                NULL,
                                sizeof (t_binop),
                                CLASS_DEFAULT,
                                A_DEFFLOAT,
                                A_NULL);
                            
    binopNotEquals_class = class_new (sym___exclamation____equals__,
                                (t_newmethod)binopNotEquals_new,
                                NULL,
                                sizeof (t_binop),
                                CLASS_DEFAULT,
                                A_DEFFLOAT,
                                A_NULL);
                            
    binopGreater_class = class_new (sym___greater__,
                                (t_newmethod)binopGreater_new,
                                NULL,
                                sizeof (t_binop),
                                CLASS_DEFAULT,
                                A_DEFFLOAT,
                                A_NULL);
                            
    binopLess_class = class_new (sym___less__,
                                (t_newmethod)binopLess_new,
                                NULL,
                                sizeof (t_binop),
                                CLASS_DEFAULT,
                                A_DEFFLOAT,
                                A_NULL);
                            
    binopGreaterEquals_class = class_new (sym___greater____equals__,
                                (t_newmethod)binopGreaterEquals_new,
                                NULL,
                                sizeof (t_binop),
                                CLASS_DEFAULT,
                                A_DEFFLOAT,
                                A_NULL);
                                
    binopLessEquals_class = class_new (sym___less____equals__,
                                (t_newmethod)binopLessEquals_new,
                                NULL,
                                sizeof (t_binop),
                                CLASS_DEFAULT,
                                A_DEFFLOAT,
                                A_NULL);
                                
    class_addBang (binopEquals_class,               (t_method)binopEquals_bang);
    class_addBang (binopNotEquals_class,            (t_method)binopNotEquals_bang);
    class_addBang (binopGreater_class,              (t_method)binopGreater_bang);
    class_addBang (binopLess_class,                 (t_method)binopLess_bang);
    class_addBang (binopGreaterEquals_class,        (t_method)binopGreaterEquals_bang);
    class_addBang (binopLessEquals_class,           (t_method)binopLessEquals_bang);
        
    class_addFloat (binopEquals_class,              (t_method)binopEquals_float);
    class_addFloat (binopNotEquals_class,           (t_method)binopNotEquals_float);
    class_addFloat (binopGreater_class,             (t_method)binopGreater_float);
    class_addFloat (binopLess_class,                (t_method)binopLess_float);
    class_addFloat (binopGreaterEquals_class,       (t_method)binopGreaterEquals_float);
    class_addFloat (binopLessEquals_class,          (t_method)binopLessEquals_float);
    
    class_setHelpName (binopEquals_class,           sym___ampersand____ampersand__);
    class_setHelpName (binopNotEquals_class,        sym___ampersand____ampersand__);
    class_setHelpName (binopGreater_class,          sym___ampersand____ampersand__);
    class_setHelpName (binopLess_class,             sym___ampersand____ampersand__);
    class_setHelpName (binopGreaterEquals_class,    sym___ampersand____ampersand__);
    class_setHelpName (binopLessEquals_class,       sym___ampersand____ampersand__);
}

void binop2_destroy (void)
{
    class_free (binopEquals_class);
    class_free (binopNotEquals_class);
    class_free (binopGreater_class);
    class_free (binopLess_class);
    class_free (binopGreaterEquals_class);
    class_free (binopLessEquals_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
