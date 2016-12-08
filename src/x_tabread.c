
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
