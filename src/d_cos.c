
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
#include "d_osc.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_float         *cos_tilde_table;       /* Shared. */

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
#pragma mark -

void cos_tilde_initialize (void)
{
    if (!cos_tilde_table) {
    //
    t_float phase = 0.0;
    t_float phaseIncrement = (2.0 * PD_PI) / COSINE_TABLE_SIZE;
    int i;
    
    cos_tilde_table = (t_float *)PD_MEMORY_GET (sizeof (t_float) * (COSINE_TABLE_SIZE + 1));
    
    for (i = 0; i < COSINE_TABLE_SIZE + 1; i++) {
        cos_tilde_table[i] = (t_float)cos ((double)phase);
        phase += phaseIncrement;
    }
    
    /* Test raw cast byte alignment at startup. */
    
    {
        t_rawcast64 z;
        z.z_d = DSP_UNITBIT32 + 0.5;
        PD_ASSERT ((z.z_i[PD_RAWCAST64_LSB] == 0x80000000));
        PD_ABORT (!(z.z_i[PD_RAWCAST64_LSB] == 0x80000000));
    }
    //
    }
}

void cos_tilde_release (void)
{
    PD_MEMORY_FREE (cos_tilde_table);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *cos_tilde_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    float *tab = cos_tilde_table, *addr, f1, f2, frac;
    double dphase;
    int normhipart;
    t_rawcast64 tf;
    
    tf.z_d = DSP_UNITBIT32;
    normhipart = tf.z_i[PD_RAWCAST64_MSB];

#if 0           /* this is the readable version of the code. */
    while (n--)
    {
        dphase = (double)(*in++ * (float)(COSINE_TABLE_SIZE)) + DSP_UNITBIT32;
        tf.z_d = dphase;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
        frac = tf.z_d - DSP_UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        *out++ = f1 + frac * (f2 - f1);
    }
#endif
#if 1           /* this is the same, unwrapped by hand. */
        dphase = (double)(*in++ * (float)(COSINE_TABLE_SIZE)) + DSP_UNITBIT32;
        tf.z_d = dphase;
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    while (--n)
    {
        dphase = (double)(*in++ * (float)(COSINE_TABLE_SIZE)) + DSP_UNITBIT32;
            frac = tf.z_d - DSP_UNITBIT32;
        tf.z_d = dphase;
            f1 = addr[0];
            f2 = addr[1];
        addr = tab + (tf.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE-1));
            *out++ = f1 + frac * (f2 - f1);
        tf.z_i[PD_RAWCAST64_MSB] = normhipart;
    }
            frac = tf.z_d - DSP_UNITBIT32;
            f1 = addr[0];
            f2 = addr[1];
            *out++ = f1 + frac * (f2 - f1);
#endif
    return (w+4);
}

static void cos_tilde_dsp (t_cos_tilde *x, t_signal **sp)
{
    dsp_add (cos_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *cos_tilde_new (void)
{
    t_cos_tilde *x = (t_cos_tilde *)pd_new (cos_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    class_addDSP (c, cos_tilde_dsp);
    
    cos_tilde_initialize();
    
    cos_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
