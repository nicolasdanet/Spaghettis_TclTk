
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

t_float rsqrt_tableMantissa[RSQRT_MANTISSA_SIZE];                   /* Shared. */
t_float rsqrt_tableExponential[RSQRT_EXPONENTIAL_SIZE];             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void rsqrt_tilde_initialize (void)
{
    int i;
    for (i = 0; i < RSQRT_EXPONENTIAL_SIZE; i++)
    {
        union {
          float f;
          long l;
        } u;
        int32_t l = (i ? (i == RSQRT_EXPONENTIAL_SIZE-1 ? RSQRT_EXPONENTIAL_SIZE-2 : i) : 1)<< 23;
        u.l = l;
        rsqrt_tableExponential[i] = 1./sqrt(u.f);   
    }
    for (i = 0; i < RSQRT_MANTISSA_SIZE; i++)
    {
        float f = 1 + (1./RSQRT_MANTISSA_SIZE) * i;
        rsqrt_tableMantissa[i] = 1./sqrt(f);      
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *rsqrt_tilde_perform (t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {   
        t_sample f = *in++;
        union {
          float f;
          long l;
        } u;
        u.f = f;
        if (f < 0) *out++ = 0;
        else
        {
            t_sample g = rsqrt_tableExponential[(u.l >> 23) & 0xff] *
                rsqrt_tableMantissa[(u.l >> 13) & 0x3ff];
            *out++ = 1.5 * g - 0.5 * g * g * g * f;
        }
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
    
    class_addDSP (c, rsqrt_tilde_dsp);
    
    #if PD_WITH_LEGACY
    
    class_addCreator (rsqrt_tilde_new, sym_q8_rsqrt__tilde__, A_NULL);
    
    #endif
    
    rsqrt_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
