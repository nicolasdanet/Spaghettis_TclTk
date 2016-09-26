
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

static t_class *change_class;

typedef struct _change
{
    t_object x_obj;
    t_float x_f;
} t_change;

static void *change_new(t_float f)
{
    t_change *x = (t_change *)pd_new(change_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void change_bang(t_change *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f);
}

static void change_float(t_change *x, t_float f)
{
    if (f != x->x_f)
    {
        x->x_f = f;
        outlet_float(x->x_obj.te_outlet, x->x_f);
    }
}

static void change_set(t_change *x, t_float f)
{
    x->x_f = f;
}

void change_setup(void)
{
    change_class = class_new(sym_change, (t_newmethod)change_new, 0,
        sizeof(t_change), 0, A_DEFFLOAT, 0);
    class_addBang(change_class, change_bang);
    class_addFloat(change_class, change_float);
    class_addMethod(change_class, (t_method)change_set, sym_set,
        A_DEFFLOAT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
