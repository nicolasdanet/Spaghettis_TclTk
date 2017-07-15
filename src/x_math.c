
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"

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
// MARK: -

#define MATH_MAXIMUM_LOGARITHM      87.3365

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
// MARK: -

static void *tan_new (void)
{
    t_math *x = (t_math *)pd_new (tan_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void tan_float (t_math *x, t_float f)
{
    t_float c = cosf (f);
    t_float t = (t_float)(c == 0.0 ? 0.0 : sinf (f) / c);
    
    outlet_float (x->x_outlet, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static void *exp_new (void)
{
    t_math *x = (t_math *)pd_new (exp_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void exp_float (t_math *x, t_float f)
{
    outlet_float (x->x_outlet, expf ((t_float)PD_MIN (f, MATH_MAXIMUM_LOGARITHM)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
// MARK: -

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
// MARK: -

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
// MARK: -

void math_setup (void)
{
    sin_class = class_new (sym_sin,
                    (t_newmethod)sin_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
    
    cos_class = class_new (sym_cos,
                    (t_newmethod)cos_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    tan_class = class_new (sym_tan,
                    (t_newmethod)tan_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    log_class = class_new (sym_log,
                    (t_newmethod)log_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    exp_class = class_new (sym_exp,
                    (t_newmethod)exp_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    abs_class = class_new (sym_abs,
                    (t_newmethod)abs_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    sqrt_class = class_new (sym_sqrt,
                    (t_newmethod)sqrt_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    wrap_class = class_new (sym_wrap,
                    (t_newmethod)wrap_new,
                    NULL,
                    sizeof (t_math),
                    CLASS_DEFAULT,
                    A_NULL);
                    
    atan_class = class_new (sym_atan,
                    (t_newmethod)atan_new,
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
            
    class_setHelpName (sin_class,   sym_math);
    class_setHelpName (cos_class,   sym_math);
    class_setHelpName (tan_class,   sym_math);
    class_setHelpName (log_class,   sym_math);
    class_setHelpName (exp_class,   sym_math);
    class_setHelpName (abs_class,   sym_math);
    class_setHelpName (sqrt_class,  sym_math);
    class_setHelpName (wrap_class,  sym_math);
    class_setHelpName (atan_class,  sym_math);
}

void math_destroy (void)
{
    class_free (sin_class);
    class_free (cos_class);
    class_free (tan_class);
    class_free (log_class);
    class_free (exp_class);
    class_free (abs_class);
    class_free (sqrt_class);
    class_free (wrap_class);
    class_free (atan_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
