
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

/* -------------------------- sig~ ------------------------------ */
static t_class *sig_tilde_class;

typedef struct _sig_tilde
{
    t_object x_obj;
    t_float x_f;
} t_sig;

static void sig_tilde_float(t_sig *x, t_float f)
{
    x->x_f = f;
}

static void sig_tilde_dsp(t_sig *x, t_signal **sp)
{
    dsp_addScalarPerform (&x->x_f, sp[0]->s_vector, sp[0]->s_vectorSize);
    // dsp_add(sig_tilde_perform, 3, &x->x_f, sp[0]->s_vector, sp[0]->s_vectorSize);
}

static void *sig_tilde_new(t_float f)
{
    t_sig *x = (t_sig *)pd_new(sig_tilde_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_signal);
    return x;
}

void sig_tilde_setup(void)
{
    sig_tilde_class = class_new(sym_sig__tilde__, (t_newmethod)sig_tilde_new, 0,
        sizeof(t_sig), 0, A_DEFFLOAT, 0);
    class_addFloat(sig_tilde_class, (t_method)sig_tilde_float);
    class_addMethod(sig_tilde_class, (t_method)sig_tilde_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* -------------------------- line~ ------------------------------ */
static t_class *line_tilde_class;

typedef struct _line_tilde
{
    t_object x_obj;
    t_sample x_target; /* target value of ramp */
    t_sample x_value; /* current value of ramp at block-borders */
    t_sample x_biginc;
    t_sample x_inc;
    t_float x_1overn;
    t_float x_dspticktomsec;
    t_float x_inletvalue;
    t_float x_inletwas;
    int x_ticksleft;
    int x_retarget;
} t_line_tilde;

static t_int *line_tilde_perform(t_int *w)
{
    t_line_tilde *x = (t_line_tilde *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample f = x->x_value;

    if (PD_BIG_OR_SMALL(f))
            x->x_value = f = 0;
    if (x->x_retarget)
    {
        int nticks = x->x_inletwas * x->x_dspticktomsec;
        if (!nticks) nticks = 1;
        x->x_ticksleft = nticks;
        x->x_biginc = (x->x_target - x->x_value)/(t_float)nticks;
        x->x_inc = x->x_1overn * x->x_biginc;
        x->x_retarget = 0;
    }
    if (x->x_ticksleft)
    {
        t_sample f = x->x_value;
        while (n--) *out++ = f, f += x->x_inc;
        x->x_value += x->x_biginc;
        x->x_ticksleft--;
    }
    else
    {
        t_sample g = x->x_value = x->x_target;
        while (n--)
            *out++ = g;
    }
    return (w+4);
}

/* TB: vectorized version */
static t_int *line_tilde_perf8(t_int *w)
{
    t_line_tilde *x = (t_line_tilde *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample f = x->x_value;

    if (PD_BIG_OR_SMALL(f))
        x->x_value = f = 0;
    if (x->x_retarget)
    {
        int nticks = x->x_inletwas * x->x_dspticktomsec;
        if (!nticks) nticks = 1;
        x->x_ticksleft = nticks;
        x->x_biginc = (x->x_target - x->x_value)/(t_sample)nticks;
        x->x_inc = x->x_1overn * x->x_biginc;
        x->x_retarget = 0;
    }
    if (x->x_ticksleft)
    {
        t_sample f = x->x_value;
        while (n--) *out++ = f, f += x->x_inc;
        x->x_value += x->x_biginc;
        x->x_ticksleft--;
    }
    else
    {
        t_sample f = x->x_value = x->x_target;
        for (; n; n -= 8, out += 8)
        {
            out[0] = f; out[1] = f; out[2] = f; out[3] = f; 
            out[4] = f; out[5] = f; out[6] = f; out[7] = f;
        }
    }
    return (w+4);
}

static void line_tilde_float(t_line_tilde *x, t_float f)
{
    if (x->x_inletvalue <= 0)
    {
        x->x_target = x->x_value = f;
        x->x_ticksleft = x->x_retarget = 0;
    }
    else
    {
        x->x_target = f;
        x->x_retarget = 1;
        x->x_inletwas = x->x_inletvalue;
        x->x_inletvalue = 0;
    }
}

static void line_tilde_stop(t_line_tilde *x)
{
    x->x_target = x->x_value;
    x->x_ticksleft = x->x_retarget = 0;
}

static void line_tilde_dsp(t_line_tilde *x, t_signal **sp)
{
    if(sp[0]->s_vectorSize&7)
        dsp_add(line_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    else
        dsp_add(line_tilde_perf8, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    x->x_1overn = 1./sp[0]->s_vectorSize;
    x->x_dspticktomsec = sp[0]->s_sampleRate / (1000 * sp[0]->s_vectorSize);
}

static void *line_tilde_new(void)
{
    t_line_tilde *x = (t_line_tilde *)pd_new(line_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    inlet_newFloat(&x->x_obj, &x->x_inletvalue);
    x->x_ticksleft = x->x_retarget = 0;
    x->x_value = x->x_target = x->x_inletvalue = x->x_inletwas = 0;
    return x;
}

void line_tilde_setup(void)
{
    line_tilde_class = class_new(sym_line__tilde__, line_tilde_new, 0,
        sizeof(t_line_tilde), 0, 0);
    class_addFloat(line_tilde_class, (t_method)line_tilde_float);
    class_addMethod(line_tilde_class, (t_method)line_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(line_tilde_class, (t_method)line_tilde_stop,
        sym_stop, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
