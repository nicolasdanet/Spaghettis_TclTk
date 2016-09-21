
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

void delay_setup(void)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
