
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
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabplay_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabplay_tilde {
    t_object    x_obj;                      /* Must be the first. */
    int         x_phase;
    int         x_end;
    int         x_size;
    t_word      *x_vector;
    t_symbol    *x_name;
    t_clock     *x_clock;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_tabplay_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabplay_tilde_task (t_tabplay_tilde *x)
{
    outlet_bang (x->x_outletRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabplay_tilde_list (t_tabplay_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    int start  = (int)atom_getFloatAtIndex (0, argc, argv);
    int length = (int)atom_getFloatAtIndex (1, argc, argv);
    
    x->x_phase = PD_MAX (0, start);
    x->x_end   = (length > 0) ? x->x_phase + length : PD_INT_MAX;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabplay_tilde_set (t_tabplay_tilde *x, t_symbol *s)
{
    x->x_vector = NULL;
    x->x_name   = s;
    
    {
        t_garray *a = (t_garray *)pd_getThingByClass (x->x_name, garray_class);
        
        if (!a) { if (s != &s_) { error_canNotFind (sym_tabplay__tilde__, x->x_name); } }
        else {
            garray_getData (a, &x->x_size, &x->x_vector);
            garray_setAsUsedInDSP (a);
        }
    }
}

static void tabplay_tilde_stop (t_tabplay_tilde *x)
{
    x->x_phase = PD_INT_MAX;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *tabplay_tilde_perform (t_int *w)
{
    t_tabplay_tilde *x = (t_tabplay_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    int phase = x->x_phase;
    int end   = PD_MIN (x->x_size, x->x_end);

    if (x->x_vector && phase < end) {
    //
    t_word *data = x->x_vector + phase;
    int size = PD_MIN (n, end - phase);
    int pad  = n - size;
    
    phase += size;
    
    while (size--) { *out++ = WORD_FLOAT (data); data++; }
    while (pad--)  { *out++ = 0.0; }
            
    if (phase >= end) { clock_delay (x->x_clock, 0.0); x->x_phase = PD_INT_MAX; }
    else { 
        x->x_phase = phase;
    }
    //
    } else { while (n--) { *out++ = 0.0; } }
    
    return (w + 4);
}

static void tabplay_tilde_dsp (t_tabplay_tilde *x, t_signal **sp)
{
    tabplay_tilde_set (x, x->x_name);
    
    dsp_add (tabplay_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tabplay_tilde_new (t_symbol *s)
{
    t_tabplay_tilde *x = (t_tabplay_tilde *)pd_new (tabplay_tilde_class);
    
    x->x_phase       = PD_INT_MAX;
    x->x_name        = s;
    x->x_clock       = clock_new ((void *)x, (t_method)tabplay_tilde_task);
    x->x_outletLeft  = outlet_new (cast_object (x), &s_signal);
    x->x_outletRight = outlet_new (cast_object (x), &s_bang);
    
    return x;
}

static void tabplay_tilde_free (t_tabplay_tilde *x)
{
    clock_free (x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tabplay_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabplay__tilde__,
            (t_newmethod)tabplay_tilde_new,
            (t_method)tabplay_tilde_free,
            sizeof (t_tabplay_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addDSP (c, tabplay_tilde_dsp);
    
    class_addList (c, tabplay_tilde_list);
        
    class_addMethod (c, (t_method)tabplay_tilde_set,    sym_set,    A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)tabplay_tilde_stop,   sym_stop,   A_NULL);
    
    tabplay_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
