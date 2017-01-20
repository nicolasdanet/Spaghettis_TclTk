
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
#include "d_math.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Reciprocal square root good to 8 mantissa bits. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Assumed IEEE 754 floating-point format. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that a float can be factorized into two floats. */
/* For one keep the mantissa and set the exponent to zero (i.e 0x7f with the bias). */
/* For the other keep the exponent and set the mantissa to zero. */
/* Thus the rsqrt is approximated by the product of two (with fast lookup) rsqrt. */

/* < https://en.wikipedia.org/wiki/Fast_inverse_square_root#Newton.27s_method > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Need probably some benchmarks now. */

/* < http://assemblyrequired.crashworks.org/timing-square-root/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *rsqrt_tilde_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _rsqrt_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_rsqrt_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float rsqrt_tableMantissa[RSQRT_MANTISSA_SIZE];           /* Shared. */
t_float rsqrt_tableExponential[RSQRT_EXPONENTIAL_SIZE];     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void rsqrt_tilde_initialize (void)
{
    static int rsqrt_initialized = 0;
    
    if (!rsqrt_initialized++) {
    //
    int i;
    
    for (i = 0; i < RSQRT_EXPONENTIAL_SIZE; i++) {

        t_rawcast32 z;
        
        if (i == 0) { z.z_i = 1 << 23; }
        else {
            z.z_i = (i == RSQRT_EXPONENTIAL_SIZE - 1 ? RSQRT_EXPONENTIAL_SIZE - 2 : i) << 23;
        }

        rsqrt_tableExponential[i] = 1.0 / sqrt (z.z_f);
    }
    
    for (i = 0; i < RSQRT_MANTISSA_SIZE; i++) {
    
        /* Exponent is zero with a float inside the 1.0 to 2.0 range. */
        
        t_float f = 1.0 + (1.0 / RSQRT_MANTISSA_SIZE) * i;
        
        rsqrt_tableMantissa[i] = 1.0 / sqrt (f);      
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *rsqrt_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) {
    //
    t_rawcast32 z;

    z.z_f = *in++;
        
    if (z.z_f < 0.0) { *out++ = 0.0; }
    else {
    //
    int e = (z.z_i >> 23) & (RSQRT_EXPONENTIAL_SIZE - 1);
    int m = (z.z_i >> 13) & (RSQRT_MANTISSA_SIZE - 1);
    t_sample g = rsqrt_tableExponential[e] * rsqrt_tableMantissa[m];
    
    *out++ = 1.5 * g - 0.5 * g * g * g * z.z_f;
    //
    }
    //
    }
    
    return (w + 4);
}

static void rsqrt_tilde_dsp (t_rsqrt_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (rsqrt_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *rsqrt_tilde_new (void)
{
    t_rsqrt_tilde *x = (t_rsqrt_tilde *)pd_new (rsqrt_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rsqrt_tilde_setup (void)
{
    t_class *c = NULL;
    
    rsqrt_tilde_initialize();
    
    c = class_new (sym_rsqrt__tilde__,
            (t_newmethod)rsqrt_tilde_new,
            NULL,
            sizeof (t_rsqrt_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    CLASS_SIGNAL (c, t_rsqrt_tilde, x_f);
    
    class_addDSP (c, (t_method)rsqrt_tilde_dsp);
    
    #if PD_WITH_LEGACY
    
    class_addCreator ((t_newmethod)rsqrt_tilde_new, sym_q8_rsqrt__tilde__, A_NULL);
    
    #endif
    
    rsqrt_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
