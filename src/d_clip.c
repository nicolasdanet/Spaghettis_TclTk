
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

static t_class *clip_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _clip_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_float     x_low;
    t_float     x_high;
    t_outlet    *x_outlet;
    } t_clip_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *clip_tilde_perform (t_int *w)
{
    t_clip_tilde *x = (t_clip_tilde *)(w[1]);
    PD_RESTRICTED in  = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    t_sample f1 = PD_MIN (x->x_low, x->x_high);
    t_sample f2 = PD_MAX (x->x_low, x->x_high);
    
    while (n--) {
    //
    t_sample f = *in++;
    
    *out++ = PD_CLAMP (f, f1, f2);
    //
    }
    
    return (w + 5);
}

static void clip_tilde_dsp (t_clip_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (clip_tilde_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *clip_tilde_new (t_float low, t_float high)
{
    t_clip_tilde *x = (t_clip_tilde *)pd_new (clip_tilde_class);
    
    x->x_low    = low;
    x->x_high   = high;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newFloat (cast_object (x), &x->x_low);
    inlet_newFloat (cast_object (x), &x->x_high);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void clip_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_clip__tilde__,
            (t_newmethod)clip_tilde_new,
            NULL,
            sizeof (t_clip_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_DEFFLOAT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_clip_tilde, x_f);
    
    class_addDSP (c, (t_method)clip_tilde_dsp);
    
    class_setHelpName (c, sym_math__tilde__);
    
    clip_tilde_class = c;
}

void clip_tilde_destroy (void)
{
    class_free (clip_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
