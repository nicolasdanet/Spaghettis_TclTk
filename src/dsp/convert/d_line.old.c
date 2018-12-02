
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *line_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _line_tilde {
    t_object            x_obj;          /* Must be the first. */
    pthread_mutex_t     x_mutex;
    t_float             x_f;
    t_float             x_base;
    t_float             x_target;
    t_float             x_time;
    int                 x_stop;
    int                 x_rebase;
    int                 x_retarget;
    int                 x_ticks;
    t_float             x_current;
    t_outlet            *x_outlet;
    } t_line_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_float line_tilde_target (t_line_tilde *x)
{
    t_float f = 0.0;
    
    pthread_mutex_lock (&x->x_mutex);
    
        f = x->x_target;
    
    pthread_mutex_unlock (&x->x_mutex);
    
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void line_tilde_float (t_line_tilde *x, t_float f)
{
    pthread_mutex_lock (&x->x_mutex);

        x->x_target   = f;
        x->x_time     = PD_MAX (0.0, x->x_f);
        x->x_f        = 0.0;
        x->x_stop     = 0;
        x->x_retarget = 1;
    
        if (x->x_time == 0.0) { x->x_base = f; x->x_rebase = 1; }
    
    pthread_mutex_unlock (&x->x_mutex);
}

static void line_tilde_stop (t_line_tilde *x)
{
    pthread_mutex_lock (&x->x_mutex);
    
        x->x_stop     = 1;
        x->x_retarget = 1;
    
    pthread_mutex_unlock (&x->x_mutex);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *line_tilde_perform (t_int *w)
{
    t_line_tilde *x   = (t_line_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    t_space *t        = (t_space *)(w[3]);
    int n = (int)(w[4]);
    
    t_float inverseOfVectorSize = t->s_float0;
    t_float millisecondsToTicks = t->s_float1;
    
    if (pthread_mutex_trylock (&x->x_mutex) == 0) {
    //
    if (x->x_retarget) {
        x->x_current  = x->x_rebase ? x->x_base : x->x_current;
        x->x_target   = x->x_stop   ? x->x_current : x->x_target;
        x->x_ticks    = x->x_stop   ? 0 : (int)(x->x_time * millisecondsToTicks);
        t->s_float2   = x->x_target;
        t->s_float3   = x->x_ticks ? ((x->x_target - x->x_current) / x->x_ticks) : 0.0;
        x->x_stop     = 0;
        x->x_rebase   = 0;
        x->x_retarget = 0;
    }
    
    pthread_mutex_unlock (&x->x_mutex);
    //
    }
    
    if (x->x_ticks) {
    //
    t_float f = x->x_current; x->x_current += t->s_float3;
    
    while (n--) { *out++ = (t_sample)f; f += t->s_float3 * inverseOfVectorSize; }
    
    x->x_ticks--;
    //
    } else { x->x_current = t->s_float2; while (n--) { *out++ = (t_sample)t->s_float2; } }
    
    return (w + 5);
}

static void line_tilde_dsp (t_line_tilde *x, t_signal **sp)
{
    t_space *t = space_new();

    t->s_float0 = (t_float)(1.0 / sp[0]->s_vectorSize);
    t->s_float1 = (t_float)(sp[0]->s_sampleRate / (1000.0 * sp[0]->s_vectorSize));
    t->s_float2 = line_tilde_target (x);
    
    dsp_add (line_tilde_perform, 4, x, sp[0]->s_vector, t, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *line_tilde_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_line_tilde *x = (t_line_tilde *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, &s_float);
    buffer_appendFloat (b, line_tilde_target (x));
    
    return b;
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *line_tilde_new (void)
{
    t_line_tilde *x = (t_line_tilde *)pd_new (line_tilde_class);
    
    pthread_mutex_init (&x->x_mutex, NULL);
    
    x->x_outlet = outlet_newSignal (cast_object (x));
    
    inlet_newFloat (cast_object (x), &x->x_f);
    
    return x;
}

static void line_tilde_free (t_line_tilde *x)
{
    pthread_mutex_destroy (&x->x_mutex);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void line_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_line__tilde__,
            (t_newmethod)line_tilde_new,
            (t_method)line_tilde_free,
            sizeof (t_line_tilde),
            CLASS_DEFAULT,
            A_NULL);
        
    class_addDSP (c, (t_method)line_tilde_dsp);
    class_addFloat (c, (t_method)line_tilde_float);
    
    class_addMethod (c, (t_method)line_tilde_stop, sym_stop, A_NULL);
    
    class_setDataFunction (c, line_tilde_functionData);
    
    #if PD_WITH_LEGACY
    
    class_addCreator ((t_newmethod)line_tilde_new, sym_vline__tilde__, A_NULL);

    #endif
    
    line_tilde_class = c;
}

void line_tilde_destroy (void)
{
    class_free (line_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
