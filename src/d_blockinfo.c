
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *blockinfo_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _blockinfo_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_frequency;
    t_float     x_period;
    t_float     x_blockSize;
    t_float     x_sampleRate;
    t_float     x_overlap;
    t_block     *x_block;
    t_glist     *x_owner;
    t_outlet    *x_outlet[6];
    } t_blockinfo_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_int *blockinfo_tilde_perform (t_int *w)
{
    t_blockinfo_tilde *x = (t_blockinfo_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_float f = (t_float)(x->x_block ? block_getCount (x->x_block) : 0);
    
    while (n--) { *out++ = f; }
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void blockinfo_tilde_dsp (t_blockinfo_tilde *x, t_signal **sp)
{
    x->x_block      = canvas_getBlock (x->x_owner);
    x->x_frequency  = (t_float)(x->x_block ? block_getFrequency (x->x_block) : 1);
    x->x_period     = (t_float)(x->x_block ? block_getPeriod (x->x_block) : 1);
    x->x_blockSize  = (t_float)sp[0]->s_vectorSize;
    x->x_sampleRate = (t_float)sp[0]->s_sampleRate;
    x->x_overlap    = (t_float)sp[0]->s_overlap;
    
    dsp_add (blockinfo_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    
    dsp_addScalarPerform (&x->x_frequency,  sp[1]->s_vector, sp[1]->s_vectorSize);
    dsp_addScalarPerform (&x->x_period,     sp[2]->s_vector, sp[2]->s_vectorSize);
    dsp_addScalarPerform (&x->x_blockSize,  sp[3]->s_vector, sp[3]->s_vectorSize);
    dsp_addScalarPerform (&x->x_sampleRate, sp[4]->s_vector, sp[4]->s_vectorSize);
    dsp_addScalarPerform (&x->x_overlap,    sp[5]->s_vector, sp[5]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *blockinfo_tilde_new (void)
{
    t_blockinfo_tilde *x = (t_blockinfo_tilde *)pd_new (blockinfo_tilde_class);
    
    x->x_owner     = instance_contextGetCurrent();
    x->x_outlet[0] = outlet_new (cast_object (x), &s_signal);
    x->x_outlet[1] = outlet_new (cast_object (x), &s_signal);
    x->x_outlet[2] = outlet_new (cast_object (x), &s_signal);
    x->x_outlet[3] = outlet_new (cast_object (x), &s_signal);
    x->x_outlet[4] = outlet_new (cast_object (x), &s_signal);
    x->x_outlet[5] = outlet_new (cast_object (x), &s_signal);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void blockinfo_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_blockinfo__tilde__,
            (t_newmethod)blockinfo_tilde_new,
            NULL,
            sizeof (t_blockinfo_tilde),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
            
    class_addDSP (c, (t_method)blockinfo_tilde_dsp);
    
    blockinfo_tilde_class = c;
}

void blockinfo_tilde_destroy (void)
{
    class_free (blockinfo_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
