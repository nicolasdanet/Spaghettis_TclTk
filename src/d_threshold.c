
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

/* --------------------- threshold~ ----------------------------- */

static t_class *threshold_tilde_class;

typedef struct _threshold_tilde
{
    t_object x_obj;
    t_outlet *x_outlet1;        /* bang out for high thresh */
    t_outlet *x_outlet2;        /* bang out for low thresh */
    t_clock *x_clock;           /* wakeup for message output */
    t_float x_f;                  /* scalar inlet */
    int x_state;                /* 1 = high, 0 = low */
    t_float x_hithresh;           /* value of high threshold */
    t_float x_lothresh;           /* value of low threshold */
    t_float x_deadwait;           /* msec remaining in dead period */
    t_float x_msecpertick;        /* msec per DSP tick */
    t_float x_hideadtime;         /* hi dead time in msec */
    t_float x_lodeadtime;         /* lo dead time in msec */
} t_threshold_tilde;

static void threshold_tilde_tick(t_threshold_tilde *x);
static void threshold_tilde_set(t_threshold_tilde *x,
    t_float hithresh, t_float hideadtime,
    t_float lothresh, t_float lodeadtime);

static t_threshold_tilde *threshold_tilde_new(t_float hithresh,
    t_float hideadtime, t_float lothresh, t_float lodeadtime)
{
    t_threshold_tilde *x = (t_threshold_tilde *)
        pd_new(threshold_tilde_class);
    x->x_state = 0;             /* low state */
    x->x_deadwait = 0;          /* no dead time */
    x->x_clock = clock_new(x, (t_method)threshold_tilde_tick);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_bang);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_bang);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_float, sym_inlet2);
    x->x_msecpertick = 0.;
    x->x_f = 0;
    threshold_tilde_set(x, hithresh, hideadtime, lothresh, lodeadtime);
    return x;
}

    /* "set" message to specify thresholds and dead times */
static void threshold_tilde_set(t_threshold_tilde *x,
    t_float hithresh, t_float hideadtime,
    t_float lothresh, t_float lodeadtime)
{
    if (lothresh > hithresh)
        lothresh = hithresh;
    x->x_hithresh = hithresh;
    x->x_hideadtime = hideadtime;
    x->x_lothresh = lothresh;
    x->x_lodeadtime = lodeadtime;
}

    /* number in inlet sets state -- note incompatible with JMAX which used
    "int" message for this, impossible here because of auto signal conversion */
static void threshold_tilde_ft1(t_threshold_tilde *x, t_float f)
{
    x->x_state = (f != 0);
    x->x_deadwait = 0;
}

static void threshold_tilde_tick(t_threshold_tilde *x)  
{
    if (x->x_state)
        outlet_bang(x->x_outlet1);
    else outlet_bang(x->x_outlet2);
}

static t_int *threshold_tilde_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_threshold_tilde *x = (t_threshold_tilde *)(w[2]);
    int n = (t_int)(w[3]);
    if (x->x_deadwait > 0)
        x->x_deadwait -= x->x_msecpertick;
    else if (x->x_state)
    {
            /* we're high; look for low sample */
        for (; n--; in1++)
        {
            if (*in1 < x->x_lothresh)
            {
                clock_delay(x->x_clock, 0L);
                x->x_state = 0;
                x->x_deadwait = x->x_lodeadtime;
                goto done;
            }
        }
    }
    else
    {
            /* we're low; look for high sample */
        for (; n--; in1++)
        {
            if (*in1 >= x->x_hithresh)
            {
                clock_delay(x->x_clock, 0L);
                x->x_state = 1;
                x->x_deadwait = x->x_hideadtime;
                goto done;
            }
        }
    }
done:
    return (w+4);
}

void threshold_tilde_dsp(t_threshold_tilde *x, t_signal **sp)
{
    x->x_msecpertick = 1000. * sp[0]->s_vectorSize / sp[0]->s_sampleRate;
    dsp_add(threshold_tilde_perform, 3, sp[0]->s_vector, x, sp[0]->s_vectorSize);
}

static void threshold_tilde_ff(t_threshold_tilde *x)
{
    clock_free(x->x_clock);
}

void threshold_tilde_setup( void)
{
    threshold_tilde_class = class_new(sym_threshold__tilde__,
        (t_newmethod)threshold_tilde_new, (t_method)threshold_tilde_ff,
        sizeof(t_threshold_tilde), 0,
            A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(threshold_tilde_class, t_threshold_tilde, x_f);
    class_addMethod(threshold_tilde_class, (t_method)threshold_tilde_set,
        sym_set, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(threshold_tilde_class, (t_method)threshold_tilde_ft1,
        sym_inlet2, A_FLOAT, 0);
    class_addMethod(threshold_tilde_class, (t_method)threshold_tilde_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
