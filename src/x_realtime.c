
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
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"

static t_class *realtime_class;

typedef struct _realtime
{
    t_object x_obj;
    double x_setrealtime;
} t_realtime;

static void realtime_bang(t_realtime *x)
{
    x->x_setrealtime = sys_getRealTimeInSeconds();
}

static void realtime_bang2(t_realtime *x)
{
    outlet_float(x->x_obj.te_outlet,
        (sys_getRealTimeInSeconds() - x->x_setrealtime) * 1000.);
}

static void *realtime_new(void)
{
    t_realtime *x = (t_realtime *)pd_new(realtime_class);
    outlet_new(&x->x_obj, &s_float);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_bang, sym_inlet2);
    realtime_bang(x);
    return (x);
}

void realtime_setup(void)
{
    realtime_class = class_new(sym_realtime, (t_newmethod)realtime_new, 0,
        sizeof(t_realtime), 0, 0);
    class_addBang(realtime_class, realtime_bang);
    class_addMethod(realtime_class, (t_method)realtime_bang2, sym_inlet2,
        0);
}
