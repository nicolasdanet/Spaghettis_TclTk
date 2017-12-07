
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"
#include "d_global.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *send_tilde_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void send_tilde_dsp (t_send_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != DSP_SEND_SIZE) { error_mismatch (sym_send__tilde__, sym_size); }
    else {
        dsp_addCopyPerform (sp[0]->s_vector, x->x_vector, DSP_SEND_SIZE);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *send_tilde_new (t_symbol *s)
{
    t_send_tilde *x = (t_send_tilde *)pd_new (send_tilde_class);

    x->x_f      = (t_float)0.0;
    x->x_vector = (t_sample *)PD_MEMORY_GET (DSP_SEND_SIZE * sizeof (t_sample));
    x->x_name   = s;
    
    if (x->x_name != &s_) { pd_bind (cast_pd (x), s); }
        
    return x;
}

static void send_tilde_free (t_send_tilde *x)
{
    if (x->x_name != &s_) { pd_unbind (cast_pd (x), x->x_name); }
    
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void send_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_send__tilde__,
            (t_newmethod)send_tilde_new,
            (t_method)send_tilde_free,
            sizeof (t_send_tilde),
            CLASS_DEFAULT, 
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)send_tilde_new, sym_s__tilde__, A_DEFSYMBOL, A_NULL);
    
    CLASS_SIGNAL (c, t_send_tilde, x_f);
    
    class_addDSP (c, (t_method)send_tilde_dsp);
        
    send_tilde_class = c;
}

void send_tilde_destroy (void)
{
    class_free (send_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
