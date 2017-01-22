
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_tab.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabosc4_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabosc4_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    double      x_phase;
    t_float     x_conversion;
    t_float     x_size;
    t_float     x_sizeInverse;
    t_word      *x_vector;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabosc4_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabosc4_tilde_set (t_tabosc4_tilde *x, t_symbol *s)
{
    int n, size;
    
    tab_fetchArray ((x->x_name = s), &n, &x->x_vector, sym_tabosc4__tilde__);

    size = n - 3;   /* Size of the usable size of the table. */
    
    if (size > 0 && PD_IS_POWER_2 (size)) { 
        x->x_size = size;
        x->x_sizeInverse = 1.0 / x->x_size;
        
    } else {
        x->x_vector = NULL; if (s != &s_) { error_invalid (sym_tabosc4__tilde__, sym_array); }
    }
}

static void tabosc4_tilde_phase (t_tabosc4_tilde *x, t_float f)
{
    x->x_phase = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *tabosc4_tilde_perform (t_int *w)
{
    t_tabosc4_tilde *x = (t_tabosc4_tilde *)(w[1]);
    PD_RESTRICTED in   = (t_sample *)(w[2]);
    PD_RESTRICTED out  = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    t_word *data = x->x_vector;
    
    if (data) {
    //
    t_float size = x->x_size;
    int sizeMask = size - 1;
    t_float conversion = size * x->x_conversion;
    
    t_rawcast64 z;
    t_word *p = NULL;
    int t;
    
    double phase = (size * x->x_phase) + DSP_UNITBIT;
    
    while (n--) {
    //
    z.z_d = phase;
    phase += (*in++) * conversion;
    p = data + (z.z_i[PD_RAWCAST64_MSB] & sizeMask);    /* Integer part. */
    z.z_i[PD_RAWCAST64_MSB] = DSP_UNITBIT_MSB;
    *out++ = dsp_4PointsInterpolationWithWords ((t_float)(z.z_d - DSP_UNITBIT), p);     /* Fractional part. */
    //
    }

    /* Wrap the phase (keep only the fractional part). */
    /* Size must be a power of two. */
    
    z.z_d = DSP_UNITBIT * size; t = z.z_i[PD_RAWCAST64_MSB];
    z.z_d = phase + (DSP_UNITBIT * size - DSP_UNITBIT);
    z.z_i[PD_RAWCAST64_MSB] = t;
    x->x_phase = (z.z_d - DSP_UNITBIT * size) * x->x_sizeInverse;
    //
    } else { while (n--) { *out++ = 0.0; } }
    
    return (w + 5);
}

static void tabosc4_tilde_dsp (t_tabosc4_tilde *x, t_signal **sp)
{
    x->x_conversion = 1.0 / sp[0]->s_sampleRate;
    
    tabosc4_tilde_set (x, x->x_name);

    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (tabosc4_tilde_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tabosc4_tilde_new (t_symbol *s)
{
    t_tabosc4_tilde *x = (t_tabosc4_tilde *)pd_new (tabosc4_tilde_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tabosc4_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabosc4__tilde__,
            (t_newmethod)tabosc4_tilde_new,
            NULL,
            sizeof (t_tabosc4_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    CLASS_SIGNAL (c, t_tabosc4_tilde, x_f);
    
    class_addDSP (c, (t_method)tabosc4_tilde_dsp);
    
    class_addMethod (c, (t_method)tabosc4_tilde_set,    sym_set,    A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)tabosc4_tilde_phase,  sym_inlet2, A_FLOAT, A_NULL);
    
    tabosc4_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
