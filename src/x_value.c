
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
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
#pragma mark -

t_float *valuecommon_fetch (t_symbol *s)
{
    t_valuecommon *x = (t_valuecommon *)pd_getThingByClass (s, valuecommon_class);
    
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
    t_valuecommon *x = (t_valuecommon *)pd_getThingByClass (s, valuecommon_class);
    
    PD_ASSERT (x != NULL);
    
    x->x_referenceCount--;
    
    if (!x->x_referenceCount) { pd_unbind (cast_pd (x), s); pd_free (cast_pd (x)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void value_bang (t_value *x)
{
    outlet_float (x->x_outlet, *x->x_raw);
}

static void value_float (t_value *x, t_float f)
{
    *x->x_raw = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *value_new (t_symbol *s)
{
    t_value *x = (t_value *)pd_new (value_class);
    
    x->x_name   = s;
    x->x_raw    = valuecommon_fetch (s);
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void value_free (t_value *x)
{
    valuecommon_release (x->x_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
}

void value_destroy (void)
{
    CLASS_FREE (valuecommon_class);
    CLASS_FREE (value_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
