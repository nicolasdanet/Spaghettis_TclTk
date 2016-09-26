
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

static t_class *moses_class;

typedef struct _moses
{
    t_object x_ob;
    t_outlet *x_out2;
    t_float x_y;
} t_moses;

static void *moses_new(t_float f)
{
    t_moses *x = (t_moses *)pd_new(moses_class);
    inlet_newFloat(&x->x_ob, &x->x_y);
    outlet_new(&x->x_ob, &s_float);
    x->x_out2 = outlet_new(&x->x_ob, &s_float);
    x->x_y = f;
    return (x);
}

static void moses_float(t_moses *x, t_float f)
{
    if (f < x->x_y) outlet_float(x->x_ob.te_outlet, f);
    else outlet_float(x->x_out2, f);
}

void moses_setup(void)
{
    moses_class = class_new(sym_moses, (t_newmethod)moses_new, 0,
        sizeof(t_moses), 0, A_DEFFLOAT, 0);
    class_addFloat(moses_class, moses_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
