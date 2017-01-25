
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
#include "d_delay.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *delwrite_tilde_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *delread_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _delread_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_samplesPerMilliseconds;
    t_float     x_delayInMilliseconds;
    int         x_delayInSamples;
    int         x_vectorSize;
    int         x_masterVectorSize;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_delread_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void delread_tilde_setDelayInSamples (t_delread_tilde *x)
{
    t_delwrite_tilde *m = (t_delwrite_tilde *)pd_getThingByClass (x->x_name, delwrite_tilde_class);
    
    if (m) {
    //
    int d = (int)((x->x_delayInMilliseconds * x->x_samplesPerMilliseconds) + 0.5);
    
    /* Note that the master vector size is reported as zero in non-recirculating cases. */
    
    x->x_delayInSamples = d + (x->x_vectorSize - x->x_masterVectorSize);
    x->x_delayInSamples = PD_CLAMP (x->x_delayInSamples, x->x_vectorSize, m->dw_space.c_size);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void delread_tilde_float (t_delread_tilde *x, t_float f)
{
    x->x_delayInMilliseconds = f; if (dsp_isRunning()) { delread_tilde_setDelayInSamples (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *delread_tilde_perform (t_int *w)
{
    t_delwrite_tilde_control *c = (t_delwrite_tilde_control *)(w[1]);
    int delayInSamples = *(int *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    int phase = c->c_phase - delayInSamples;
    
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
    
    return (w + 5);
}

static void delread_tilde_dsp (t_delread_tilde *x, t_signal **sp)
{
    t_delwrite_tilde *m = (t_delwrite_tilde *)pd_getThingByClass (x->x_name, delwrite_tilde_class);
    
    x->x_vectorSize = sp[0]->s_vectorSize;
    x->x_samplesPerMilliseconds = (t_float)(sp[0]->s_sampleRate * 0.001);
    
    if (!m) { if (x->x_name != &s_) { error_canNotFind (sym_delread__tilde__, x->x_name); } }
    else {
    //
    delwrite_tilde_setMasterVectorSize (m, sp[0]->s_vectorSize);
    delwrite_tilde_updateDelayLine (m, sp[0]->s_sampleRate);
        
    /* Set master vector size as zero in non-recirculating cases. */
        
    x->x_masterVectorSize = (m->dw_buildIdentifier == ugen_getBuildIdentifier() ? 0 : m->dw_masterVectorSize);
        
    delread_tilde_setDelayInSamples (x);
        
    dsp_add (delread_tilde_perform, 4,
        &m->dw_space,
        &x->x_delayInSamples,
        sp[0]->s_vector,
        sp[0]->s_vectorSize);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *delread_tilde_new (t_symbol *s, t_float f)
{
    t_delread_tilde *x = (t_delread_tilde *)pd_new (delread_tilde_class);
    
    x->x_delayInMilliseconds = f;
    x->x_name                = s;
    x->x_outlet              = outlet_new (cast_object (x), &s_signal);
        
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

