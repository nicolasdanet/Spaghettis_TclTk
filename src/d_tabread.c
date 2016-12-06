
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

static t_class *tabread_class;

typedef struct _tabread
{
    t_object x_obj;
    t_symbol *x_arrayname;
} t_tabread;

static void tabread_float(t_tabread *x, t_float f)
{
    t_garray *a;
    int npoints;
    t_word *vec;

    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
        post_error ("%s: no such array", x->x_arrayname->s_name);
    else if (!garray_getData(a, &npoints, &vec)) /* Always true now !!! */
        post_error ("%s: bad template for tabread", x->x_arrayname->s_name);
    else
    {
        int n = f;
        if (n < 0) n = 0;
        else if (n >= npoints) n = npoints - 1;
        outlet_float(x->x_obj.te_outlet, (npoints ? vec[n].w_float : 0));
    }
}

static void tabread_set(t_tabread *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *tabread_new(t_symbol *s)
{
    t_tabread *x = (t_tabread *)pd_new(tabread_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_float);
    return x;
}

void tabread_setup(void)
{
    tabread_class = class_new(sym_tabread, (t_newmethod)tabread_new,
        0, sizeof(t_tabread), 0, A_DEFSYMBOL, 0);
    class_addFloat(tabread_class, (t_method)tabread_float);
    class_addMethod(tabread_class, (t_method)tabread_set, sym_set,
        A_SYMBOL, 0);
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
