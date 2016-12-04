
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

/* ------------------------- clip~ -------------------------- */
static t_class *clip_class;

typedef struct _clip_tilde
{
    t_object x_obj;
    t_float x_f;
    t_float x_lo;
    t_float x_hi;
} t_clip_tilde;

static void *clip_tilde_new(t_float lo, t_float hi)
{
    t_clip_tilde *x = (t_clip_tilde *)pd_new(clip_class);
    x->x_lo = lo;
    x->x_hi = hi;
    outlet_new(&x->x_obj, &s_signal);
    inlet_newFloat(&x->x_obj, &x->x_lo);
    inlet_newFloat(&x->x_obj, &x->x_hi);
    x->x_f = 0;
    return (x);
}

static t_int *clip_perform(t_int *w)
{
    t_clip_tilde *x = (t_clip_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
        t_sample f = *in++;
        if (f < x->x_lo) f = x->x_lo;
        if (f > x->x_hi) f = x->x_hi;
        *out++ = f;
    }
    return (w+5);
}

static void clip_dsp(t_clip_tilde *x, t_signal **sp)
{
    dsp_add(clip_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void clip_tilde_setup(void)
{
    clip_class = class_new(sym_clip__tilde__, (t_newmethod)clip_tilde_new, 0,
        sizeof(t_clip_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(clip_class, t_clip_tilde, x_f);
    class_addMethod(clip_class, (t_method)clip_dsp, sym_dsp, A_CANT, 0);
}

/* sigrsqrt - reciprocal square root good to 8 mantissa bits  */

#define DUMTAB1SIZE 256
#define DUMTAB2SIZE 1024

static float rsqrt_exptab[DUMTAB1SIZE], rsqrt_mantissatab[DUMTAB2SIZE];

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


/* sigsqrt -  square root good to 8 mantissa bits  */

typedef struct sigsqrt
{
    t_object x_obj;
    t_float x_f;
} t_sigsqrt;

static t_class *sigsqrt_class;

static void *sigsqrt_new(void)
{
    t_sigsqrt *x = (t_sigsqrt *)pd_new(sigsqrt_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

t_int *sigsqrt_perform(t_int *w)    /* not static; also used in d_fft.c */
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
            *out++ = f * (1.5 * g - 0.5 * g * g * g * f);
        }
    }
    return (w + 4);
}

static void sigsqrt_dsp(t_sigsqrt *x, t_signal **sp)
{
    dsp_add(sigsqrt_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void sigsqrt_tilde_setup(void)
{
    sigsqrt_class = class_new(sym_sqrt__tilde__, (t_newmethod)sigsqrt_new, 0,
        sizeof(t_sigsqrt), 0, 0);
    class_addCreator(sigsqrt_new, sym_q8_sqrt__tilde__, 0);   /* LEGACY !!! */
    CLASS_SIGNAL(sigsqrt_class, t_sigsqrt, x_f);
    class_addMethod(sigsqrt_class, (t_method)sigsqrt_dsp,
        sym_dsp, A_CANT, 0);
}

/* ------------------------------ wrap~ -------------------------- */

typedef struct wrap
{
    t_object x_obj;
    t_float x_f;
} t_sigwrap;

t_class *sigwrap_class;

static void *sigwrap_new(void)
{
    t_sigwrap *x = (t_sigwrap *)pd_new(sigwrap_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *sigwrap_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {   
        t_sample f = *in++;
        int k = f;
        if (f > 0) *out++ = f-k;
        else *out++ = f - (k-1);
    }
    return (w + 4);
}

static void sigwrap_dsp(t_sigwrap *x, t_signal **sp)
{
    dsp_add(sigwrap_perform, 3, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

void sigwrap_tilde_setup(void)
{
    sigwrap_class = class_new(sym_wrap__tilde__, (t_newmethod)sigwrap_new, 0,
        sizeof(t_sigwrap), 0, 0);
    CLASS_SIGNAL(sigwrap_class, t_sigwrap, x_f);
    class_addMethod(sigwrap_class, (t_method)sigwrap_dsp,
        sym_dsp, A_CANT, 0);
}
