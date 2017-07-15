
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *mtof_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct mtof_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_mtof_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *mtof_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *out++ = math_midiToFrequency (*in++); }
    
    return (w + 4);
}

static void mtof_tilde_dsp (t_mtof_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (mtof_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *mtof_tilde_new (void)
{
    t_mtof_tilde *x = (t_mtof_tilde *)pd_new (mtof_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void mtof_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mtof__tilde__,
            (t_newmethod)mtof_tilde_new,
            NULL,
            sizeof (t_mtof_tilde),
            CLASS_DEFAULT,
            A_NULL);
        
    CLASS_SIGNAL (c, t_mtof_tilde, x_f);
    
    class_addDSP (c, (t_method)mtof_tilde_dsp);
    
    class_setHelpName (c, sym_acoustic__tilde__);
    
    mtof_tilde_class = c;
}

void mtof_tilde_destroy (void)
{
    class_free (mtof_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
