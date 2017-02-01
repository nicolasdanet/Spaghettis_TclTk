
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
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *phasor_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _phasor_tilde {
    t_object    x_obj;                      /* Must be the first. */
    double      x_phase;
    t_float     x_conversion;
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_phasor_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void phasor_tilde_phase (t_phasor_tilde *x, t_float f)
{
    x->x_phase = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *phasor_tilde_perform (t_int *w)
{
    t_phasor_tilde *x = (t_phasor_tilde *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    double phase = x->x_phase + DSP_UNITBIT;
    t_float k = x->x_conversion;
    t_rawcast64 z;
    
    z.z_d = phase;

    while (n--) {
    //
    z.z_i[PD_RAWCAST64_MSB] = DSP_UNITBIT_MSB;      /* Wrap the phase (keep only the fractional part). */
    phase += (*in++) * k;
    *out++ = (t_sample)(z.z_d - DSP_UNITBIT);
    z.z_d = phase;
    //
    }
    
    z.z_i[PD_RAWCAST64_MSB] = DSP_UNITBIT_MSB;      /* Ditto. */
    
    x->x_phase = z.z_d - DSP_UNITBIT;
    
    return (w + 5);
}

static void phasor_tilde_dsp (t_phasor_tilde *x, t_signal **sp)
{
    x->x_conversion = (t_float)(1.0 / sp[0]->s_sampleRate);
    
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (phasor_tilde_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *phasor_tilde_new (t_float f)
{
    t_phasor_tilde *x = (t_phasor_tilde *)pd_new (phasor_tilde_class);
    
    x->x_f = f;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);
        
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void phasor_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_phasor__tilde__,
            (t_newmethod)phasor_tilde_new,
            NULL,
            sizeof (t_phasor_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_phasor_tilde, x_f);
    
    class_addDSP (c, (t_method)phasor_tilde_dsp);
    
    class_addMethod (c, (t_method)phasor_tilde_phase, sym_inlet2, A_FLOAT, A_NULL);
    
    phasor_tilde_class = c;
}

void phasor_tilde_destroy (void)
{
    CLASS_FREE (phasor_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
