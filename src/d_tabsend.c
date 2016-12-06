
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

static t_class *tabsend_class;

typedef struct _tabsend
{
    t_object x_obj;
    t_word *x_vec;
    int x_graphperiod;
    int x_graphcount;
    t_symbol *x_arrayname;
    t_float x_f;
    int x_npoints;
} t_tabsend;

static void tabsend_tick(t_tabsend *x);

static void *tabsend_new(t_symbol *s)
{
    t_tabsend *x = (t_tabsend *)pd_new(tabsend_class);
    x->x_graphcount = 0;
    x->x_arrayname = s;
    x->x_f = 0;
    return x;
}

static t_int *tabsend_perform(t_int *w)
{
    t_tabsend *x = (t_tabsend *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = w[3];
    t_word *dest = x->x_vec;
    int i = x->x_graphcount;
    if (!x->x_vec) goto bad;
    if (n > x->x_npoints)
        n = x->x_npoints;
    while (n--)
    {   
        t_sample f = *in++;
        if (PD_BIG_OR_SMALL(f))
            f = 0;
         (dest++)->w_float = f;
    }
    if (!i--)
    {
        t_garray *a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class);
        if (!a) { PD_BUG; }
        else garray_redraw(a);
        i = x->x_graphperiod;
    }
    x->x_graphcount = i;
bad:
    return (w+4);
}

static void tabsend_set(t_tabsend *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            post_error ("tabsend~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getData(a, &x->x_npoints, &x->x_vec)) /* Always true now !!! */
    {
        post_error ("%s: bad template for tabsend~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_setAsUsedInDSP(a);
}

static void tabsend_dsp(t_tabsend *x, t_signal **sp)
{
    int i, vecsize;
    int n = sp[0]->s_vectorSize;
    int ticksper = sp[0]->s_sampleRate/n;
    tabsend_set(x, x->x_arrayname);
    if (ticksper < 1) ticksper = 1;
    x->x_graphperiod = ticksper;
    if (x->x_graphcount > ticksper) x->x_graphcount = ticksper;
    dsp_add(tabsend_perform, 3, x, sp[0]->s_vector, n);
}

void tabsend_setup(void)
{
    tabsend_class = class_new(sym_tabsend__tilde__, (t_newmethod)tabsend_new,
        0, sizeof(t_tabsend), 0, A_DEFSYMBOL, 0);
    CLASS_SIGNAL(tabsend_class, t_tabsend, x_f);
    class_addMethod(tabsend_class, (t_method)tabsend_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(tabsend_class, (t_method)tabsend_set,
        sym_set, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
