
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"
#include "d_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sqrt_tilde_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct sqrt_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_sqrt_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *sqrt_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *out++ = (t_sample)sqrt_fast ((t_float)(*in++)); }
    
    return (w + 4);
}

static void sqrt_tilde_dsp (t_sqrt_tilde *x, t_signal **sp)
{
    dsp_add (sqrt_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *sqrt_tilde_new (void)
{
    t_sqrt_tilde *x = (t_sqrt_tilde *)pd_new (sqrt_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void sqrt_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_sqrt__tilde__,
            (t_newmethod)sqrt_tilde_new,
            NULL,
            sizeof (t_sqrt_tilde),
            CLASS_DEFAULT,
            A_NULL);
    
    CLASS_SIGNAL (c, t_sqrt_tilde, x_f);
    
    class_addDSP (c, (t_method)sqrt_tilde_dsp);
    
    #if PD_WITH_LEGACY
    
    class_addCreator ((t_newmethod)sqrt_tilde_new, sym_q8_sqrt__tilde__, A_NULL);
    
    #endif
    
    class_setHelpName (c, sym_math__tilde__);
    
    sqrt_tilde_class = c;
}

void sqrt_tilde_destroy (void)
{
    class_free (sqrt_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
