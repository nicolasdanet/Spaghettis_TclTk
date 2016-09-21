
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

/* -------------------------- timer ------------------------------ */
static t_class *timer_class;

typedef struct _timer
{
    t_object x_obj;
    double x_settime;
    double x_moreelapsed;
    t_float x_unit;
    int x_samps;
} t_timer;

static void timer_bang(t_timer *x)
{
    x->x_settime = scheduler_getLogicalTime();
    x->x_moreelapsed = 0;
}

static void timer_bang2(t_timer *x)
{
    outlet_float(x->x_obj.te_outlet,
        scheduler_getUnitsSince(x->x_settime, x->x_unit, x->x_samps)
            + x->x_moreelapsed);
}

static void timer_tempo(t_timer *x, t_float f, t_symbol *unitName)
{
    x->x_moreelapsed += scheduler_getUnitsSince(x->x_settime, x->x_unit, x->x_samps);
    x->x_settime = scheduler_getLogicalTime();

    clock_parseUnit (f, unitName, &x->x_unit, &x->x_samps);
}

static void *timer_new(t_symbol *unitname, t_float tempo)
{
    t_timer *x = (t_timer *)pd_new(timer_class);
    x->x_unit = 1;
    x->x_samps = 0;
    timer_bang(x);
    outlet_new(&x->x_obj, sym_float);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, sym_bang, sym_bang2);
    if (tempo != 0)
        timer_tempo(x, tempo, unitname);
    return (x);
}

void timer_setup(void)
{
    timer_class = class_new(sym_timer, (t_newmethod)timer_new, 0,
        sizeof(t_timer), 0, A_DEFFLOAT, A_DEFSYMBOL, 0);
    class_addBang(timer_class, timer_bang);
    class_addMethod(timer_class, (t_method)timer_bang2, sym_bang2, 0);
    class_addMethod(timer_class, (t_method)timer_tempo,
        sym_tempo, A_FLOAT, A_SYMBOL, 0); /* LEGACY !!! */
    class_addMethod(timer_class, (t_method)timer_tempo,
        sym_unit, A_FLOAT, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
