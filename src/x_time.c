
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void x_time_setup(void)
{
    delay_setup();
    metro_setup();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
