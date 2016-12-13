
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
#include "d_delay.h"

extern t_class *sigdelwrite_class;
/* ----------------------------- delread~ ----------------------------- */
static t_class *sigdelread_class;

typedef struct _sigdelread
{
    t_object x_obj;
    t_symbol *x_sym;
    t_float x_deltime;  /* delay in msec */
    int x_delsamps;     /* delay in samples */
    t_float x_sr;       /* samples per msec */
    t_float x_n;        /* vector size */
    int x_zerodel;      /* 0 or vecsize depending on read/write order */
} t_sigdelread;

static void sigdelread_float(t_sigdelread *x, t_float f);

static void *sigdelread_new(t_symbol *s, t_float f)
{
    t_sigdelread *x = (t_sigdelread *)pd_new(sigdelread_class);
    x->x_sym = s;
    x->x_sr = 1;
    x->x_n = 1;
    x->x_zerodel = 0;
    sigdelread_float(x, f);
    outlet_new(&x->x_obj, &s_signal);
    return x;
}

static void sigdelread_float(t_sigdelread *x, t_float f)
{
    int samps;
    t_delwrite_tilde *delwriter =
        (t_delwrite_tilde *)pd_getThingByClass(x->x_sym, sigdelwrite_class);
    x->x_deltime = f;
    if (delwriter)
    {
        int delsize = delwriter->dw_space.c_size;
        x->x_delsamps = (int)(0.5 + x->x_sr * x->x_deltime)
            + x->x_n - x->x_zerodel;
        if (x->x_delsamps < x->x_n) x->x_delsamps = x->x_n;
        else if (x->x_delsamps > delwriter->dw_space.c_size)
            x->x_delsamps = delwriter->dw_space.c_size;
    }
}

static t_int *sigdelread_perform(t_int *w)
{
    t_sample *out = (t_sample *)(w[1]);
    t_delwrite_tilde_control *c = (t_delwrite_tilde_control *)(w[2]);
    int delsamps = *(int *)(w[3]);
    int n = (int)(w[4]);
    int phase = c->c_phase - delsamps, nsamps = c->c_size;
    t_sample *vp = c->c_vector, *bp, *ep = vp + (c->c_size + DELAY_EXTRA_SAMPLES);
    if (phase < 0) phase += nsamps;
    bp = vp + phase;

    while (n--)
    {
        *out++ = *bp++;
        if (bp == ep) bp -= nsamps;
    }
    return (w+5);
}

static void sigdelread_dsp(t_sigdelread *x, t_signal **sp)
{
    t_delwrite_tilde *delwriter =
        (t_delwrite_tilde *)pd_getThingByClass(x->x_sym, sigdelwrite_class);
    x->x_sr = sp[0]->s_sampleRate * 0.001;
    x->x_n = sp[0]->s_vectorSize;
    if (delwriter)
    {
        sigdelwrite_updatesr(delwriter, sp[0]->s_sampleRate);
        sigdelwrite_checkvecsize(delwriter, sp[0]->s_vectorSize);
        x->x_zerodel = (delwriter->dw_buildIdentifier == ugen_getBuildIdentifier() ?
            0 : delwriter->dw_vectorSize);
        sigdelread_float(x, x->x_deltime);
        dsp_add(sigdelread_perform, 4,
            sp[0]->s_vector, &delwriter->dw_space, &x->x_delsamps, sp[0]->s_vectorSize);
    }
    else if (*x->x_sym->s_name)
        post_error ("delread~: %s: no such delwrite~",x->x_sym->s_name);
}

void sigdelread_setup(void)
{
    sigdelread_class = class_new(sym_delread__tilde__,
        (t_newmethod)sigdelread_new, 0,
        sizeof(t_sigdelread), 0, A_DEFSYMBOL, A_DEFFLOAT, 0);
    class_addMethod(sigdelread_class, (t_method)sigdelread_dsp,
        sym_dsp, A_CANT, 0);
    class_addFloat(sigdelread_class, (t_method)sigdelread_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

