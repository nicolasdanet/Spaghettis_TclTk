
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *clip_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _clip_tilde {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f;
    t_float     x_low;
    t_float     x_high;
    t_outlet    *x_outlet;
    } t_clip_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *clip_perform (t_int *w)
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
    
    dsp_add (clip_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *clip_tilde_new (t_float low, t_float high)
{
    t_clip_tilde *x = (t_clip_tilde *)pd_new (clip_class);
    
    x->x_low    = low;
    x->x_high   = high;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_newFloat (cast_object (x), &x->x_low);
    inlet_newFloat (cast_object (x), &x->x_high);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    clip_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
