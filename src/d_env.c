
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
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---------------- env~ - simple envelope follower. ----------------- */

#define MAXOVERLAP 32
#define INITVSTAKEN 64

typedef struct sigenv
{
    t_object x_obj;                 /* header */
    void *x_outlet;                 /* a "float" outlet */
    void *x_clock;                  /* a "clock" object */
    t_sample *x_buf;                   /* a Hanning window */
    int x_phase;                    /* number of points since last output */
    int x_period;                   /* requested period of output */
    int x_realperiod;               /* period rounded up to vecsize multiple */
    int x_npoints;                  /* analysis window size in samples */
    t_float x_result;                 /* result to output */
    t_sample x_sumbuf[MAXOVERLAP];     /* summing buffer */
    t_float x_f;
    int x_allocforvs;               /* extra buffer for DSP vector size */
} t_sigenv;

t_class *env_tilde_class;
static void env_tilde_tick(t_sigenv *x);

static void *env_tilde_new(t_float fnpoints, t_float fperiod)
{
    int npoints = fnpoints;
    int period = fperiod;
    t_sigenv *x;
    t_sample *buf;
    int i;

    if (npoints < 1) npoints = 1024;
    if (period < 1) period = npoints/2;
    if (period < npoints / MAXOVERLAP + 1)
        period = npoints / MAXOVERLAP + 1;
    if (!(buf = PD_MEMORY_GET(sizeof(t_sample) * (npoints + INITVSTAKEN))))
    {
        post_error ("env: couldn't allocate buffer");
        return (0);
    }
    x = (t_sigenv *)pd_new(env_tilde_class);
    x->x_buf = buf;
    x->x_npoints = npoints;
    x->x_phase = 0;
    x->x_period = period;
    for (i = 0; i < MAXOVERLAP; i++) x->x_sumbuf[i] = 0;
    for (i = 0; i < npoints; i++)
        buf[i] = (1. - cos((2 * PD_PI * i) / npoints))/npoints;
    for (; i < npoints+INITVSTAKEN; i++) buf[i] = 0;
    x->x_clock = clock_new(x, (t_method)env_tilde_tick);
    x->x_outlet = outlet_new(&x->x_obj, &s_float);
    x->x_f = 0;
    x->x_allocforvs = INITVSTAKEN;
    return x;
}

static t_int *env_tilde_perform(t_int *w)
{
    t_sigenv *x = (t_sigenv *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    int count;
    t_sample *sump; 
    in += n;
    for (count = x->x_phase, sump = x->x_sumbuf;
        count < x->x_npoints; count += x->x_realperiod, sump++)
    {
        t_sample *hp = x->x_buf + count;
        t_sample *fp = in;
        t_sample sum = *sump;
        int i;
        
        for (i = 0; i < n; i++)
        {
            fp--;
            sum += *hp++ * (*fp * *fp);
        }
        *sump = sum;
    }
    sump[0] = 0;
    x->x_phase -= n;
    if (x->x_phase < 0)
    {
        x->x_result = x->x_sumbuf[0];
        for (count = x->x_realperiod, sump = x->x_sumbuf;
            count < x->x_npoints; count += x->x_realperiod, sump++)
                sump[0] = sump[1];
        sump[0] = 0;
        x->x_phase = x->x_realperiod - n;
        clock_delay(x->x_clock, 0L);
    }
    return (w+4);
}

static void env_tilde_dsp(t_sigenv *x, t_signal **sp)
{
    if (x->x_period % sp[0]->s_vectorSize) x->x_realperiod =
        x->x_period + sp[0]->s_vectorSize - (x->x_period % sp[0]->s_vectorSize);
    else x->x_realperiod = x->x_period;
    if (sp[0]->s_vectorSize > x->x_allocforvs)
    {
        void *xx = PD_MEMORY_RESIZE(x->x_buf,
            (x->x_npoints + x->x_allocforvs) * sizeof(t_sample),
            (x->x_npoints + sp[0]->s_vectorSize) * sizeof(t_sample));
        if (!xx)
        {
            post_error ("env~: out of memory");
            return;
        }
        x->x_buf = (t_sample *)xx;
        x->x_allocforvs = sp[0]->s_vectorSize;
    }
    dsp_add(env_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

static void env_tilde_tick(t_sigenv *x) /* callback function for the clock */
{
    outlet_float(x->x_outlet, math_powerToDecibel(x->x_result));
}

static void env_tilde_ff(t_sigenv *x)           /* cleanup on free */
{
    clock_free(x->x_clock);
    PD_MEMORY_FREE(x->x_buf);
}


void env_tilde_setup(void )
{
    env_tilde_class = class_new(sym_env__tilde__, (t_newmethod)env_tilde_new,
        (t_method)env_tilde_ff, sizeof(t_sigenv), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(env_tilde_class, t_sigenv, x_f);
    class_addMethod(env_tilde_class, (t_method)env_tilde_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
