
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *delwrite_tilde_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *vd_tilde_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vd_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_float     x_samplesPerMilliseconds;
    int         x_masterVectorSize;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_vd_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *vd_tilde_perform(t_int *w)
{
    t_vd_tilde *x = (t_vd_tilde *)(w[1]);
    t_delwrite_tilde_control *ctl = (t_delwrite_tilde_control *)(w[2]);
    t_sample *in = (t_sample *)(w[3]);
    t_sample *out = (t_sample *)(w[4]);
    int n = (int)(w[5]);

    int nsamps = ctl->c_size;
    t_sample limit = nsamps - n;
    t_sample fn = n-1;
    t_sample *vp = ctl->c_vector, *bp, *wp = vp + ctl->c_phase;
    t_sample zerodel = x->x_masterVectorSize;
    while (n--)
    {
        t_sample delsamps = x->x_samplesPerMilliseconds * *in++ - zerodel, frac;
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

static void vd_tilde_dsp (t_vd_tilde *x, t_signal **sp)
{
    t_delwrite_tilde *m = (t_delwrite_tilde *)pd_getThingByClass (x->x_name, delwrite_tilde_class);
    
    x->x_samplesPerMilliseconds = sp[0]->s_sampleRate / 1000.0;
    
    if (!m) { if (x->x_name != &s_) { error_canNotFind (sym_delread__tilde__, x->x_name); } }
    else {
    //
    delwrite_tilde_setVectorSize (m, sp[0]->s_vectorSize);
    
    /* Set master vector size as zero in non-recirculating cases. */
    
    x->x_masterVectorSize = (m->dw_buildIdentifier == ugen_getBuildIdentifier() ? 0 : m->dw_vectorSize);
    
    dsp_add (vd_tilde_perform, 5, x, &m->dw_space, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vd_tilde_new (t_symbol *s)
{
    t_vd_tilde *x = (t_vd_tilde *)pd_new (vd_tilde_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vd_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_vd__tilde__,
            (t_newmethod)vd_tilde_new,
            NULL,
            sizeof (t_vd_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
    
    class_addCreator ((t_newmethod)vd_tilde_new, sym_delread4__tilde__, A_DEFSYMBOL, A_NULL);
    
    CLASS_SIGNAL (c, t_vd_tilde, x_f);
    
    class_addDSP (c, vd_tilde_dsp);
    
    vd_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

