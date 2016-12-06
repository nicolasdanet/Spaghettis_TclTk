
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
#include "g_graphics.h"
#include "d_dsp.h"

extern t_class *garray_class;

/* ------------------------- tabwrite~ -------------------------- */

static t_class *tabwrite_tilde_class;

typedef struct _tabwrite_tilde
{
    t_object x_obj;
    int x_phase;
    int x_nsampsintab;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
} t_tabwrite_tilde;

static void tabwrite_tilde_tick(t_tabwrite_tilde *x);

static void *tabwrite_tilde_new(t_symbol *s)
{
    t_tabwrite_tilde *x = (t_tabwrite_tilde *)pd_new(tabwrite_tilde_class);
    x->x_phase = PD_INT_MAX;
    x->x_arrayname = s;
    x->x_f = 0;
    return x;
}

static void tabwrite_tilde_redraw(t_tabwrite_tilde *x)
{
    t_garray *a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class);
    if (!a) { PD_BUG; }
    else garray_redraw(a);
}

static t_int *tabwrite_tilde_perform(t_int *w)
{
    t_tabwrite_tilde *x = (t_tabwrite_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]), phase = x->x_phase, endphase = x->x_nsampsintab;
    if (!x->x_vec) goto bad;
    
    if (endphase > phase)
    {
        int nxfer = endphase - phase;
        t_word *wp = x->x_vec + phase;
        if (nxfer > n) nxfer = n;
        phase += nxfer;
        while (nxfer--)
        {
            t_sample f = *in++;
            if (PD_BIG_OR_SMALL(f))
                f = 0;
            (wp++)->w_float = f;
        }
        if (phase >= endphase)
        {
            tabwrite_tilde_redraw(x);
            phase = PD_INT_MAX;
        }
        x->x_phase = phase;
    }
    else x->x_phase = PD_INT_MAX;
bad:
    return (w+4);
}

