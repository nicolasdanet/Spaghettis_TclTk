
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

static t_class *tabwrite_class;

typedef struct _tabwrite
{
    t_object x_obj;
    t_symbol *x_arrayname;
    t_float x_ft1;
} t_tabwrite;

static void tabwrite_float(t_tabwrite *x, t_float f)
{
    int i, vecsize;
    t_garray *a;
    t_word *vec;

    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
        post_error ("%s: no such array", x->x_arrayname->s_name);
    else if (!garray_getData(a, &vecsize, &vec)) /* Always true now !!! */
        post_error ("%s: bad template for tabwrite", x->x_arrayname->s_name);
    else
    {
        int n = x->x_ft1;
        if (n < 0)
            n = 0;
        else if (n >= vecsize)
            n = vecsize-1;
        vec[n].w_float = f;
        garray_redraw(a);
    }
}

static void tabwrite_set(t_tabwrite *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *tabwrite_new(t_symbol *s)
{
    t_tabwrite *x = (t_tabwrite *)pd_new(tabwrite_class);
    x->x_ft1 = 0;
    x->x_arrayname = s;
    inlet_newFloat(&x->x_obj, &x->x_ft1);
    return x;
}

void tabwrite_setup(void)
{
    tabwrite_class = class_new(sym_tabwrite, (t_newmethod)tabwrite_new,
        0, sizeof(t_tabwrite), 0, A_DEFSYMBOL, 0);
    class_addFloat(tabwrite_class, (t_method)tabwrite_float);
    class_addMethod(tabwrite_class, (t_method)tabwrite_set, sym_set,
        A_SYMBOL, 0);
}

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
