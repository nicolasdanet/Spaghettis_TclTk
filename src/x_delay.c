
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DELAY_DEFAULT_DELAY         1000.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *delay_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _delay {
    t_object    x_obj;              /* Must be the first. */
    double      x_delay;
    t_outlet    *x_outlet;
    t_clock     *x_clock;
    } t_delay;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void delay_floatDelay (t_delay *, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void delay_task (t_delay *x)
{
    outlet_bang (x->x_outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void delay_bang (t_delay *x)
{
    clock_delay (x->x_clock, x->x_delay);
}

static void delay_float (t_delay *x, t_float f)
{
    delay_floatDelay (x, f);
    delay_bang (x);
}

static void delay_floatDelay (t_delay *x, t_float f)
{
    if (f < 0.0) { error_invalid (sym_delay, sym_delay); }
    else {
        x->x_delay = (double)((f == 0.0) ? DELAY_DEFAULT_DELAY : f);
    }
}

static void delay_stop (t_delay *x)
{
    clock_unset (x->x_clock);
}

/* Note that float arguments are always passed at last. */

static void delay_unit (t_delay *x, t_symbol *unitName, t_float f)
{
    t_error err = clock_setUnitParsed (x->x_clock, f, unitName);
    
    if (err) {
        error_invalid (sym_delay, sym_unit); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *delay_new (t_symbol *s, int argc, t_atom *argv)
{
    t_float f          = atom_getFloatAtIndex (0, argc, argv);
    t_float unit       = atom_getFloatAtIndex (1, argc, argv);
    t_symbol *unitName = atom_getSymbolAtIndex (2, argc, argv);
    
    t_delay *x = (t_delay *)pd_new (delay_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_bang);
    x->x_clock  = clock_new ((void *)x, (t_method)delay_task);
    
    inlet_new2 (x, &s_float);
    
    delay_floatDelay (x, f);
    
    if (unit != 0.0) { delay_unit (x, unitName, unit); }
    
    return x;
}

static void delay_free (t_delay *x)
{
    clock_free (x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void delay_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_delay,
            (t_newmethod)delay_new,
            (t_method)delay_free,
            sizeof (t_delay),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addCreator ((t_newmethod)delay_new, sym_del, A_GIMME, A_NULL);
    
    class_addBang (c, (t_method)delay_bang);
    class_addFloat (c, (t_method)delay_float);
        
    class_addMethod (c, (t_method)delay_floatDelay, sym__inlet2,    A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)delay_stop,       sym_stop,       A_NULL);
    class_addMethod (c, (t_method)delay_unit,       sym_unit,       A_FLOAT, A_SYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)delay_unit,       sym_tempo,      A_FLOAT, A_SYMBOL, A_NULL);
        
    #endif
    
    delay_class = c;
}

void delay_destroy (void)
{
    class_free (delay_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
