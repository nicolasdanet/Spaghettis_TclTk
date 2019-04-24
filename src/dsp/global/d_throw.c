
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "d_global.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *throw_tilde_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _throw_tilde {
    t_object            x_obj;              /* Must be the first. */
    t_pointerAtomic     x_p;
    t_symbol            *x_name;
    } t_throw_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void throw_tilde_set (t_throw_tilde *x, t_symbol *s)
{
    t_catch_tilde *catcher = (t_catch_tilde *)symbol_getThingByClass ((x->x_name = s), catch_tilde_class);
    t_sample *t = catcher ? catcher->x_vector : NULL;
    
    PD_ATOMIC_POINTER_WRITE (t, &x->x_p);
    
    if (!t && x->x_name != &s_) { error_canNotFind (sym_throw__tilde__, x->x_name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing.*/

static t_int *throw_tilde_perform (t_int *w)
{
    t_throw_tilde *x  = (t_throw_tilde *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)PD_ATOMIC_POINTER_READ (&x->x_p);
    
    if (out) { int i; for (i = 0; i < INTERNAL_BLOCKSIZE; i++) { *out += *in; out++; in++; } }
    
    return (w + 3);
}

static void throw_tilde_dsp (t_throw_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != INTERNAL_BLOCKSIZE) { error_mismatch (sym_throw__tilde__, sym_size); }
    else {
        throw_tilde_set (x, x->x_name);
        PD_ASSERT (sp[0]->s_vector != (t_sample *)PD_ATOMIC_POINTER_READ (&x->x_p));
        dsp_add (throw_tilde_perform, 2, x, sp[0]->s_vector);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *throw_tilde_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_throw_tilde *x = (t_throw_tilde *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym_set);
    buffer_appendSymbol (b, x->x_name);
    buffer_appendComma (b);
    object_getSignalValues (cast_object (x), b);
    
    return b;
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *throw_tilde_new (t_symbol *s)
{
    t_throw_tilde *x = (t_throw_tilde *)pd_new (throw_tilde_class);
    
    PD_ATOMIC_POINTER_WRITE (NULL, &x->x_p);

    x->x_name = s;

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
            CLASS_DEFAULT | CLASS_SIGNAL,
            A_DEFSYMBOL,
            A_NULL);
    
    class_addDSP (c, (t_method)throw_tilde_dsp);
    
    class_addMethod (c, (t_method)throw_tilde_set, sym_set, A_SYMBOL, A_NULL);
    
    class_setDataFunction (c, throw_tilde_functionData);
    
    throw_tilde_class = c;
}

void throw_tilde_destroy (void)
{
    class_free (throw_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
