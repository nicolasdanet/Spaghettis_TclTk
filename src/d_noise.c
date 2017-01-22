
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

static t_class *noise_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _noise_tilde {
    t_object    x_obj;                  /* Must be the first. */
    int         x_state;
    t_outlet    *x_outlet;
    } t_noise_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *noise_tilde_perform (t_int *w)                    /* Weird LCG kept for compatbility. */
{
    int *p = (int *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    int state = *p;
    
    while (n--) {
    //
    *out++ = (t_sample)(((float)((state & PD_INT_MAX) - 0x40000000)) * ((float)(1.0 / 0x40000000)));
    state  = state * 435898247 + 382842987;
    //
    }
    
    *p = state;
    
    return (w + 4);
}

static void noise_tilde_dsp (t_noise_tilde *x, t_signal **sp)
{
    dsp_add (noise_tilde_perform, 3, &x->x_state, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *noise_tilde_new (void)
{
    static int seed = 307;
    
    t_noise_tilde *x = (t_noise_tilde *)pd_new (noise_tilde_class);
    
    seed *= 1319;
    
    x->x_state  = seed;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void noise_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_noise__tilde__,
            (t_newmethod)noise_tilde_new,
            NULL,
            sizeof (t_noise_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addDSP (c, (t_method)noise_tilde_dsp);
    
    noise_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
