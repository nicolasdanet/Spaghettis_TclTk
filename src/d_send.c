
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
#include "d_global.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *send_tilde_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
/*
static t_int *send_tilde_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
    {
        *out = (PD_BIG_OR_SMALL(*in) ? 0 : *in);
        out++;
        in++;
    }
    return (w+4);
}
*/

static void send_tilde_dsp (t_send_tilde *x, t_signal **sp)
{
    if (x->x_vectorSize != sp[0]->s_vectorSize) { error_mismatch (sym_send__tilde__, sym_size); }
    else {
        dsp_addCopyPerform (sp[0]->s_vector, x->x_vector, sp[0]->s_vectorSize);
        //dsp_add (send_tilde_perform, 3, sp[0]->s_vector, x->x_vector, sp[0]->s_vectorSize);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *send_tilde_new (t_symbol *s)
{
    t_send_tilde *x = (t_send_tilde *)pd_new (send_tilde_class);

    x->x_f          = 0.0;
    x->x_vectorSize = DSP_SEND_SIZE;
    x->x_vector     = (t_sample *)PD_MEMORY_GET (x->x_vectorSize * sizeof (t_sample));
    x->x_name       = s;
    
    pd_bind (cast_pd (x), s);
        
    return x;
}

static void send_tilde_free (t_send_tilde *x)
{
    pd_unbind (cast_pd (x), x->x_name);
    
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void send_tilde_setup(void)
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
    
    class_addDSP (c, send_tilde_dsp);
        
    send_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
