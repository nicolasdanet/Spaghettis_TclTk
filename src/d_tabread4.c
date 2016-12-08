
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_class *garray_class;

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
