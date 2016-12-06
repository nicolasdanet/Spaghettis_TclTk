
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
