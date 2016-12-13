
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"
#include "d_delay.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *delwrite_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void delwrite_tilde_setVectorSize (t_delwrite_tilde *x, int vectorSize)
{
    if (x->dw_buildIdentifierForVectorSize != ugen_getBuildIdentifier()) {
        x->dw_vectorSize = vectorSize;
        x->dw_buildIdentifierForVectorSize = ugen_getBuildIdentifier();
    }
}

void delwrite_tilde_updateBuffer (t_delwrite_tilde *x, t_float sampleRate)
{
    int n = MILLISECONDS_TO_SECONDS (x->dw_delay) * sampleRate;
    
    n = PD_MAX (1, n);
    n += ((- n) & (DELAY_ROUND_SAMPLES - 1));   /* Snap to the next multiple of DELAY_ROUND_SAMPLES. */
    n += DELAY_BLOCK_SIZE;
    
    if (x->dw_space.c_size != n) {
    //
    size_t oldSize = sizeof (t_sample) * (x->dw_space.c_size + DELAY_EXTRA_SAMPLES);
    size_t newSize = sizeof (t_sample) * (n + DELAY_EXTRA_SAMPLES);
    
    x->dw_space.c_vector = (t_sample *)PD_MEMORY_RESIZE (x->dw_space.c_vector, oldSize, newSize);
    x->dw_space.c_size   = n;
    x->dw_space.c_phase  = DELAY_EXTRA_SAMPLES;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *delwrite_tilde_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_delwrite_tilde_control *c = (t_delwrite_tilde_control *)(w[2]);
    int n = (int)(w[3]);
    int phase = c->c_phase, nsamps = c->c_size;
    t_sample *vp = c->c_vector, *bp = vp + phase, *ep = vp + (c->c_size + DELAY_EXTRA_SAMPLES);
    phase += n;

    while (n--)
    {
        t_sample f = *in++;
        if (PD_BIG_OR_SMALL(f))
            f = 0;
        *bp++ = f;
        if (bp == ep)
        {
            vp[0] = ep[-4];
            vp[1] = ep[-3];
            vp[2] = ep[-2];
            vp[3] = ep[-1];
            bp = vp + DELAY_EXTRA_SAMPLES;
            phase -= nsamps;
        }
    }
    c->c_phase = phase; 
    return (w+4);
}

static void delwrite_tilde_dsp (t_delwrite_tilde *x, t_signal **sp)
{
    x->dw_buildIdentifier = ugen_getBuildIdentifier();
    
    delwrite_tilde_setVectorSize (x, sp[0]->s_vectorSize);
    delwrite_tilde_updateBuffer (x, sp[0]->s_sampleRate);
    
    dsp_add (delwrite_tilde_perform, 3, sp[0]->s_vector, &x->dw_space, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *delwrite_tilde_new (t_symbol *s, t_float milliseconds)
{
    t_delwrite_tilde *x = (t_delwrite_tilde *)pd_new (delwrite_tilde_class);

    x->dw_delay             = milliseconds;
    x->dw_space.c_size      = 0;
    x->dw_space.c_vector    = PD_MEMORY_GET ((x->dw_space.c_size + DELAY_EXTRA_SAMPLES) * sizeof (t_sample));
    x->dw_name              = (s == &s_) ? sym_delwrite__tilde__ : s;
    
    pd_bind (cast_pd (x), x->dw_name);
    
    return x;
}


static void delwrite_tilde_free (t_delwrite_tilde *x)
{
    pd_unbind (cast_pd (x), x->dw_name);
    
    PD_MEMORY_FREE (x->dw_space.c_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void delwrite_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_delwrite__tilde__, 
            (t_newmethod)delwrite_tilde_new,
            (t_method)delwrite_tilde_free,
            sizeof (t_delwrite_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_delwrite_tilde, dw_f);
    
    class_addDSP (c, delwrite_tilde_dsp);
    
    delwrite_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

