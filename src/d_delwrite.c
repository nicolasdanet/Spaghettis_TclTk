
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

static int delread_zero = 0;    /* four bytes of zero for delread~, vd~ */

/* ----------------------------- delwrite~ ----------------------------- */
t_class *sigdelwrite_class;

void sigdelwrite_updatesr (t_delwrite_tilde *x, t_float sr) /* added by  */
{
    int nsamps = x->dw_delayTime * sr * (t_float)(0.001f);
    if (nsamps < 1) nsamps = 1;
    nsamps += ((- nsamps) & (DELAY_BLANK_SAMPLES - 1));
    nsamps += DELAY_BLOCK_SIZE;
    if (x->dw_space.c_size != nsamps) {
      x->dw_space.c_vector = (t_sample *)PD_MEMORY_RESIZE(x->dw_space.c_vector,
        (x->dw_space.c_size + DELAY_EXTRA_SAMPLES) * sizeof(t_sample),
        (         nsamps + DELAY_EXTRA_SAMPLES) * sizeof(t_sample));
      x->dw_space.c_size = nsamps;
      x->dw_space.c_phase = DELAY_EXTRA_SAMPLES;
    }
}

    /* routine to check that all delwrites/delreads/vds have same vecsize */
void sigdelwrite_checkvecsize(t_delwrite_tilde *x, int vecsize)
{
    if (x->dw_buildIdentifierCheck != ugen_getBuildIdentifier())
    {
        x->dw_vectorSize = vecsize;
        x->dw_buildIdentifierCheck = ugen_getBuildIdentifier();
    }
    /*
        LATER this should really check sample rate and blocking, once that is
        supported.  Probably we don't actually care about vecsize.
        For now just suppress this check. */
#if 0
    else if (vecsize != x->x_vecsize)
        post_error ("delread/delwrite/vd vector size mismatch");
#endif
}

static void *sigdelwrite_new(t_symbol *s, t_float msec)
{
    t_delwrite_tilde *x = (t_delwrite_tilde *)pd_new(sigdelwrite_class);
    if (!*s->s_name) s = sym_delwrite__tilde__;
    pd_bind(cast_pd (x), s);
    x->dw_name = s;
    x->dw_delayTime = msec;
    x->dw_space.c_size = 0;
    x->dw_space.c_vector = PD_MEMORY_GET(DELAY_EXTRA_SAMPLES * sizeof(t_sample));

    return x;
}

static t_int *sigdelwrite_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_delwrite_tilde_control *c = (t_delwrite_tilde_control *)(w[2]);
    int n = (int)(w[3]);
    int phase = c->c_phase, nsamps = c->c_size;
    t_sample *vp = c->c_vector, *bp = vp + phase, *ep = vp + (c->c_size + DELAY_EXTRA_SAMPLES);
    phase += n;

    while (n--)
    {
        t_sample f = *in++;
        if (PD_BIG_OR_SMALL(f))
            f = 0;
        *bp++ = f;
        if (bp == ep)
        {
            vp[0] = ep[-4];
            vp[1] = ep[-3];
            vp[2] = ep[-2];
            vp[3] = ep[-1];
            bp = vp + DELAY_EXTRA_SAMPLES;
            phase -= nsamps;
        }
    }
    c->c_phase = phase; 
    return (w+4);
}

static void sigdelwrite_dsp(t_delwrite_tilde *x, t_signal **sp)
{
    dsp_add(sigdelwrite_perform, 3, sp[0]->s_vector, &x->dw_space, sp[0]->s_vectorSize);
    x->dw_buildIdentifier = ugen_getBuildIdentifier();
    sigdelwrite_checkvecsize(x, sp[0]->s_vectorSize);
    sigdelwrite_updatesr(x, sp[0]->s_sampleRate);
}

static void sigdelwrite_free(t_delwrite_tilde *x)
{
    pd_unbind (cast_pd (x), x->dw_name);
    PD_MEMORY_FREE(x->dw_space.c_vector);
}

void sigdelwrite_setup(void)
{
    sigdelwrite_class = class_new(sym_delwrite__tilde__, 
        (t_newmethod)sigdelwrite_new, (t_method)sigdelwrite_free,
        sizeof(t_delwrite_tilde), 0, A_DEFSYMBOL, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigdelwrite_class, t_delwrite_tilde, dw_f);
    class_addMethod(sigdelwrite_class, (t_method)sigdelwrite_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

