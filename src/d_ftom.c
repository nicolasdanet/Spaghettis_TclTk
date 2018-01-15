
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *ftom_tilde_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct ftom_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_ftom_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *ftom_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *out++ = math_frequencyToMidi (*in++); }
    
    return (w + 4);
}

static void ftom_tilde_dsp (t_ftom_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (ftom_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *ftom_tilde_new (void)
{
    t_ftom_tilde *x = (t_ftom_tilde *)pd_new (ftom_tilde_class);

    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void ftom_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_ftom__tilde__,
            (t_newmethod)ftom_tilde_new,
            NULL,
            sizeof (t_ftom_tilde),
            CLASS_DEFAULT,
            A_NULL);
        
    CLASS_SIGNAL (c, t_ftom_tilde, x_f);
    
    class_addDSP (c, (t_method)ftom_tilde_dsp);
    
    class_setHelpName (c, sym_acoustic__tilde__);
    
    ftom_tilde_class = c;
}

void ftom_tilde_destroy (void)
{
    class_free (ftom_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
