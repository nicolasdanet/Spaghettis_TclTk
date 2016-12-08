
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

static t_class *tabread4_class;

typedef struct _tabread4
{
    t_object x_obj;
    t_symbol *x_arrayname;
} t_tabread4;

static void tabread4_float(t_tabread4 *x, t_float f)
{
    t_garray *a;
    int npoints;
    t_word *vec;

    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
        post_error ("%s: no such array", x->x_arrayname->s_name);
    else if (!garray_getData(a, &npoints, &vec)) /* Always true now !!! */
        post_error ("%s: bad template for tabread4", x->x_arrayname->s_name);
    else if (npoints < 4)
        outlet_float(x->x_obj.te_outlet, 0);
    else if (f <= 1)
        outlet_float(x->x_obj.te_outlet, vec[1].w_float);
    else if (f >= npoints - 2)
        outlet_float(x->x_obj.te_outlet, vec[npoints - 2].w_float);
    else
    {
        int n = f;
        float a, b, c, d, cminusb, frac;
        t_word *wp;
        if (n >= npoints - 2)
            n = npoints - 3;
        wp = vec + n;
        frac = f - n;
        a = wp[-1].w_float;
        b = wp[0].w_float;
        c = wp[1].w_float;
        d = wp[2].w_float;
        cminusb = c-b;
        outlet_float(x->x_obj.te_outlet, b + frac * (
            cminusb - 0.1666667f * (1.-frac) * (
                (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b))));
    }
}

static void tabread4_set(t_tabread4 *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *tabread4_new(t_symbol *s)
{
    t_tabread4 *x = (t_tabread4 *)pd_new(tabread4_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_float);
    return x;
}

void tabread4_setup(void)
{
    tabread4_class = class_new(sym_tabread4, (t_newmethod)tabread4_new,
        0, sizeof(t_tabread4), 0, A_DEFSYMBOL, 0);
    class_addFloat(tabread4_class, (t_method)tabread4_float);
    class_addMethod(tabread4_class, (t_method)tabread4_set, sym_set,
        A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
