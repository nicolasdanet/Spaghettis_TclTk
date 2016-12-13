
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

#define XTRASAMPS 4
#define SAMPBLK 4
#define DEFDELVS 64             /* LATER get this from canvas at DSP time */

static void sigdelwrite_updatesr (t_sigdelwrite *x, t_float sr) /* added by Mathieu Bouchard */
{
    int nsamps = x->x_deltime * sr * (t_float)(0.001f);
    if (nsamps < 1) nsamps = 1;
    nsamps += ((- nsamps) & (SAMPBLK - 1));
    nsamps += DEFDELVS;
    if (x->x_cspace.c_n != nsamps) {
      x->x_cspace.c_vec = (t_sample *)PD_MEMORY_RESIZE(x->x_cspace.c_vec,
        (x->x_cspace.c_n + XTRASAMPS) * sizeof(t_sample),
        (         nsamps + XTRASAMPS) * sizeof(t_sample));
      x->x_cspace.c_n = nsamps;
      x->x_cspace.c_phase = XTRASAMPS;
    }
}

    /* routine to check that all delwrites/delreads/vds have same vecsize */
void sigdelwrite_checkvecsize(t_sigdelwrite *x, int vecsize)
{
    if (x->x_rsortno != ugen_getBuildIdentifier())
    {
        x->x_vecsize = vecsize;
        x->x_rsortno = ugen_getBuildIdentifier();
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
    t_sigdelwrite *x = (t_sigdelwrite *)pd_new(sigdelwrite_class);
    if (!*s->s_name) s = sym_delwrite__tilde__;
    pd_bind(&x->x_obj.te_g.g_pd, s);
    x->x_sym = s;
    x->x_deltime = msec;
    x->x_cspace.c_n = 0;
    x->x_cspace.c_vec = PD_MEMORY_GET(XTRASAMPS * sizeof(t_sample));
    x->x_sortno = 0;
    x->x_vecsize = 0;
    x->x_f = 0;
    return x;
}

static t_int *sigdelwrite_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_delwritectl *c = (t_delwritectl *)(w[2]);
    int n = (int)(w[3]);
    int phase = c->c_phase, nsamps = c->c_n;
    t_sample *vp = c->c_vec, *bp = vp + phase, *ep = vp + (c->c_n + XTRASAMPS);
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
            bp = vp + XTRASAMPS;
            phase -= nsamps;
        }
    }
    c->c_phase = phase; 
    return (w+4);
}

static void sigdelwrite_dsp(t_sigdelwrite *x, t_signal **sp)
{
    dsp_add(sigdelwrite_perform, 3, sp[0]->s_vector, &x->x_cspace, sp[0]->s_vectorSize);
    x->x_sortno = ugen_getBuildIdentifier();
    sigdelwrite_checkvecsize(x, sp[0]->s_vectorSize);
    sigdelwrite_updatesr(x, sp[0]->s_sampleRate);
}

static void sigdelwrite_free(t_sigdelwrite *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_sym);
    PD_MEMORY_FREE(x->x_cspace.c_vec);
}

void sigdelwrite_setup(void)
{
    sigdelwrite_class = class_new(sym_delwrite__tilde__, 
        (t_newmethod)sigdelwrite_new, (t_method)sigdelwrite_free,
        sizeof(t_sigdelwrite), 0, A_DEFSYMBOL, A_DEFFLOAT, 0);
    CLASS_SIGNAL(sigdelwrite_class, t_sigdelwrite, x_f);
    class_addMethod(sigdelwrite_class, (t_method)sigdelwrite_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    t_sigdelwrite *delwriter =
        (t_sigdelwrite *)pd_getThingByClass(x->x_sym, sigdelwrite_class);
    x->x_deltime = f;
    if (delwriter)
    {
        int delsize = delwriter->x_cspace.c_n;
        x->x_delsamps = (int)(0.5 + x->x_sr * x->x_deltime)
            + x->x_n - x->x_zerodel;
        if (x->x_delsamps < x->x_n) x->x_delsamps = x->x_n;
        else if (x->x_delsamps > delwriter->x_cspace.c_n)
            x->x_delsamps = delwriter->x_cspace.c_n;
    }
}

static t_int *sigdelread_perform(t_int *w)
{
    t_sample *out = (t_sample *)(w[1]);
    t_delwritectl *c = (t_delwritectl *)(w[2]);
    int delsamps = *(int *)(w[3]);
    int n = (int)(w[4]);
    int phase = c->c_phase - delsamps, nsamps = c->c_n;
    t_sample *vp = c->c_vec, *bp, *ep = vp + (c->c_n + XTRASAMPS);
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
    t_sigdelwrite *delwriter =
        (t_sigdelwrite *)pd_getThingByClass(x->x_sym, sigdelwrite_class);
    x->x_sr = sp[0]->s_sampleRate * 0.001;
    x->x_n = sp[0]->s_vectorSize;
    if (delwriter)
    {
        sigdelwrite_updatesr(delwriter, sp[0]->s_sampleRate);
        sigdelwrite_checkvecsize(delwriter, sp[0]->s_vectorSize);
        x->x_zerodel = (delwriter->x_sortno == ugen_getBuildIdentifier() ?
            0 : delwriter->x_vecsize);
        sigdelread_float(x, x->x_deltime);
        dsp_add(sigdelread_perform, 4,
            sp[0]->s_vector, &delwriter->x_cspace, &x->x_delsamps, sp[0]->s_vectorSize);
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