static void tabwrite_tilde_set(t_tabwrite_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name) post_error ("tabwrite~: %s: no such array",
            x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getData(a, &x->x_nsampsintab, &x->x_vec))  /* Always true now !!! */
    {
        post_error ("%s: bad template for tabwrite~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_setAsUsedInDSP(a);
}

static void tabwrite_tilde_dsp(t_tabwrite_tilde *x, t_signal **sp)
{
    tabwrite_tilde_set(x, x->x_arrayname);
    dsp_add(tabwrite_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

static void tabwrite_tilde_bang(t_tabwrite_tilde *x)
{
    x->x_phase = 0;
}

static void tabwrite_tilde_start(t_tabwrite_tilde *x, t_float f)
{
    x->x_phase = (f > 0 ? f : 0);
}

static void tabwrite_tilde_stop(t_tabwrite_tilde *x)
{
    if (x->x_phase != PD_INT_MAX)
    {
        tabwrite_tilde_redraw(x);
        x->x_phase = PD_INT_MAX;
    }
}

void tabwrite_tilde_setup(void)
{
    tabwrite_tilde_class = class_new(sym_tabwrite__tilde__,
        (t_newmethod)tabwrite_tilde_new, 0,
        sizeof(t_tabwrite_tilde), 0, A_DEFSYMBOL, 0);
    CLASS_SIGNAL(tabwrite_tilde_class, t_tabwrite_tilde, x_f);
    class_addMethod(tabwrite_tilde_class, (t_method)tabwrite_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(tabwrite_tilde_class, (t_method)tabwrite_tilde_set,
        sym_set, A_SYMBOL, 0);
    class_addMethod(tabwrite_tilde_class, (t_method)tabwrite_tilde_stop,
        sym_stop, 0);
    class_addMethod(tabwrite_tilde_class, (t_method)tabwrite_tilde_start,
        sym_start, A_DEFFLOAT, 0);
    class_addBang(tabwrite_tilde_class, tabwrite_tilde_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ------------ tabplay~ - non-transposing sample playback --------------- */

static t_class *tabplay_tilde_class;

typedef struct _tabplay_tilde
{
    t_object x_obj;
    t_outlet *x_bangout;
    int x_phase;
    int x_nsampsintab;
    int x_limit;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_clock *x_clock;
} t_tabplay_tilde;

static void tabplay_tilde_tick(t_tabplay_tilde *x);

static void *tabplay_tilde_new(t_symbol *s)
{
    t_tabplay_tilde *x = (t_tabplay_tilde *)pd_new(tabplay_tilde_class);
    x->x_clock = clock_new(x, (t_method)tabplay_tilde_tick);
    x->x_phase = PD_INT_MAX;
    x->x_limit = 0;
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_signal);
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    return x;
}

static t_int *tabplay_tilde_perform(t_int *w)
{
    t_tabplay_tilde *x = (t_tabplay_tilde *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_word *wp;
    int n = (int)(w[3]), phase = x->x_phase,
        endphase = (x->x_nsampsintab < x->x_limit ?
            x->x_nsampsintab : x->x_limit), nxfer, n3;
    if (!x->x_vec || phase >= endphase)
        goto zero;
    
    nxfer = endphase - phase;
    wp = x->x_vec + phase;
    if (nxfer > n)
        nxfer = n;
    n3 = n - nxfer;
    phase += nxfer;
    while (nxfer--)
        *out++ = (wp++)->w_float;
    if (phase >= endphase)
    {
        clock_delay(x->x_clock, 0);
        x->x_phase = PD_INT_MAX;
        while (n3--)
            *out++ = 0;
    }
    else x->x_phase = phase;
    
    return (w+4);
zero:
    while (n--) *out++ = 0;
    return (w+4);
}

static void tabplay_tilde_set(t_tabplay_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name) post_error ("tabplay~: %s: no such array",
            x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getData(a, &x->x_nsampsintab, &x->x_vec)) /* Always true now !!! */
    {
        post_error ("%s: bad template for tabplay~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_setAsUsedInDSP(a);
}

static void tabplay_tilde_dsp(t_tabplay_tilde *x, t_signal **sp)
{
    tabplay_tilde_set(x, x->x_arrayname);
    dsp_add(tabplay_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

static void tabplay_tilde_list(t_tabplay_tilde *x, t_symbol *s,
    int argc, t_atom *argv)
{
    long start = atom_getFloatAtIndex(0, argc, argv);
    long length = atom_getFloatAtIndex(1, argc, argv);
    if (start < 0) start = 0;
    if (length <= 0)
        x->x_limit = PD_INT_MAX;
    else
        x->x_limit = start + length;
    x->x_phase = start;
}

static void tabplay_tilde_stop(t_tabplay_tilde *x)
{
    x->x_phase = PD_INT_MAX;
}

static void tabplay_tilde_tick(t_tabplay_tilde *x)
{
    outlet_bang(x->x_bangout);
}

static void tabplay_tilde_free(t_tabplay_tilde *x)
{
    clock_free(x->x_clock);
}

void tabplay_tilde_setup(void)
{
    tabplay_tilde_class = class_new(sym_tabplay__tilde__,
        (t_newmethod)tabplay_tilde_new, (t_method)tabplay_tilde_free,
        sizeof(t_tabplay_tilde), 0, A_DEFSYMBOL, 0);
    class_addMethod(tabplay_tilde_class, (t_method)tabplay_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(tabplay_tilde_class, (t_method)tabplay_tilde_stop,
        sym_stop, 0);
    class_addMethod(tabplay_tilde_class, (t_method)tabplay_tilde_set,
        sym_set, A_DEFSYMBOL, 0);
    class_addList(tabplay_tilde_class, tabplay_tilde_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *tabread_tilde_class;

typedef struct _tabread_tilde
{
    t_object x_obj;
    int x_npoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
} t_tabread_tilde;

static void *tabread_tilde_new(t_symbol *s)
{
    t_tabread_tilde *x = (t_tabread_tilde *)pd_new(tabread_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return x;
}

static t_int *tabread_tilde_perform(t_int *w)
{
    t_tabread_tilde *x = (t_tabread_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);    
    int maxindex;
    t_word *buf = x->x_vec;
    int i;
    
    maxindex = x->x_npoints - 1;
    if(maxindex<0) goto zero;
    if (!buf) goto zero;

    for (i = 0; i < n; i++)
    {
        int index = *in++;
        if (index < 0)
            index = 0;
        else if (index > maxindex)
            index = maxindex;
        *out++ = buf[index].w_float;
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

static void tabread_tilde_set(t_tabread_tilde *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            post_error ("tabread~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getData(a, &x->x_npoints, &x->x_vec)) /* Always true now !!! */
    {
        post_error ("%s: bad template for tabread~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_setAsUsedInDSP(a);
}

static void tabread_tilde_dsp(t_tabread_tilde *x, t_signal **sp)
{
    tabread_tilde_set(x, x->x_arrayname);

    dsp_add(tabread_tilde_perform, 4, x,
        sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);

}

static void tabread_tilde_free(t_tabread_tilde *x)
{
}

void tabread_tilde_setup(void)
{
    tabread_tilde_class = class_new(sym_tabread__tilde__,
        (t_newmethod)tabread_tilde_new, (t_method)tabread_tilde_free,
        sizeof(t_tabread_tilde), 0, A_DEFSYMBOL, 0);
    CLASS_SIGNAL(tabread_tilde_class, t_tabread_tilde, x_f);
    class_addMethod(tabread_tilde_class, (t_method)tabread_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(tabread_tilde_class, (t_method)tabread_tilde_set,
        sym_set, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *tabread4_tilde_class;

typedef struct _tabread4_tilde
{
    t_object x_obj;
    int x_npoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
    t_float x_onset;
} t_tabread4_tilde;

static void *tabread4_tilde_new(t_symbol *s)
{
    t_tabread4_tilde *x = (t_tabread4_tilde *)pd_new(tabread4_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, &s_signal);
    inlet_newFloat(&x->x_obj, &x->x_onset);
    x->x_f = 0;
    x->x_onset = 0;
    return x;
}

static t_int *tabread4_tilde_perform(t_int *w)
{
    t_tabread4_tilde *x = (t_tabread4_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);    
    int maxindex;
    t_word *buf = x->x_vec, *wp;
    double onset = x->x_onset;
    int i;
    
    maxindex = x->x_npoints - 3;
    if(maxindex<0) goto zero;

    if (!buf) goto zero;

#if 0       /* test for spam -- I'm not ready to deal with this */
    for (i = 0,  xmax = 0, xmin = maxindex,  fp = in1; i < n; i++,  fp++)
    {
        t_sample f = *in1;
        if (f < xmin) xmin = f;
        else if (f > xmax) xmax = f;
    }
    if (xmax < xmin + x->c_maxextent) xmax = xmin + x->c_maxextent;
    for (i = 0, splitlo = xmin+ x->c_maxextent, splithi = xmax - x->c_maxextent,
        fp = in1; i < n; i++,  fp++)
    {
        t_sample f = *in1;
        if (f > splitlo && f < splithi) goto zero;
    }
#endif

    for (i = 0; i < n; i++)
    {
        double findex = *in++ + onset;
        int index = findex;
        t_sample frac,  a,  b,  c,  d, cminusb;
        static int count;
        if (index < 1)
            index = 1, frac = 0;
        else if (index > maxindex)
            index = maxindex, frac = 1;
        else frac = findex - index;
        wp = buf + index;
        a = wp[-1].w_float;
        b = wp[0].w_float;
        c = wp[1].w_float;
        d = wp[2].w_float;
        cminusb = c-b;
        *out++ = b + frac * (
            cminusb - 0.1666667f * (1.-frac) * (
                (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
            )
        );
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

static void tabread4_tilde_set(t_tabread4_tilde *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            post_error ("tabread4~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getData(a, &x->x_npoints, &x->x_vec)) /* Always true now !!! */
    {
        post_error ("%s: bad template for tabread4~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_setAsUsedInDSP(a);
}

static void tabread4_tilde_dsp(t_tabread4_tilde *x, t_signal **sp)
{
    tabread4_tilde_set(x, x->x_arrayname);

    dsp_add(tabread4_tilde_perform, 4, x,
        sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);

}

static void tabread4_tilde_free(t_tabread4_tilde *x)
{
}

void tabread4_tilde_setup(void)
{
    tabread4_tilde_class = class_new(sym_tabread4__tilde__,
        (t_newmethod)tabread4_tilde_new, (t_method)tabread4_tilde_free,
        sizeof(t_tabread4_tilde), 0, A_DEFSYMBOL, 0);
    CLASS_SIGNAL(tabread4_tilde_class, t_tabread4_tilde, x_f);
    class_addMethod(tabread4_tilde_class, (t_method)tabread4_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(tabread4_tilde_class, (t_method)tabread4_tilde_set,
        sym_set, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
