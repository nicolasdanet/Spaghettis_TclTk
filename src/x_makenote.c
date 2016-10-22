
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
#include "s_midi.h"
#include "x_control.h"

/* -------------------------- makenote -------------------------- */

static t_class *makenote_class;

typedef struct _makenote
{
    t_object x_obj;
    t_float x_velo;
    t_float x_dur;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
    t_hang *x_hang;
} t_makenote;

static void *makenote_new(t_float velo, t_float dur)
{
    t_makenote *x = (t_makenote *)pd_new(makenote_class);
    x->x_velo = velo;
    x->x_dur = dur;
    inlet_newFloat(&x->x_obj, &x->x_velo);
    inlet_newFloat(&x->x_obj, &x->x_dur);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    x->x_hang = 0;
    return (x);
}

static void makenote_tick(t_hang *hang)
{
    t_makenote *x = hang->h_owner;
    t_hang *h2, *h3;
    outlet_float(x->x_velout, 0);
    outlet_float(x->x_pitchout, hang->h_pitch);
    if (x->x_hang == hang) x->x_hang = hang->h_next;
    else for (h2 = x->x_hang; h3 = h2->h_next; h2 = h3)
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

static void makenote_float(t_makenote *x, t_float f)
{
    t_hang *hang;
    if (!x->x_velo) return;
    outlet_float(x->x_velout, x->x_velo);
    outlet_float(x->x_pitchout, f);
    hang = (t_hang *)PD_MEMORY_GET(sizeof *hang);
    hang->h_next = x->x_hang;
    x->x_hang = hang;
    hang->h_pitch = f;
    hang->h_owner = x;
    hang->h_clock = clock_new(hang, (t_method)makenote_tick);
    clock_delay(hang->h_clock, (x->x_dur >= 0 ? x->x_dur : 0));
}

static void makenote_stop(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hang)
    {
        outlet_float(x->x_velout, 0);
        outlet_float(x->x_pitchout, hang->h_pitch);
        x->x_hang = hang->h_next;
        clock_free(hang->h_clock);
        PD_MEMORY_FREE(hang);
    }
}

static void makenote_clear(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hang)
    {
        x->x_hang = hang->h_next;
        clock_free(hang->h_clock);
        PD_MEMORY_FREE(hang);
    }
}

void makenote_setup(void)
{
    makenote_class = class_new(sym_makenote, 
        (t_newmethod)makenote_new, (t_method)makenote_clear,
        sizeof(t_makenote), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(makenote_class, makenote_float);
    class_addMethod(makenote_class, (t_method)makenote_stop, sym_stop,
        0);
    class_addMethod(makenote_class, (t_method)makenote_clear, sym_clear,
        0);
}
