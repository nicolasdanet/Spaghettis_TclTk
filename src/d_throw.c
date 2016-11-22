
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

extern t_class *catch_tilde_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *throw_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _throw_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    int         x_vectorSize;
    t_sample    *x_vector;
    t_symbol    *x_name;
    } t_throw_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void throw_tilde_set (t_throw_tilde *x, t_symbol *s)
{
    t_catch_tilde *catcher = (t_catch_tilde *)pd_getThingByClass ((x->x_name = s), catch_tilde_class);
    
    x->x_vector = NULL;
    
    if (!catcher) { error_canNotFind (sym_throw__tilde__, x->x_name); }
    else {
        if (catcher->x_vectorSize == x->x_vectorSize) { x->x_vector = catcher->x_vector; }
        else {
            error_mismatch (sym_throw__tilde__, sym_size);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *throw_tilde_perform (t_int *w)
{
    t_throw_tilde *x = (t_throw_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_sample *out = x->x_vector;
    
    if (out) { while (n--) { *out += *in; out++; in++; } }
    
    return (w + 4);
}

static void throw_tilde_dsp (t_throw_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != x->x_vectorSize) { error_mismatch (sym_throw__tilde__, sym_size); }
    else {
        throw_tilde_set (x, x->x_name);
        dsp_add (throw_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *throw_tilde_new (t_symbol *s)
{
    t_throw_tilde *x = (t_throw_tilde *)pd_new (throw_tilde_class);
    
    x->x_f           = 0.0;
    x->x_vectorSize  = DSP_SEND_SIZE;
    x->x_vector      = NULL;
    x->x_name        = s;

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void throw_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_throw__tilde__,
            (t_newmethod)throw_tilde_new,
            NULL,
            sizeof (t_throw_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
        
    CLASS_SIGNAL (c, t_throw_tilde, x_f);
    
    class_addDSP (c, throw_tilde_dsp);
    
    class_addMethod (c, (t_method)throw_tilde_set, sym_set, A_SYMBOL, A_NULL);
        
    throw_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
