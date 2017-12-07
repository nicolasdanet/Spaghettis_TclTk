
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *dbtorms_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct dbtorms_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_dbtorms_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *dbtorms_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *out++ = math_decibelToRootMeanSquare (*in++); }
    
    return (w + 4);
}

static void dbtorms_tilde_dsp (t_dbtorms_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (dbtorms_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *dbtorms_tilde_new (void)
{
    t_dbtorms_tilde *x = (t_dbtorms_tilde *)pd_new (dbtorms_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dbtorms_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_dbtorms__tilde__,
            (t_newmethod)dbtorms_tilde_new,
            NULL,
            sizeof (t_dbtorms_tilde),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addCreator ((t_newmethod)dbtorms_tilde_new, sym_dbtoamp__tilde__, A_NULL);
    
    CLASS_SIGNAL (c, t_dbtorms_tilde, x_f);
    
    class_addDSP (c, (t_method)dbtorms_tilde_dsp);
    
    class_setHelpName (c, sym_acoustic__tilde__);
    
    dbtorms_tilde_class = c;
}

void dbtorms_tilde_destroy (void)
{
    class_free (dbtorms_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

