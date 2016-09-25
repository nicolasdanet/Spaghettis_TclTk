
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

#define MAXLOG 87.3365

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *sin_new(void)
{
    t_object *x = (t_object *)pd_new(sin_class);
    outlet_new(x, &s_float);
    return (x);
}

static void sin_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, sinf(f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *cos_new(void)
{
    t_object *x = (t_object *)pd_new(cos_class);
    outlet_new(x, &s_float);
    return (x);
}

static void cos_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, cosf(f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tan_new(void)
{
    t_object *x = (t_object *)pd_new(tan_class);
    outlet_new(x, &s_float);
    return (x);
}

static void tan_float(t_object *x, t_float f)
{
    t_float c = cosf(f);
    t_float t = (c == 0 ? 0 : sinf(f)/c);
    outlet_float(x->te_outlet, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *log_new(void)
{
    t_object *x = (t_object *)pd_new(log_class);
    outlet_new(x, &s_float);
    return (x);
}

static void log_float(t_object *x, t_float f)
{
    t_float r = (f > 0 ? logf(f) : -1000);
    outlet_float(x->te_outlet, r);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *exp_new(void)
{
    t_object *x = (t_object *)pd_new(exp_class);
    outlet_new(x, &s_float);
    return (x);
}

static void exp_float(t_object *x, t_float f)
{
    t_float g;
#ifdef _WIN32
    char buf[10];
#endif
    if (f > MAXLOG) f = MAXLOG;
    g = expf(f);
    outlet_float(x->te_outlet, g);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *abs_new(void)
{
    t_object *x = (t_object *)pd_new(abs_class);
    outlet_new(x, &s_float);
    return (x);
}

static void abs_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, fabsf(f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *sqrt_new(void)
{
    t_object *x = (t_object *)pd_new(sqrt_class);
    outlet_new(x, &s_float);
    return (x);
}

static void sqrt_float(t_object *x, t_float f)
{
    t_float r = (f > 0 ? sqrtf(f) : 0);
    outlet_float(x->te_outlet, r);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *wrap_new(void)
{
    t_object *x = (t_object *)pd_new(wrap_class);
    outlet_new(x, &s_float);
    return (x);
}

static void wrap_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, f - floor(f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *atan_new(void)
{
    t_object *x = (t_object *)pd_new(atan_class);
    outlet_new(x, &s_float);
    return (x);
}

static void atan_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, atanf(f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void math_setup (void)
{
    t_symbol *math_sym = sym_sqrt;
    
    sin_class = class_new (sym_sin, sin_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(sin_class, (t_method)sin_float);
    class_setHelpName(sin_class, math_sym);
    
    cos_class = class_new (sym_cos, cos_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(cos_class, (t_method)cos_float);
    class_setHelpName(cos_class, math_sym);
    
    tan_class = class_new (sym_tan, tan_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(tan_class, (t_method)tan_float);
    class_setHelpName(tan_class, math_sym);

    log_class = class_new (sym_log, log_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(log_class, (t_method)log_float);    
    class_setHelpName(log_class, math_sym);

    exp_class = class_new(sym_exp, exp_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(exp_class, (t_method)exp_float);
    class_setHelpName(exp_class, math_sym);

    abs_class = class_new(sym_abs, abs_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(abs_class, (t_method)abs_float);    
    class_setHelpName(abs_class, math_sym);
    
    sqrt_class = class_new(sym_sqrt, sqrt_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(sqrt_class, (t_method)sqrt_float);
    class_setHelpName(sqrt_class, math_sym);

    wrap_class = class_new (sym_wrap, wrap_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(wrap_class, (t_method)wrap_float);    
    class_setHelpName(wrap_class, math_sym);
    
    atan_class = class_new(sym_atan, atan_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(atan_class, (t_method)atan_float);
    class_setHelpName(atan_class, math_sym);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
