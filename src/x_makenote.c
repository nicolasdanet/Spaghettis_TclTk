
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *makenote_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _hang {
    t_float             h_pitch;
    t_clock             *h_clock;
    struct _makenote    *h_owner;
    struct _hang        *h_next;
    } t_hang;

typedef struct _makenote {
    t_object            x_obj;              /* Must be the first. */
    t_float             x_velocity;
    t_float             x_duration;
    t_hang              *x_hangs;
    t_outlet            *x_outletLeft;
    t_outlet            *x_outletRight;
    } t_makenote;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void makenote_task (t_hang *h)
{
    t_makenote *x = h->h_owner;
    
    outlet_float (x->x_outletRight, (t_float)0.0);
    outlet_float (x->x_outletLeft,  h->h_pitch);
    
    if (x->x_hangs == h) { x->x_hangs = h->h_next; }
    else {
        t_hang *h1 = NULL;
        t_hang *h2 = NULL;
        
        for ((h1 = x->x_hangs); (h2 = h1->h_next); (h1 = h2)) {
            if (h2 == h) { 
                h1->h_next = h2->h_next; break; 
            }
        }
    }
    
    clock_free (h->h_clock);
    
    PD_MEMORY_FREE (h);
}

static void makenote_add (t_makenote *x, t_float pitch, t_float duration)
{
    t_hang *h = (t_hang *)PD_MEMORY_GET (sizeof (t_hang));
    
    h->h_pitch = pitch;
    h->h_clock = clock_new ((void *)h, (t_method)makenote_task);
    h->h_owner = x;
    h->h_next  = x->x_hangs;
    
    x->x_hangs = h;
    
    clock_delay (h->h_clock, PD_MAX (0.0, duration));
}

static void makenote_removeAll (t_makenote *x, int dump)
{
    t_hang *h = NULL;
    
    while ((h = x->x_hangs)) {
    //
    if (dump) {
        outlet_float (x->x_outletRight, (t_float)0.0);
        outlet_float (x->x_outletLeft,  h->h_pitch);
    }

    x->x_hangs = h->h_next; clock_free (h->h_clock); PD_MEMORY_FREE (h);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void makenote_float (t_makenote *x, t_float pitch)
{
    if (x->x_velocity) {
    //
    outlet_float (x->x_outletRight, x->x_velocity);
    outlet_float (x->x_outletLeft,  pitch);
    makenote_add (x, pitch, x->x_duration);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void makenote_stop (t_makenote *x)
{
    makenote_removeAll (x, 1);
}

static void makenote_clear (t_makenote *x)
{
    makenote_removeAll (x, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *makenote_new (t_float velocity, t_float duration)
{
    t_makenote *x = (t_makenote *)pd_new (makenote_class);
    
    x->x_velocity    = velocity;
    x->x_duration    = duration;
    x->x_hangs       = NULL;
    x->x_outletLeft  = outlet_newFloat (cast_object (x));
    x->x_outletRight = outlet_newFloat (cast_object (x));
    
    inlet_newFloat (cast_object (x), &x->x_velocity);
    inlet_newFloat (cast_object (x), &x->x_duration);

    return x;
}

static void makenote_free (t_makenote *x)
{
    makenote_clear (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void makenote_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_makenote, 
            (t_newmethod)makenote_new,
            (t_method)makenote_free,
            sizeof (t_makenote),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addFloat (c, (t_method)makenote_float);
    
    class_addMethod (c, (t_method)makenote_stop,    sym_stop,   A_NULL);
    class_addMethod (c, (t_method)makenote_stop,    sym_flush,  A_NULL);
    class_addMethod (c, (t_method)makenote_clear,   sym_clear,  A_NULL);
    
    makenote_class = c;
}

void makenote_destroy (void)
{
    class_free (makenote_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
