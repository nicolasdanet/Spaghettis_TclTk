
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

static t_class *delread_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _delread_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_delayInMilliseconds;
    t_float     x_samplesPerMilliseconds;
    t_float     x_sampleRate;
    int         x_delayInSamples;
    int         x_vectorSize;
    int         x_offsetSize;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_delread_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void delread_tilde_setDelayInSamples (t_delread_tilde *x)
{
    t_delwrite_tilde *m = (t_delwrite_tilde *)symbol_getThingByClass (x->x_name, delwrite_tilde_class);
    
    if (m) {
    //
    int d = (int)((x->x_delayInMilliseconds * x->x_samplesPerMilliseconds) + 0.5);
    
    /* Note that the offset size is reported as zero in non-recirculating cases. */
    
    x->x_delayInSamples = d + (x->x_vectorSize - x->x_offsetSize);
    x->x_delayInSamples = PD_CLAMP (x->x_delayInSamples, x->x_vectorSize, m->dw_space.c_size);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void delread_tilde_float (t_delread_tilde *x, t_float f)
{
    x->x_delayInMilliseconds = f; if (dsp_getState()) { delread_tilde_setDelayInSamples (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *delread_tilde_perform (t_int *w)
{
    t_delread_tilde *x = (t_delread_tilde *)(w[1]);
    t_delwrite_tilde_control *c = (t_delwrite_tilde_control *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    int mismatch = (x->x_vectorSize != c->c_vectorSize) || (x->x_sampleRate != c->c_sampleRate);
    
    if (mismatch) { while (n--) { *out++ = (t_sample)0.0; } }
    else {
    //
    int phase = c->c_phase - x->x_delayInSamples;
    
    if (phase < 0) { phase += c->c_size; }
    
    {
        PD_RESTRICTED p = c->c_vector + phase;

        while (n--) {
            *out++ = *p++;
            if (p == c->c_vector + (c->c_size + DELAY_EXTRA_SAMPLES)) { 
                p -= c->c_size;
            }
        }
    }
    //
    }
    
    return (w + 5);
}

static void delread_tilde_dsp (t_delread_tilde *x, t_signal **sp)
{
    t_delwrite_tilde *m = (t_delwrite_tilde *)symbol_getThingByClass (x->x_name, delwrite_tilde_class);
    
    x->x_samplesPerMilliseconds = (t_float)(sp[0]->s_sampleRate * 0.001);
    x->x_sampleRate = sp[0]->s_sampleRate;
    x->x_vectorSize = sp[0]->s_vectorSize;
    
    if (!m) { if (x->x_name != &s_) { error_canNotFind (sym_delread__tilde__, x->x_name); } }
    else {
    //
    int buildIdentifier = instance_getDspChainIdentifier();
    
    /* Set the offset size as zero in non-recirculating cases. */
        
    x->x_offsetSize = (m->dw_buildIdentifier == buildIdentifier ? 0 : sp[0]->s_vectorSize);
        
    delread_tilde_setDelayInSamples (x);
        
    dsp_add (delread_tilde_perform, 4, x,
        &m->dw_space,
        sp[0]->s_vector,
        sp[0]->s_vectorSize);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *delread_tilde_new (t_symbol *s, t_float f)
{
    t_delread_tilde *x = (t_delread_tilde *)pd_new (delread_tilde_class);
    
    x->x_delayInMilliseconds = f;
    x->x_name                = s;
    x->x_outlet              = outlet_newSignal (cast_object (x));
        
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void delread_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_delread__tilde__,
            (t_newmethod)delread_tilde_new,
            NULL,
            sizeof (t_delread_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFFLOAT,
            A_NULL);
            
    class_addDSP (c, (t_method)delread_tilde_dsp);
    
    class_addFloat (c, (t_method)delread_tilde_float);
    
    delread_tilde_class = c;
}

void delread_tilde_destroy (void)
{
    class_free (delread_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

