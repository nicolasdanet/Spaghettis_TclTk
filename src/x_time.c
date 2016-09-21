
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

/* -------------------------- delay ------------------------------ */
static t_class *delay_class;

typedef struct _delay
{
    t_object x_obj;
    t_clock *x_clock;
    double x_deltime;
} t_delay;

static void delay_ft1(t_delay *x, t_float g)
{
    if (g < 0) g = 0;
    x->x_deltime = g;
}

static void delay_tick(t_delay *x)
{
    outlet_bang(x->x_obj.te_outlet);
}

static void delay_bang(t_delay *x)
{
    clock_delay(x->x_clock, x->x_deltime);
}

static void delay_stop(t_delay *x)
{
    clock_unset(x->x_clock);
}

static void delay_float(t_delay *x, t_float f)
{
    delay_ft1(x, f);
    delay_bang(x);
}

static void delay_tempo(t_delay *x, t_float f, t_symbol *unitName)
{
    t_error err = clock_setUnitParsed (x->x_clock, f, unitName);
    
    if (err) {
        error_invalid (sym_delay, sym_unit); 
    }
}

static void delay_free(t_delay *x)
{
    clock_free(x->x_clock);
}

static void *delay_new(t_symbol *unitname, t_float f, t_float tempo)
{
    t_delay *x = (t_delay *)pd_new(delay_class);
    delay_ft1(x, f);
    x->x_clock = clock_new(x, (t_method)delay_tick);
    outlet_new(&x->x_obj, sym_bang);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, sym_float, sym_ft1);
    if (tempo != 0)
        delay_tempo(x, tempo, unitname);
    return (x);
}

static void delay_setup(void)
{
    delay_class = class_new(sym_delay, (t_newmethod)delay_new,
        (t_method)delay_free, sizeof(t_delay), 0,
            A_DEFFLOAT, A_DEFFLOAT, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)delay_new, sym_del,
        A_DEFFLOAT, A_DEFFLOAT, A_DEFSYMBOL, 0);
    class_addBang(delay_class, delay_bang);
    class_addMethod(delay_class, (t_method)delay_stop, sym_stop, 0);
    class_addMethod(delay_class, (t_method)delay_ft1,
        sym_ft1, A_FLOAT, 0);
    class_addMethod(delay_class, (t_method)delay_tempo,
        sym_tempo, A_FLOAT, A_SYMBOL, 0); /* LEGACY !!! */
    class_addMethod(delay_class, (t_method)delay_tempo,
        sym_unit, A_FLOAT, A_SYMBOL, 0);
    class_addFloat(delay_class, (t_method)delay_float);
}

/* -------------------------- metro ------------------------------ */
static t_class *metro_class;

typedef struct _metro
{
    t_object x_obj;
    t_clock *x_clock;
    double x_deltime;
    int x_hit;
} t_metro;

static void metro_ft1(t_metro *x, t_float g)
{
    if (g <= 0) /* as of 0.45, we're willing to try any positive time value */
        g = 1;  /* but default to 1 (arbitrary and probably not so good) */
    x->x_deltime = g;
}

static void metro_tick(t_metro *x)
{
    x->x_hit = 0;
    outlet_bang(x->x_obj.te_outlet);
    if (!x->x_hit) clock_delay(x->x_clock, x->x_deltime);
}

static void metro_float(t_metro *x, t_float f)
{
    if (f != 0) metro_tick(x);
    else clock_unset(x->x_clock);
    x->x_hit = 1;
}

static void metro_bang(t_metro *x)
{
    metro_float(x, 1);
}

static void metro_stop(t_metro *x)
{
    metro_float(x, 0);
}

static void metro_tempo(t_metro *x, t_float f, t_symbol *unitName)
{
    t_error err = clock_setUnitParsed (x->x_clock, f, unitName);
    
    if (err) {
        error_invalid (sym_delay, sym_unit); 
    }
}

static void metro_free(t_metro *x)
{
    clock_free(x->x_clock);
}

static void *metro_new(t_symbol *unitname, t_float f, t_float tempo)
{
    t_metro *x = (t_metro *)pd_new(metro_class);
    metro_ft1(x, f);
    x->x_hit = 0;
    x->x_clock = clock_new(x, (t_method)metro_tick);
    outlet_new(&x->x_obj, sym_bang);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, sym_float, sym_ft1);
    if (tempo != 0)
        metro_tempo(x, tempo, unitname);
    return (x);
}

static void metro_setup(void)
{
    metro_class = class_new(sym_metro, (t_newmethod)metro_new,
        (t_method)metro_free, sizeof(t_metro), 0,
            A_DEFFLOAT, A_DEFFLOAT, A_DEFSYMBOL, 0);
    class_addBang(metro_class, metro_bang);
    class_addMethod(metro_class, (t_method)metro_stop, sym_stop, 0);
    class_addMethod(metro_class, (t_method)metro_ft1, sym_ft1,
        A_FLOAT, 0);
    class_addMethod(metro_class, (t_method)metro_tempo,
        sym_tempo, A_FLOAT, A_SYMBOL, 0); /* LEGACY !!! */
    class_addMethod(metro_class, (t_method)metro_tempo,
        sym_unit, A_FLOAT, A_SYMBOL, 0);
    class_addFloat(metro_class, (t_method)metro_float);
}

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

static void line_setup(void)
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

void x_time_setup(void)
{
    delay_setup();
    metro_setup();
    line_setup();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
