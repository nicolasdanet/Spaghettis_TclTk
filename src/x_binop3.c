
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *binopBitwiseAnd_class;          /* Shared. */
static t_class *binopLogicalAnd_class;          /* Shared. */
static t_class *binopBitwiseOr_class;           /* Shared. */
static t_class *binopLogicalOr_class;           /* Shared. */
static t_class *binopShiftLeft_class;           /* Shared. */
static t_class *binopShiftRight_class;          /* Shared. */
static t_class *binopModulo_class;              /* Shared. */
static t_class *binopIntegerModulo_class;       /* Shared. */
static t_class *binopIntegerDivide_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *binopBitwiseAnd_new (t_float f)
{
    return binop_new (binopBitwiseAnd_class, f);
}

void binopBitwiseAnd_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (t_float)((int)(x->bo_f1) & (int)(x->bo_f2)));
}

void binopBitwiseAnd_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopBitwiseAnd_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopLogicalAnd_new (t_float f)
{
    return binop_new (binopLogicalAnd_class, f);
}

void binopLogicalAnd_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (t_float)((int)(x->bo_f1) && (int)(x->bo_f2)));
}

void binopLogicalAnd_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopLogicalAnd_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopBitwiseOr_new (t_float f)
{
    return binop_new (binopBitwiseOr_class, f);
}

void binopBitwiseOr_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (t_float)((int)(x->bo_f1) | (int)(x->bo_f2)));
}

void binopBitwiseOr_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopBitwiseOr_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopLogicalOr_new (t_float f)
{
    return binop_new (binopLogicalOr_class, f);
}

void binopLogicalOr_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (t_float)((int)(x->bo_f1) || (int)(x->bo_f2)));
}

void binopLogicalOr_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopLogicalOr_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopShiftLeft_new (t_float f)
{
    return binop_new (binopShiftLeft_class, f);
}

void binopShiftLeft_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (t_float)((int)(x->bo_f1) << (int)(x->bo_f2)));
}

void binopShiftLeft_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopShiftLeft_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopShiftRight_new (t_float f)
{
    return binop_new (binopShiftRight_class, f);
}

void binopShiftRight_bang (t_binop *x)
{
    outlet_float (x->bo_outlet, (t_float)((int)(x->bo_f1) >> (int)(x->bo_f2)));
}

void binopShiftRight_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopShiftRight_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopModulo_new (t_float f)
{
    return binop_new (binopModulo_class, f);
}

void binopModulo_bang (t_binop *x)
{
    int n1 = (int)x->bo_f1;
    int n2 = (int)x->bo_f2 == 0 ? 1 : (int)x->bo_f2;
    
    outlet_float (x->bo_outlet, (t_float)(n1 % n2));
}

void binopModulo_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopModulo_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopIntegerModulo_new (t_float f)
{
    return binop_new (binopIntegerModulo_class, f);
}

static void binopIntegerModulo_bang (t_binop *x)
{
    int n1 = (int)x->bo_f1;
    int n2 = (int)x->bo_f2 == 0 ? 1 : (int)x->bo_f2;
    int k;
        
    n2 = PD_ABS (n2);
    
    k = n1 % n2;
    
    if (k < 0) { k += n2; }
    
    outlet_float (x->bo_outlet, (t_float)k);
}

static void binopIntegerModulo_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopIntegerModulo_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *binopIntegerDivide_new (t_float f)
{
    return binop_new (binopIntegerDivide_class, f);
}

static void binopIntegerDivide_bang (t_binop *x)
{
    int n1 = (int)x->bo_f1;
    int n2 = (int)x->bo_f2 == 0 ? 1 : (int)x->bo_f2;
    int k;
    
    n2 = PD_ABS (n2);
    
    if (n1 < 0) { n1 -= (n2 - 1); }
    
    k = n1 / n2;
    
    outlet_float (x->bo_outlet, (t_float)k);
}

