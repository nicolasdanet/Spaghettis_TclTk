
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

static t_class *tabreceive_class;

typedef struct _tabreceive
{
    t_object x_obj;
    t_word *x_vec;
    t_symbol *x_arrayname;
    int x_npoints;
} t_tabreceive;

static t_int *tabreceive_perform(t_int *w)
{
    t_tabreceive *x = (t_tabreceive *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = w[3];
    t_word *from = x->x_vec;
    if (from)
    {
        int vecsize = x->x_npoints;
        if (vecsize > n)
            vecsize = n;
        while (vecsize--)
            *out++ = (from++)->w_float;
        vecsize = n - x->x_npoints;
        if (vecsize > 0)
            while (vecsize--)
                *out++ = 0;
    }
    else while (n--) *out++ = 0;
    return (w+4);
}

static void tabreceive_set(t_tabreceive *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_getThingByClass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            post_error ("tabreceive~: %s: no such array",
                x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getData(a, &x->x_npoints, &x->x_vec)) /* Always true now !!! */
    {
        post_error ("%s: bad template for tabreceive~",
            x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_setAsUsedInDSP(a);
}

static void tabreceive_dsp(t_tabreceive *x, t_signal **sp)
{
    tabreceive_set(x, x->x_arrayname);
    dsp_add(tabreceive_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

static void *tabreceive_new(t_symbol *s)
{
    t_tabreceive *x = (t_tabreceive *)pd_new(tabreceive_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_signal);
    return x;
}

void tabreceive_setup(void)
{
    tabreceive_class = class_new(sym_tabreceive__tilde__,
        (t_newmethod)tabreceive_new, 0,
        sizeof(t_tabreceive), 0, A_DEFSYMBOL, 0);
    class_addMethod(tabreceive_class, (t_method)tabreceive_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(tabreceive_class, (t_method)tabreceive_set,
        sym_set, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
