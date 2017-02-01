
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

/* Two-pole bandpass filter. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://ccrma.stanford.edu/~jos/filters/Two_Pole.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *bp_tilde_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _bp_tilde_control {
    t_sample            c_real1;
    t_sample            c_real2;
    t_sample            c_a1;
    t_sample            c_a2;
    t_sample            c_gain;
    } t_bp_tilde_control;

typedef struct _bp_tilde {
    t_object            x_obj;                  /* Must be the first. */
    t_float             x_f;
    t_float             x_sampleRate;
    t_float             x_frequency;
    t_float             x_q;
    t_bp_tilde_control  x_space;
    t_outlet            *x_outlet;
    } t_bp_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline double bp_tilde_coefficientsProceedQCosine (double f)
{
    if (f < -PD_HALF_PI || f > PD_HALF_PI) { return 0.0; }
    else {
        return cos (f);
    }
}

static void bp_tilde_coefficientsProceed (t_bp_tilde *x, t_float f, t_float q)
{
    x->x_frequency  = (f < 0.001) ? (t_float)10.0 : f;
    x->x_q          = (t_float)PD_MAX (0.0, q);
    x->x_sampleRate = (x->x_sampleRate <= 0) ? AUDIO_DEFAULT_SAMPLERATE : x->x_sampleRate;
    
    {
        double omega      = x->x_frequency * PD_TWO_PI / x->x_sampleRate;
        double omegaPerQ  = PD_MIN ((x->x_q < 0.001) ? 1.0 : (omega / x->x_q), 1.0);
        double r          = 1.0 - omegaPerQ;
        
        x->x_space.c_a1   = (t_sample)(2.0 * bp_tilde_coefficientsProceedQCosine (omega) * r);
        x->x_space.c_a2   = (t_sample)(- r * r);
        x->x_space.c_gain = (t_sample)(2.0 * omegaPerQ * (omegaPerQ + r * omega));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bp_tilde_frequency (t_bp_tilde *x, t_float f)
{
    bp_tilde_coefficientsProceed (x, f, x->x_q);
}

static void bp_tilde_q (t_bp_tilde *x, t_float q)
{
    bp_tilde_coefficientsProceed (x, x->x_frequency, q);
}

static void bp_tilde_clear (t_bp_tilde *x)
{
    x->x_space.c_real1 = (t_sample)0.0;
    x->x_space.c_real2 = (t_sample)0.0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *bp_tilde_perform (t_int *w)
{
    t_bp_tilde_control *c = (t_bp_tilde_control *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);

    t_sample last1  = c->c_real1;
    t_sample last2  = c->c_real2;
    t_sample a1     = c->c_a1;
    t_sample a2     = c->c_a2;
    t_sample gain   = c->c_gain;
    
    while (n--) {
        t_sample f = (*in++) + a1 * last1 + a2 * last2;
        *out++ = gain * f; 
        last2  = last1;
        last1  = f;
    }
    
    if (PD_IS_BIG_OR_SMALL (last1)) { last1 = (t_sample)0.0; }
    if (PD_IS_BIG_OR_SMALL (last2)) { last2 = (t_sample)0.0; }
    
    c->c_real1 = last1;
    c->c_real2 = last2;
    
    return (w + 5);
}

static void bp_tilde_dsp (t_bp_tilde *x, t_signal **sp)
{
    x->x_sampleRate = sp[0]->s_sampleRate;
    
    bp_tilde_coefficientsProceed (x, x->x_frequency, x->x_q);
    
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (bp_tilde_perform, 4, &x->x_space, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *bp_tilde_new (t_float f, t_float q)
{
    t_bp_tilde *x = (t_bp_tilde *)pd_new (bp_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet3);
    
    bp_tilde_coefficientsProceed (x, f, q);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void bp_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_bp__tilde__,
            (t_newmethod)bp_tilde_new,
            NULL,
            sizeof (t_bp_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_bp_tilde, x_f);
    
    class_addDSP (c, (t_method)bp_tilde_dsp);
    
    class_addMethod (c, (t_method)bp_tilde_frequency,   sym_inlet2, A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bp_tilde_q,           sym_inlet3, A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bp_tilde_clear,       sym_clear,  A_NULL);
    
    bp_tilde_class = c;
}

void bp_tilde_destroy (void)
{
    CLASS_FREE (bp_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
