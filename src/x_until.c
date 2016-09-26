
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

static t_class *until_class;

typedef struct _until
{
    t_object x_obj;
    int x_run;
    int x_count;
} t_until;

static void *until_new(void)
{
    t_until *x = (t_until *)pd_new(until_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_bang, sym_inlet2);
    outlet_new(&x->x_obj, &s_bang);
    x->x_run = 0;
    return (x);
}

static void until_bang(t_until *x)
{
    x->x_run = 1;
    x->x_count = -1;
    while (x->x_run && x->x_count)
        x->x_count--, outlet_bang(x->x_obj.te_outlet);
}

static void until_float(t_until *x, t_float f)
{
    if (f < 0)
        f = 0;
    x->x_run = 1;
    x->x_count = f;
    while (x->x_run && x->x_count)
        x->x_count--, outlet_bang(x->x_obj.te_outlet);
}

static void until_bang2(t_until *x)
{
    x->x_run = 0;
}

void until_setup(void)
{
    until_class = class_new(sym_until, (t_newmethod)until_new, 0,
        sizeof(t_until), 0, 0);
    class_addBang(until_class, until_bang);
    class_addFloat(until_class, until_float);
    class_addMethod(until_class, (t_method)until_bang2, sym_inlet2, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