static void binopIntegerDivide_float (t_binop *x, t_float f)
{
    x->bo_f1 = f; binopIntegerDivide_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void binop3_setup (void)
{
    binopBitwiseAnd_class = class_new (sym___ampersand__, 
                                    (t_newmethod)binopBitwiseAnd_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);
                                    
    binopLogicalAnd_class = class_new (sym___ampersand____ampersand__,
                                    (t_newmethod)binopLogicalAnd_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);
                                    
    binopBitwiseOr_class = class_new (sym___bar__,
                                    (t_newmethod)binopBitwiseOr_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);
                                    
    binopLogicalOr_class = class_new (sym___bar____bar__,
                                    (t_newmethod)binopLogicalOr_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);
                                    
    binopShiftLeft_class = class_new (sym___less____less__,
                                    (t_newmethod)binopShiftLeft_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);

    binopShiftRight_class = class_new (sym___greater____greater__,
                                    (t_newmethod)binopShiftRight_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);
                                    
    binopModulo_class = class_new (sym___percent__,
                                    (t_newmethod)binopModulo_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);

    binopIntegerModulo_class = class_new (sym_mod,
                                    (t_newmethod)binopIntegerModulo_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);
                                    
    binopIntegerDivide_class = class_new (sym_div, 
                                    (t_newmethod)binopIntegerDivide_new,
                                    NULL,
                                    sizeof (t_binop),
                                    CLASS_DEFAULT,
                                    A_DEFFLOAT,
                                    A_NULL);
                                    
    class_addBang (binopBitwiseAnd_class,           (t_method)binopBitwiseAnd_bang);
    class_addBang (binopLogicalAnd_class,           (t_method)binopLogicalAnd_bang);
    class_addBang (binopBitwiseOr_class,            (t_method)binopBitwiseOr_bang);
    class_addBang (binopLogicalOr_class,            (t_method)binopLogicalOr_bang);
    class_addBang (binopShiftLeft_class,            (t_method)binopShiftLeft_bang);
    class_addBang (binopShiftRight_class,           (t_method)binopShiftRight_bang);  
    class_addBang (binopModulo_class,               (t_method)binopModulo_bang);
    class_addBang (binopIntegerModulo_class,        (t_method)binopIntegerModulo_bang);
    class_addBang (binopIntegerDivide_class,        (t_method)binopIntegerDivide_bang);
        
    class_addFloat (binopBitwiseAnd_class,          (t_method)binopBitwiseAnd_float);
    class_addFloat (binopLogicalAnd_class,          (t_method)binopLogicalAnd_float);
    class_addFloat (binopBitwiseOr_class,           (t_method)binopBitwiseOr_float);
    class_addFloat (binopLogicalOr_class,           (t_method)binopLogicalOr_float);
    class_addFloat (binopShiftLeft_class,           (t_method)binopShiftLeft_float);
    class_addFloat (binopShiftRight_class,          (t_method)binopShiftRight_float);
    class_addFloat (binopModulo_class,              (t_method)binopModulo_float);
    class_addFloat (binopIntegerModulo_class,       (t_method)binopIntegerModulo_float);
    class_addFloat (binopIntegerDivide_class,       (t_method)binopIntegerDivide_float);
        
    class_setHelpName (binopBitwiseAnd_class,       sym___ampersand____ampersand__);
    class_setHelpName (binopLogicalAnd_class,       sym___ampersand____ampersand__);
    class_setHelpName (binopBitwiseOr_class,        sym___ampersand____ampersand__);
    class_setHelpName (binopLogicalOr_class,        sym___ampersand____ampersand__);
    class_setHelpName (binopShiftLeft_class,        sym___ampersand____ampersand__);
    class_setHelpName (binopShiftRight_class,       sym___ampersand____ampersand__);
    class_setHelpName (binopModulo_class,           sym___ampersand____ampersand__);
    class_setHelpName (binopIntegerModulo_class,    sym___ampersand____ampersand__);
    class_setHelpName (binopIntegerDivide_class,    sym___ampersand____ampersand__);
}

void binop3_destroy (void)
{
    CLASS_FREE (binopBitwiseAnd_class);
    CLASS_FREE (binopLogicalAnd_class);
    CLASS_FREE (binopBitwiseOr_class);
    CLASS_FREE (binopLogicalOr_class);
    CLASS_FREE (binopShiftLeft_class);
    CLASS_FREE (binopShiftRight_class);
    CLASS_FREE (binopModulo_class);
    CLASS_FREE (binopIntegerModulo_class);
    CLASS_FREE (binopIntegerDivide_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
