
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

/* sigrsqrt - reciprocal square root good to 8 mantissa bits  */

#define DUMTAB1SIZE 256
#define DUMTAB2SIZE 1024

float rsqrt_exptab[DUMTAB1SIZE];
float rsqrt_mantissatab[DUMTAB2SIZE];

static void init_rsqrt(void)
{
    int i;
    for (i = 0; i < DUMTAB1SIZE; i++)
    {
        union {
          float f;
          long l;
        } u;
        int32_t l = (i ? (i == DUMTAB1SIZE-1 ? DUMTAB1SIZE-2 : i) : 1)<< 23;
        u.l = l;
        rsqrt_exptab[i] = 1./sqrt(u.f);   
    }
    for (i = 0; i < DUMTAB2SIZE; i++)
    {
        float f = 1 + (1./DUMTAB2SIZE) * i;
        rsqrt_mantissatab[i] = 1./sqrt(f);      
    }
}

    /* these are used in externs like "bonk" */

/*
t_float q8_rsqrt(t_float f0)
{
    union {
      float f;
      long l;
    } u;
    u.f=f0;
    if (u.f < 0) return (0);
    else return (rsqrt_exptab[(u.l >> 23) & 0xff] *
            rsqrt_mantissatab[(u.l >> 13) & 0x3ff]);
}

t_float q8_sqrt(t_float f0)
{
    union {
      float f;
      long l;
    } u;
    u.f=f0;
    if (u.f < 0) return (0);
    else return (u.f * rsqrt_exptab[(u.l >> 23) & 0xff] *
            rsqrt_mantissatab[(u.l >> 13) & 0x3ff]);
}
*/
typedef struct sigrsqrt
{
    t_object x_obj;
    t_float x_f;
} t_sigrsqrt;

static t_class *sigrsqrt_class;

static void *sigrsqrt_new(void)
{
    t_sigrsqrt *x = (t_sigrsqrt *)pd_new(sigrsqrt_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *sigrsqrt_perform(t_int *w)
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
            t_sample g = rsqrt_exptab[(u.l >> 23) & 0xff] *
                rsqrt_mantissatab[(u.l >> 13) & 0x3ff];
            *out++ = 1.5 * g - 0.5 * g * g * g * f;
        }
    }
    return (w + 4);
}

static void sigrsqrt_dsp(t_sigrsqrt *x, t_signal **sp)
{
    dsp_add(sigrsqrt_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void sigrsqrt_tilde_setup(void)
{
    init_rsqrt();
    sigrsqrt_class = class_new(sym_rsqrt__tilde__, (t_newmethod)sigrsqrt_new, 0,
        sizeof(t_sigrsqrt), 0, 0);
            /* an old name for it: */
    class_addCreator(sigrsqrt_new, sym_q8_rsqrt__tilde__, 0); /* LEGACY !!! */
    CLASS_SIGNAL(sigrsqrt_class, t_sigrsqrt, x_f);
    class_addMethod(sigrsqrt_class, (t_method)sigrsqrt_dsp,
        sym_dsp, A_CANT, 0);
}
