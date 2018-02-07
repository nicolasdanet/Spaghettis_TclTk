
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

static t_class *throw_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _throw_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_sample    *x_vector;
    t_symbol    *x_name;
    } t_throw_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void throw_tilde_set (t_throw_tilde *x, t_symbol *s)
{
    t_catch_tilde *catcher = (t_catch_tilde *)symbol_getThingByClass ((x->x_name = s), catch_tilde_class);
    
    x->x_vector = NULL;
    
    if (catcher) { x->x_vector = catcher->x_vector; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing.*/

static t_int *throw_tilde_perform (t_int *w)
{
    t_throw_tilde *x  = (t_throw_tilde *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = x->x_vector;
    
    if (out) { int i; for (i = 0; i < INTERNAL_BLOCKSIZE; i++) { *out += *in; out++; in++; } }
    
    return (w + 3);
}

static void throw_tilde_dsp (t_throw_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != INTERNAL_BLOCKSIZE) { error_mismatch (sym_throw__tilde__, sym_size); }
    else {
        throw_tilde_set (x, x->x_name);
        if (!x->x_vector && x->x_name != &s_) { error_canNotFind (sym_throw__tilde__, x->x_name); }
        PD_ASSERT (sp[0]->s_vector != x->x_vector);
        dsp_add (throw_tilde_perform, 2, x, sp[0]->s_vector);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *throw_tilde_new (t_symbol *s)
{
    t_throw_tilde *x = (t_throw_tilde *)pd_new (throw_tilde_class);
    
    x->x_f      = (t_float)0.0;
    x->x_vector = NULL;
    x->x_name   = s;

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    class_addDSP (c, (t_method)throw_tilde_dsp);
    
    class_addMethod (c, (t_method)throw_tilde_set, sym_set, A_SYMBOL, A_NULL);
        
    throw_tilde_class = c;
}

void throw_tilde_destroy (void)
{
    class_free (throw_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
