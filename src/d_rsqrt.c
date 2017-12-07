
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
// MARK: -

t_float rsqrt_tableMantissa[RSQRT_MANTISSA_SIZE];           /* Static. */
t_float rsqrt_tableExponential[RSQRT_EXPONENTIAL_SIZE];     /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void rsqrt_tilde_initialize (void)
{
    int i;
    
    for (i = 0; i < RSQRT_EXPONENTIAL_SIZE; i++) {

        t_rawcast32 z;
        
        if (i == 0) { z.z_i = 1 << 23; }
        else {
            z.z_i = (i == RSQRT_EXPONENTIAL_SIZE - 1 ? RSQRT_EXPONENTIAL_SIZE - 2 : i) << 23;
        }

        rsqrt_tableExponential[i] = (t_float)(1.0 / sqrt (z.z_f));
    }
    
    for (i = 0; i < RSQRT_MANTISSA_SIZE; i++) {
    
        /* Exponent is zero with a IEEE float inside the 1.0 to 2.0 range. */
        
        t_float f = (t_float)(1.0 + (1.0 / RSQRT_MANTISSA_SIZE) * i);
        
        rsqrt_tableMantissa[i] = (t_float)(1.0 / sqrt (f));      
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* No aliasing. */

static t_int *rsqrt_tilde_perform (t_int *w)
{
    PD_RESTRICTED in  = (t_sample *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    while (n--) { *out++ = (t_sample)rsqrt_fast ((t_float)(*in++)); }
    
    return (w + 4);
}

static void rsqrt_tilde_dsp (t_rsqrt_tilde *x, t_signal **sp)
{
    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (rsqrt_tilde_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *rsqrt_tilde_new (void)
{
    t_rsqrt_tilde *x = (t_rsqrt_tilde *)pd_new (rsqrt_tilde_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void rsqrt_tilde_setup (void)
{
    t_class *c = NULL;
    
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
    
    class_setHelpName (c, sym_math__tilde__);
    
    rsqrt_tilde_class = c;
}

void rsqrt_tilde_destroy (void)
{
    class_free (rsqrt_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
