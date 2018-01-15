
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *valuecommon_class;          /* Shared. */
static t_class *value_class;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _valuecommon {
    t_pd        x_pd;                       /* Must be the first. */
    int         x_referenceCount;
    t_float     x_f;
    } t_valuecommon;

typedef struct _value {
    t_object    x_obj;                      /* Must be the first. */
    t_symbol    *x_name;
    t_float     *x_raw;
    t_outlet    *x_outlet;
    } t_value;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float *valuecommon_fetch (t_symbol *s)
{
    t_valuecommon *x = (t_valuecommon *)symbol_getThingByClass (s, valuecommon_class);
    
    if (!x) {
        x = (t_valuecommon *)pd_new (valuecommon_class);
        x->x_referenceCount = 0;
        x->x_f = 0;
        pd_bind (cast_pd (x), s);
    }
    
    x->x_referenceCount++;
    
    return (&x->x_f);
}

void valuecommon_release (t_symbol *s)
{
    t_valuecommon *x = (t_valuecommon *)symbol_getThingByClass (s, valuecommon_class);
    
    PD_ASSERT (x != NULL);
    
    x->x_referenceCount--;
    
    if (!x->x_referenceCount) { pd_unbind (cast_pd (x), s); pd_free (cast_pd (x)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void value_set (t_value *x, t_symbol *s)
{
    if (x->x_name != &s_) { valuecommon_release (x->x_name); }
    
    x->x_raw  = NULL;
    x->x_name = s;
    
    if (x->x_name != &s_) { x->x_raw = valuecommon_fetch (x->x_name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void value_bang (t_value *x)
{
    if (x->x_raw) { outlet_float (x->x_outlet, *x->x_raw); }
}

static void value_float (t_value *x, t_float f)
{
    if (x->x_raw) { *x->x_raw = f; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *value_new (t_symbol *s)
{
    t_value *x = (t_value *)pd_new (value_class);
    
    x->x_name   = &s_;
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    value_set (x, s);
    
    return x;
}

static void value_free (t_value *x)
{
    value_set (x, &s_);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void value_setup (void)
{
    valuecommon_class = class_new (sym_value,
                                NULL,
                                NULL,
                                sizeof (t_valuecommon),
                                CLASS_NOBOX,
                                A_NULL);
    
    value_class = class_new (sym_value,
                                (t_newmethod)value_new,
                                (t_method)value_free,
                                sizeof (t_value),
                                CLASS_DEFAULT,
                                A_DEFSYMBOL,
                                A_NULL);
                                
    class_addCreator ((t_newmethod)value_new, sym_v, A_DEFSYMBOL, A_NULL);
    
    class_addBang (value_class, (t_method)value_bang);
    class_addFloat (value_class, (t_method)value_float);
    
    class_addMethod (value_class, (t_method)value_set, sym_set, A_DEFSYMBOL, A_NULL);
}

void value_destroy (void)
{
    class_free (valuecommon_class);
    class_free (value_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
