
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *osc_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _osc_tilde {
    t_object    x_obj;                  /* Must be the first. */
    double      x_phase;
    t_float     x_conversion;
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_osc_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void osc_tilde_phase (t_osc_tilde *x, t_float f)
{
    x->x_phase = (double)COSINE_TABLE_SIZE * f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *osc_tilde_perform (t_int *w)
{
    t_osc_tilde *x = (t_osc_tilde *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    double phase = x->x_phase;
    t_float k = x->x_conversion;
    t_rawcast64 z;
    
    while (n--) { *out++ = dsp_getCosineAt (phase); phase += (*in++) * k; }

    /* Wrap the phase to the cosine table size. */
    
    z.z_d = phase + COSINE_UNITBIT;
    z.z_i[PD_RAWCAST64_MSB] = COSINE_UNITBIT_MSB;
    x->x_phase = z.z_d - COSINE_UNITBIT;
    
    return (w + 5);
}

static void osc_tilde_dsp (t_osc_tilde *x, t_signal **sp)
{
    x->x_conversion = COSINE_TABLE_SIZE / sp[0]->s_sampleRate;
    
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (osc_tilde_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *osc_tilde_new (t_float f)
{
    t_osc_tilde *x = (t_osc_tilde *)pd_new (osc_tilde_class);
    
    x->x_f = f;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_new2 (x, &s_float);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void osc_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_osc__tilde__,
            (t_newmethod)osc_tilde_new,
            NULL,
            sizeof (t_osc_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_osc_tilde, x_f);
    
    class_addDSP (c, (t_method)osc_tilde_dsp);
    
    class_addMethod (c, (t_method)osc_tilde_phase, sym__inlet2, A_FLOAT, A_NULL);

    osc_tilde_class = c;
}

void osc_tilde_destroy (void)
{
    class_free (osc_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
