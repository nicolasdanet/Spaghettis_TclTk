
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
#pragma mark -

static void makenote_tick(t_hang *hang)
{
    t_makenote *x = hang->h_owner;
    t_hang *h2, *h3;
    outlet_float(x->x_outletRight, 0);
    outlet_float(x->x_outletLeft, hang->h_pitch);
    if (x->x_hangs == hang) x->x_hangs = hang->h_next;
    else for (h2 = x->x_hangs; h3 = h2->h_next; h2 = h3)
    {
        if (h3 == hang)
        {
            h2->h_next = h3->h_next;
            break;
        }
    }
    clock_free(hang->h_clock);
    PD_MEMORY_FREE(hang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void makenote_float(t_makenote *x, t_float f)
{
    t_hang *hang;
    if (!x->x_velocity) return;
    outlet_float(x->x_outletRight, x->x_velocity);
    outlet_float(x->x_outletLeft, f);
    hang = (t_hang *)PD_MEMORY_GET(sizeof *hang);
    hang->h_next = x->x_hangs;
    x->x_hangs = hang;
    hang->h_pitch = f;
    hang->h_owner = x;
    hang->h_clock = clock_new(hang, (t_method)makenote_tick);
    clock_delay(hang->h_clock, (x->x_duration >= 0 ? x->x_duration : 0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void makenote_stop(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hangs)
    {
        outlet_float(x->x_outletRight, 0);
        outlet_float(x->x_outletLeft, hang->h_pitch);
        x->x_hangs = hang->h_next;
        clock_free(hang->h_clock);
        PD_MEMORY_FREE(hang);
    }
}

static void makenote_clear(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hangs)
    {
        x->x_hangs = hang->h_next;
        clock_free(hang->h_clock);
        PD_MEMORY_FREE(hang);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *makenote_new (t_float velocity, t_float duration)
{
    t_makenote *x = (t_makenote *)pd_new (makenote_class);
    
    x->x_velocity    = velocity;
    x->x_duration    = duration;
    x->x_hangs       = NULL;
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_float);
    
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
#pragma mark -

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
            
    class_addFloat (c, makenote_float);
    
    class_addMethod (c, (t_method)makenote_stop,    sym_stop,   A_NULL);
    class_addMethod (c, (t_method)makenote_clear,   sym_clear,  A_NULL);
    
    makenote_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
