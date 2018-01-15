
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* One-pole low-pass filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/One_Pole.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *lop_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _lop_tilde_control {
    t_sample            c_real;
    t_sample            c_coefficient;
    } t_lop_tilde_control;

typedef struct _lop_tilde {
    t_object            x_obj;              /* Must be the first. */
    t_float             x_f;
    t_float             x_sampleRate;
    t_float             x_frequency;
    t_lop_tilde_control x_space;
    t_outlet            *x_outlet;
    } t_lop_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void lop_tilde_frequency (t_lop_tilde *x, t_float f)
{
    x->x_frequency           = (t_float)PD_MAX (0.0, f);
    x->x_sampleRate          = (x->x_sampleRate <= 0) ? AUDIO_DEFAULT_SAMPLERATE : x->x_sampleRate;
    x->x_space.c_coefficient = (t_sample)(f * PD_TWO_PI / x->x_sampleRate);
    x->x_space.c_coefficient = (t_sample)(PD_CLAMP (x->x_space.c_coefficient, 0.0, 1.0));
}

static void lop_tilde_clear (t_lop_tilde *x)
{
    x->x_space.c_real = (t_sample)0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *lop_tilde_perform (t_int *w)
{
    t_lop_tilde_control *c = (t_lop_tilde_control *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    t_sample last = c->c_real;
    t_sample b = c->c_coefficient;
    t_sample a = (t_sample)(1.0 - b);
    
    while (n--) {
        t_sample f = b * (*in++) + a * last;
        *out++ = last = f;
    }
    
    if (PD_IS_BIG_OR_SMALL (last)) { last = (t_sample)0.0; }
    
    c->c_real = last;
    
    return (w + 5);
}

static void lop_tilde_dsp (t_lop_tilde *x, t_signal **sp)
{
    x->x_sampleRate = sp[0]->s_sampleRate;
    
    lop_tilde_frequency (x, x->x_frequency);
    
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (lop_tilde_perform, 4, &x->x_space, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *lop_tilde_new (t_float f)
{
    t_lop_tilde *x = (t_lop_tilde *)pd_new (lop_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_new2 (x, &s_float);

    lop_tilde_frequency (x, f);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void lop_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_lop__tilde__,
            (t_newmethod)lop_tilde_new,
            NULL,
            sizeof (t_lop_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_lop_tilde, x_f);
    
    class_addDSP (c, (t_method)lop_tilde_dsp);
    
    class_addMethod (c, (t_method)lop_tilde_frequency,  sym__inlet2,    A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)lop_tilde_clear,      sym_clear,      A_NULL);
    
    lop_tilde_class = c;
}

void lop_tilde_destroy (void)
{
    class_free (lop_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
