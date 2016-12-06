
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *wrap_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _wrap_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_wrap_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *wrap_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) {
        t_sample f = *in++;
        int k = (int)f;
        if (f > 0.0) { *out++ = f - k; } else { *out++ = f - (k - 1); }
    }
    
    return (w + 4);
}

static void wrap_tilde_dsp (t_wrap_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (wrap_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *wrap_tilde_new (void)
{
    t_wrap_tilde *x = (t_wrap_tilde *)pd_new (wrap_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void wrap_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_wrap__tilde__,
            (t_newmethod)wrap_tilde_new,
            NULL,
            sizeof (t_wrap_tilde),
            CLASS_DEFAULT,
            A_NULL);
        
    CLASS_SIGNAL (c, t_wrap_tilde, x_f);
    
    class_addDSP (c, wrap_tilde_dsp);
    
    wrap_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
