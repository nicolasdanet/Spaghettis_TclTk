
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *value_class, *vcommon_class;

typedef struct vcommon
{
    t_pd c_pd;
    int c_refcount;
    t_float c_f;
} t_vcommon;

typedef struct _value
{
    t_object x_obj;
    t_symbol *x_sym;
    t_float *x_floatstar;
} t_value;

    /* get a pointer to a named floating-point variable.  The variable
    belongs to a "vcommon" object, which is created if necessary. */
t_float *value_get(t_symbol *s)
{
    t_vcommon *c = (t_vcommon *)pd_findByClass(s, vcommon_class);
    if (!c)
    {
        c = (t_vcommon *)pd_new(vcommon_class);
        c->c_f = 0;
        c->c_refcount = 0;
        pd_bind(&c->c_pd, s);
    }
    c->c_refcount++;
    return (&c->c_f);
}

    /* release a variable.  This only frees the "vcommon" resource when the
    last interested party releases it. */
void value_release(t_symbol *s)
{
    t_vcommon *c = (t_vcommon *)pd_findByClass(s, vcommon_class);
    if (c)
    {
        if (!--c->c_refcount)
        {
            pd_unbind(&c->c_pd, s);
            pd_free(&c->c_pd);
        }
    }
    else { PD_BUG; }
}

/*
 * value_getfloat -- obtain the float value of a "value" object 
 *                  return 0 on success, 1 otherwise
 */
int
value_getfloat(t_symbol *s, t_float *f) 
{
    t_vcommon *c = (t_vcommon *)pd_findByClass(s, vcommon_class);
    if (!c)
        return (1);
    *f = c->c_f;
    return (0); 
}
 
/*
 * value_setfloat -- set the float value of a "value" object
 *                  return 0 on success, 1 otherwise
 */
int
value_setfloat(t_symbol *s, t_float f)
{
    t_vcommon *c = (t_vcommon *)pd_findByClass(s, vcommon_class);
    if (!c)
        return (1);
    c->c_f = f; 
    return (0); 
}

static void vcommon_float(t_vcommon *x, t_float f)
{
    x->c_f = f;
}

static void *value_new(t_symbol *s)
{
    t_value *x = (t_value *)pd_new(value_class);
    x->x_sym = s;
    x->x_floatstar = value_get(s);
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void value_bang(t_value *x)
{
    outlet_float(x->x_obj.te_outlet, *x->x_floatstar);
}

static void value_float(t_value *x, t_float f)
{
    *x->x_floatstar = f;
}

static void value_ff(t_value *x)
{
    value_release(x->x_sym);
}

void value_setup(void)
{
    value_class = class_new(sym_value, (t_newmethod)value_new,
        (t_method)value_ff,
        sizeof(t_value), 0, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)value_new, sym_v, A_DEFSYMBOL, 0);
    class_addBang(value_class, value_bang);
    class_addFloat(value_class, value_float);
    vcommon_class = class_new(sym_value, 0, 0,
        sizeof(t_vcommon), CLASS_NOBOX, 0);
    class_addFloat(vcommon_class, vcommon_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
