
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
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


/* -------------------------- line ------------------------------ */
#define DEFAULTLINEGRAIN 20
static t_class *line_class;

typedef struct _line
{
    t_object x_obj;
    t_clock *x_clock;
    double x_targettime;
    t_float x_targetval;
    double x_prevtime;
    t_float x_setval;
    int x_gotinlet;
    t_float x_grain;
    double x_1overtimediff;
    double x_in1val;
} t_line;

static void line_tick(t_line *x)
{
    t_systime timenow = scheduler_getLogicalTime();
    double msectogo = - scheduler_getMillisecondsSince(x->x_targettime);
    if (msectogo < 1E-9)
    {
        outlet_float(x->x_obj.te_outlet, x->x_targetval);
    }
    else
    {
        outlet_float(x->x_obj.te_outlet,
            x->x_setval + x->x_1overtimediff * (timenow - x->x_prevtime)
                * (x->x_targetval - x->x_setval));
        if (x->x_grain <= 0)
            x->x_grain = DEFAULTLINEGRAIN;
        clock_delay(x->x_clock,
            (x->x_grain > msectogo ? msectogo : x->x_grain));
    }
}

static void line_float(t_line *x, t_float f)
{
    t_systime timenow = scheduler_getLogicalTime();
    if (x->x_gotinlet && x->x_in1val > 0)
    {
        if (timenow > x->x_targettime) x->x_setval = x->x_targetval;
        else x->x_setval = x->x_setval + x->x_1overtimediff *
            (timenow - x->x_prevtime)
            * (x->x_targetval - x->x_setval);
        x->x_prevtime = timenow;
        x->x_targettime = scheduler_getLogicalTimeAfter(x->x_in1val);
        x->x_targetval = f;
        line_tick(x);
        x->x_gotinlet = 0;
        x->x_1overtimediff = 1./ (x->x_targettime - timenow);
        if (x->x_grain <= 0)
            x->x_grain = DEFAULTLINEGRAIN;
        clock_delay(x->x_clock,
            (x->x_grain > x->x_in1val ? x->x_in1val : x->x_grain));
    
    }
    else
    {
        clock_unset(x->x_clock);
        x->x_targetval = x->x_setval = f;
        outlet_float(x->x_obj.te_outlet, f);
    }
    x->x_gotinlet = 0;
}

static void line_ft1(t_line *x, t_float g)
{
    x->x_in1val = g;
    x->x_gotinlet = 1;
}

static void line_stop(t_line *x)
{
    x->x_targetval = x->x_setval;
    clock_unset(x->x_clock);
}

static void line_set(t_line *x, t_float f)
{
    clock_unset(x->x_clock);
    x->x_targetval = x->x_setval = f;
}

static void line_free(t_line *x)
{
    clock_free(x->x_clock);
}

static void *line_new(t_float f, t_float grain)
{
    t_line *x = (t_line *)pd_new(line_class);
    x->x_targetval = x->x_setval = f;
    x->x_gotinlet = 0;
    x->x_1overtimediff = 1;
    x->x_clock = clock_new(x, (t_method)line_tick);
    x->x_targettime = x->x_prevtime = scheduler_getLogicalTime();
    x->x_grain = grain;
    outlet_new(&x->x_obj, sym_float);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, sym_float, sym_ft1);
    inlet_newFloat(&x->x_obj, &x->x_grain);
    return (x);
}

void line_setup(void)
{
    line_class = class_new(sym_line, (t_newmethod)line_new,
        (t_method)line_free, sizeof(t_line), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(line_class, (t_method)line_ft1,
        sym_ft1, A_FLOAT, 0);
    class_addMethod(line_class, (t_method)line_stop,
        sym_stop, 0);
    class_addMethod(line_class, (t_method)line_set,
        sym_set, A_FLOAT, 0);
    class_addFloat(line_class, (t_method)line_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
