
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_float         *cos_tilde_table;       /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *cos_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _cost_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_cos_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void cos_tilde_initialize (void)
{
    if (!cos_tilde_table) {
    //
    double phase = 0.0;
    double phaseIncrement = PD_TWO_PI / COSINE_TABLE_SIZE;
    int i;
    
    cos_tilde_table = (t_float *)PD_MEMORY_GET (sizeof (t_float) * (COSINE_TABLE_SIZE + 1));
    
    for (i = 0; i < COSINE_TABLE_SIZE + 1; i++) {
        cos_tilde_table[i] = (t_float)cos (phase);
        phase += phaseIncrement;
    }
    //
    }
}

void cos_tilde_release (void)
{
    if (cos_tilde_table) { PD_MEMORY_FREE (cos_tilde_table); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *cos_tilde_perform (t_int *w)
{
    PD_RESTRICTED in = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *out++ = dsp_getCosineAtLUT ((*in++) * COSINE_TABLE_SIZE); }

    return (w + 4);
}

static void cos_tilde_dsp (t_cos_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (cos_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *cos_tilde_new (void)
{
    t_cos_tilde *x = (t_cos_tilde *)pd_new (cos_tilde_class);
    
    x->x_outlet = outlet_newSignal (cast_object (x));

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void cos_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_cos__tilde__,
            (t_newmethod)cos_tilde_new,
            NULL,
            sizeof (t_cos_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_cos_tilde, x_f);
    
    class_addDSP (c, (t_method)cos_tilde_dsp);
    
    cos_tilde_class = c;
}

void cos_tilde_destroy (void)
{
    class_free (cos_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
