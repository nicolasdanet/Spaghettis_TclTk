
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"
#include "d_delay.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *vd_tilde_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vd_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_float     x_samplesPerMilliseconds;
    t_float     x_sampleRate;
    int         x_vectorSize;
    int         x_offsetSize;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_vd_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *vd_tilde_perform (t_int *w)
{
    t_vd_tilde *x = (t_vd_tilde *)(w[1]);
    t_delwrite_tilde_control *c = (t_delwrite_tilde_control *)(w[2]);
    PD_RESTRICTED in  = (t_sample *)(w[3]);
    PD_RESTRICTED out = (t_sample *)(w[4]);
    int n = (int)(w[5]);

    int mismatch = (x->x_vectorSize != c->c_vectorSize) || (x->x_sampleRate != c->c_sampleRate);
    
    if (mismatch) { while (n--) { *out++ = (t_sample)0.0; } }
    else {
    //
    t_float limit = c->c_size - n;
    
    while (n--) {
    //
    t_float delayInSamples, f = (t_float)(*in++);
    
    /* Note that the offset size is reported as zero in non-recirculating cases. */
    
    delayInSamples = (x->x_samplesPerMilliseconds * f) - x->x_offsetSize;
    delayInSamples = (t_float)(PD_CLAMP (delayInSamples, 1.0, limit) + n);
    
    {
        int integer = (int)delayInSamples;
        t_float fractional = delayInSamples - (t_float)integer;
        PD_RESTRICTED p = c->c_vector + (c->c_phase - integer);
        
        if (p < c->c_vector + DELAY_EXTRA_SAMPLES) { p += c->c_size; }
        
        *out++ = (t_sample)dsp_4PointsInterpolationWithFloats (fractional, p[0], p[-1], p[-2], p[-3]);
    }
    //
    }
    //
    }
    
    return (w + 6);
}

static void vd_tilde_dsp (t_vd_tilde *x, t_signal **sp)
{
    t_delwrite_tilde *m = (t_delwrite_tilde *)symbol_getThingByClass (x->x_name, delwrite_tilde_class);
    
    x->x_samplesPerMilliseconds = (t_float)(sp[0]->s_sampleRate * 0.001);
    x->x_sampleRate = sp[0]->s_sampleRate;
    x->x_vectorSize = sp[0]->s_vectorSize;
    
    if (!m) { if (x->x_name != &s_) { error_canNotFind (sym_vd__tilde__, x->x_name); } }
    else {
    //
    int buildIdentifier = instance_getDspChainIdentifier();
    
    /* Set the offset size as zero in non-recirculating cases. */
    
    x->x_offsetSize = (m->dw_buildIdentifier == buildIdentifier ? 0 : sp[0]->s_vectorSize);
    
    dsp_add (vd_tilde_perform, 5, x, &m->dw_space, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *vd_tilde_new (t_symbol *s)
{
    t_vd_tilde *x = (t_vd_tilde *)pd_new (vd_tilde_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void vd_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_vd__tilde__,
            (t_newmethod)vd_tilde_new,
            NULL,
            sizeof (t_vd_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
    
    class_addCreator ((t_newmethod)vd_tilde_new, sym_delread4__tilde__, A_DEFSYMBOL, A_NULL);
    
    CLASS_SIGNAL (c, t_vd_tilde, x_f);
    
    class_addDSP (c, (t_method)vd_tilde_dsp);
    
    vd_tilde_class = c;
}

void vd_tilde_destroy (void)
{
    class_free (vd_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

