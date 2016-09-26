
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

static t_class *swap_class;

typedef struct _swap
{
    t_object x_obj;
    t_outlet *x_out2;
    t_float x_f1;
    t_float x_f2;
} t_swap;

static void *swap_new(t_float f)
{
    t_swap *x = (t_swap *)pd_new(swap_class);
    x->x_f2 = f;
    x->x_f1 = 0;
    outlet_new(&x->x_obj, &s_float);
    x->x_out2 = outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    return (x);
}

static void swap_bang(t_swap *x)
{
    outlet_float(x->x_out2, x->x_f1);
    outlet_float(x->x_obj.te_outlet, x->x_f2);
}

static void swap_float(t_swap *x, t_float f)
{
    x->x_f1 = f;
    swap_bang(x);
}

void swap_setup(void)
{
    swap_class = class_new(sym_swap, (t_newmethod)swap_new, 0,
        sizeof(t_swap), 0, A_DEFFLOAT, 0);
    class_addCreator((t_newmethod)swap_new, sym_fswap, A_DEFFLOAT, 0);
    class_addBang(swap_class, swap_bang);
    class_addFloat(swap_class, swap_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
