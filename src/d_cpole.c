
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

/* -------------- cpole~ - complex one-pole filter (raw) --------------- */

typedef struct sigcpole
{
    t_object x_obj;
    t_float x_f;
    t_sample x_lastre;
    t_sample x_lastim;
} t_sigcpole;

t_class *sigcpole_class;

static void *sigcpole_new(t_float re, t_float im)
{
    t_sigcpole *x = (t_sigcpole *)pd_new(sigcpole_class);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    pd_float(
        (t_pd *)inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal),
            re);
    pd_float(
        (t_pd *)inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal),
            im);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_lastre = x->x_lastim = 0;
    x->x_f = 0;
    return x;
}

static t_int *sigcpole_perform(t_int *w)
{
    t_sample *inre1 = (t_sample *)(w[1]);
    t_sample *inim1 = (t_sample *)(w[2]);
    t_sample *inre2 = (t_sample *)(w[3]);
    t_sample *inim2 = (t_sample *)(w[4]);
    t_sample *outre = (t_sample *)(w[5]);
    t_sample *outim = (t_sample *)(w[6]);
    t_sigcpole *x = (t_sigcpole *)(w[7]);
    int n = (t_int)(w[8]);
    int i;
    t_sample lastre = x->x_lastre;
    t_sample lastim = x->x_lastim;
    for (i = 0; i < n; i++)
    {
        t_sample nextre = *inre1++;
        t_sample nextim = *inim1++;
        t_sample coefre = *inre2++;
        t_sample coefim = *inim2++;
        t_sample tempre = *outre++ = nextre + lastre * coefre - lastim * coefim;
        lastim = *outim++ = nextim + lastre * coefim + lastim * coefre;
        lastre = tempre;
    }
    if (PD_IS_BIG_OR_SMALL(lastre))
        lastre = 0;
    if (PD_IS_BIG_OR_SMALL(lastim))
        lastim = 0;
    x->x_lastre = lastre;
    x->x_lastim = lastim;
    return (w+9);
}

static void sigcpole_dsp(t_sigcpole *x, t_signal **sp)
{
    dsp_add(sigcpole_perform, 8,
        sp[0]->s_vector, sp[1]->s_vector, sp[2]->s_vector, sp[3]->s_vector, 
        sp[4]->s_vector, sp[5]->s_vector, x, sp[0]->s_vectorSize);
}

static void sigcpole_clear(t_sigcpole *x)
{
    x->x_lastre = x->x_lastim = 0;
}

static void sigcpole_set(t_sigcpole *x, t_float re, t_float im)
{
    x->x_lastre = re;
    x->x_lastim = im;
}

void sigcpole_setup(void)
{
    sigcpole_class = class_new(sym_cpole__tilde__,
        (t_newmethod)sigcpole_new, 0, sizeof(t_sigcpole), 0, 
            A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigcpole_class, t_sigcpole, x_f);
    class_addMethod(sigcpole_class, (t_method)sigcpole_set,
        sym_set, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(sigcpole_class, (t_method)sigcpole_clear,
        sym_clear, 0);
    class_addMethod(sigcpole_class, (t_method)sigcpole_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
