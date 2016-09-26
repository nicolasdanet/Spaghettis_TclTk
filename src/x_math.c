
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sin_class;          /* Shared. */
static t_class *cos_class;          /* Shared. */
static t_class *tan_class;          /* Shared. */
static t_class *log_class;          /* Shared. */
static t_class *exp_class;          /* Shared. */
static t_class *abs_class;          /* Shared. */
static t_class *sqrt_class;         /* Shared. */
static t_class *wrap_class;         /* Shared. */
static t_class *atan_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _math {
    t_object    x_obj;              /* Must be the first. */
    t_outlet    *x_outlet;
    } t_math;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MATH_MAXIMUM_LOGARITHM      87.3365

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *sin_new (void)
{
    t_math *x = (t_math *)pd_new (sin_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void sin_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, sinf (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *cos_new (void)
{
    t_math *x = (t_math *)pd_new (cos_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void cos_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, cosf (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tan_new (void)
{
    t_math *x = (t_math *)pd_new (tan_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void tan_float (t_math *x, t_float f)
{
    t_float c = cosf (f);
    t_float t = (c == 0.0 ? 0.0 : sinf (f) / c);
    
    outlet_float (x->x_outlet, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *log_new (void)
{
    t_math *x = (t_math *)pd_new (log_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void log_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, (t_float)(f > 0.0 ? logf (f) : -1000.0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *exp_new (void)
{
    t_math *x = (t_math *)pd_new (exp_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void exp_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, expf (PD_MIN (f, MATH_MAXIMUM_LOGARITHM)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *abs_new (void)
{
    t_math *x = (t_math *)pd_new (abs_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void abs_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, fabsf (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *sqrt_new (void)
{
    t_math *x = (t_math *)pd_new (sqrt_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void sqrt_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, (t_float)(f > 0.0 ? sqrtf (f) : 0.0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *wrap_new (void)
{
    t_math *x = (t_math *)pd_new (wrap_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void wrap_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, (t_float)(f - floor (f)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *atan_new (void)
{
    t_math *x = (t_math *)pd_new (atan_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void atan_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, atanf (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void math_setup (void)
{
    sin_class = class_new (sym_sin,
                    sin_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
    
    cos_class = class_new (sym_cos,
                    cos_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    tan_class = class_new (sym_tan,
                    tan_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    log_class = class_new (sym_log,
                    log_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    exp_class = class_new (sym_exp,
                    exp_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    abs_class = class_new (sym_abs,
                    abs_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    sqrt_class = class_new (sym_sqrt,
                    sqrt_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    wrap_class = class_new (sym_wrap,
                    wrap_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    atan_class = class_new (sym_atan,
                    atan_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
        
    class_addFloat (sin_class,      (t_method)sin_float);  
    class_addFloat (cos_class,      (t_method)cos_float);
    class_addFloat (tan_class,      (t_method)tan_float);
    class_addFloat (log_class,      (t_method)log_float);
    class_addFloat (exp_class,      (t_method)exp_float);
    class_addFloat (abs_class,      (t_method)abs_float); 
    class_addFloat (wrap_class,     (t_method)wrap_float);    
    class_addFloat (sqrt_class,     (t_method)sqrt_float);
    class_addFloat (atan_class,     (t_method)atan_float);
            
    class_setHelpName (sin_class,   sym_sqrt);
    class_setHelpName (cos_class,   sym_sqrt);
    class_setHelpName (tan_class,   sym_sqrt);
    class_setHelpName (log_class,   sym_sqrt);
    class_setHelpName (exp_class,   sym_sqrt);
    class_setHelpName (abs_class,   sym_sqrt);
    class_setHelpName (sqrt_class,  sym_sqrt);
    class_setHelpName (wrap_class,  sym_sqrt);
    class_setHelpName (atan_class,  sym_sqrt);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
