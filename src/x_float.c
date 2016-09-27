
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;
static t_class *pdfloat_class;

typedef struct _pdfloat
{
    t_object x_obj;
    t_float x_f;
} t_pdfloat;

    /* "float," "symbol," and "bang" are special because
    they're created by short-circuited messages to the "new"
    object which are handled specially in pd_message(). */

/* Called by the t_floatmethod of the object maker class. */

static void *pdfloat_new(t_pd *dummy, t_float f)
{
    t_pdfloat *x = (t_pdfloat *)pd_new(pdfloat_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f);
    pd_newest = &x->x_obj.te_g.g_pd;
    return (x);
}

static void *pdfloat_new2(t_float f)
{
    return (pdfloat_new(0, f));
}

static void pdfloat_bang(t_pdfloat *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f);
}

static void pdfloat_float(t_pdfloat *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, x->x_f = f);
}

static void pdfloat_send(t_pdfloat *x, t_symbol *s)
{
    if (s->s_thing)
        pd_float(s->s_thing, x->x_f);
    else post_error ("%s: no such object", s->s_name);
}

void pdfloat_setup(void)
{
    pdfloat_class = class_new(&s_float, (t_newmethod)pdfloat_new, 0,
        sizeof(t_pdfloat), 0, A_FLOAT, 0);
    class_addCreator((t_newmethod)pdfloat_new2, sym_f, A_DEFFLOAT, 0);
    class_addMethod(pdfloat_class, (t_method)pdfloat_send, sym_send,
        A_SYMBOL, 0);
    class_addBang(pdfloat_class, pdfloat_bang);
    class_addFloat(pdfloat_class, (t_method)pdfloat_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
