
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* One-pole one-zero hipass filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.embedded.com/print/4007653 > */
/* < https://ccrma.stanford.edu/~jos/fp/DC_Blocker.html > */
/* < http://msp.ucsd.edu/techniques/v0.11/book-html/node141.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *hip_tilde_class;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _hip_tilde_control {
    t_sample            c_real;
    t_sample            c_coefficient;
    } t_hip_tilde_control;

typedef struct _hip_tilde {
    t_object            x_obj;                  /* Must be the first. */
    t_float             x_f;
    t_float             x_frequency;
    t_float             x_sampleRate;
    t_hip_tilde_control x_space;
    t_outlet            *x_outlet;
    } t_hip_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void hip_tilde_frequency (t_hip_tilde *x, t_float f)
{
    x->x_frequency           = (t_float)PD_MAX (0.0, f);
    x->x_sampleRate          = (x->x_sampleRate <= 0) ? AUDIO_DEFAULT_SAMPLERATE : x->x_sampleRate;
    x->x_space.c_coefficient = (t_sample)(1.0 - x->x_frequency * PD_TWO_PI / x->x_sampleRate);
    x->x_space.c_coefficient = (t_sample)PD_CLAMP (x->x_space.c_coefficient, 0.0, 1.0);
}

static void hip_tilde_clear (t_hip_tilde *x)
{
    x->x_space.c_real = (t_sample)0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *hip_tilde_perform (t_int *w)
{
    t_hip_tilde_control *c = (t_hip_tilde_control *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (t_int)(w[4]);
    
    t_sample a = c->c_coefficient;
    
    if (a < 1.0) {

        t_sample normalize = (t_sample)(0.5 * (1.0 + a));
        t_sample last = c->c_real;
        
        while (n--) {
            t_sample f = (*in++) + (a * last);
            *out++ = normalize * (f - last);
            last = f;
        }
        
        if (PD_IS_BIG_OR_SMALL (last)) { last = (t_sample)0.0; }
        
        c->c_real = last;

    } else { while (n--) { *out++ = *in++; } c->c_real = 0; }
    
    return (w + 5);
}

static void hip_tilde_dsp (t_hip_tilde *x, t_signal **sp)
{
    x->x_sampleRate = sp[0]->s_sampleRate;
    
    hip_tilde_frequency (x, x->x_frequency);
    
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (hip_tilde_perform, 4, &x->x_space, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *hip_tilde_new (t_float f)
{
    t_hip_tilde *x = (t_hip_tilde *)pd_new (hip_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);
    
    hip_tilde_frequency (x, f);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void hip_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_hip__tilde__,
            (t_newmethod)hip_tilde_new,
            NULL,
            sizeof (t_hip_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
    
    CLASS_SIGNAL (c, t_hip_tilde, x_f);
    
    class_addDSP (c, (t_method)hip_tilde_dsp);
    
    class_addMethod (c, (t_method)hip_tilde_frequency,  sym_inlet2, A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)hip_tilde_clear,      sym_clear,  A_NULL);
    
    hip_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
