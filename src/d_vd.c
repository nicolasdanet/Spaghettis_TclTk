
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

/* ----------------------------- vd~ ----------------------------- */
static t_class *sigvd_class;

typedef struct _sigvd
{
    t_object x_obj;
    t_symbol *x_sym;
    t_float x_sr;       /* samples per msec */
    int x_zerodel;      /* 0 or vecsize depending on read/write order */
    t_float x_f;
} t_sigvd;

static void *sigvd_new(t_symbol *s)
{
    t_sigvd *x = (t_sigvd *)pd_new(sigvd_class);
    x->x_sym = s;
    x->x_sr = 1;
    x->x_zerodel = 0;
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *sigvd_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_delwrite_tilde_control *ctl = (t_delwrite_tilde_control *)(w[3]);
    t_sigvd *x = (t_sigvd *)(w[4]);
    int n = (int)(w[5]);

    int nsamps = ctl->c_size;
    t_sample limit = nsamps - n;
    t_sample fn = n-1;
    t_sample *vp = ctl->c_vector, *bp, *wp = vp + ctl->c_phase;
    t_sample zerodel = x->x_zerodel;
    while (n--)
    {
        t_sample delsamps = x->x_sr * *in++ - zerodel, frac;
        int idelsamps;
        t_sample a, b, c, d, cminusb;
        if (!(delsamps >= 1.00001f))    /* too small or NAN */
            delsamps = 1.00001f;
        if (delsamps > limit)           /* too big */
            delsamps = limit;
        delsamps += fn;
        fn = fn - 1.0f;
        idelsamps = delsamps;
        frac = delsamps - (t_sample)idelsamps;
        bp = wp - idelsamps;
        if (bp < vp + 4) bp += nsamps;
        d = bp[-3];
        c = bp[-2];
        b = bp[-1];
        a = bp[0];
        cminusb = c-b;
        *out++ = b + frac * (
            cminusb - 0.1666667f * (1.-frac) * (
                (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
            )
        );
    }
    return (w+6);
}

static void sigvd_dsp(t_sigvd *x, t_signal **sp)
{
    t_delwrite_tilde *delwriter =
        (t_delwrite_tilde *)pd_getThingByClass(x->x_sym, sigdelwrite_class);
    x->x_sr = sp[0]->s_sampleRate * 0.001;
    if (delwriter)
    {
        sigdelwrite_checkvecsize(delwriter, sp[0]->s_vectorSize);
        x->x_zerodel = (delwriter->dw_buildIdentifier == ugen_getBuildIdentifier() ?
            0 : delwriter->dw_vectorSize);
        dsp_add(sigvd_perform, 5,
            sp[0]->s_vector, sp[1]->s_vector,
                &delwriter->dw_space, x, sp[0]->s_vectorSize);
    }
    else if (*x->x_sym->s_name)
        post_error ("vd~: %s: no such delwrite~",x->x_sym->s_name);
}

void sigvd_setup(void)
{
    sigvd_class = class_new(sym_vd__tilde__, (t_newmethod)sigvd_new, 0,
        sizeof(t_sigvd), 0, A_DEFSYMBOL, 0);
    //gensym("delread4~")
    class_addMethod(sigvd_class, (t_method)sigvd_dsp, sym_dsp, A_CANT, 0);
    CLASS_SIGNAL(sigvd_class, t_sigvd, x_f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

