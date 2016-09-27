
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

/* -------------------------- int ------------------------------ */
static t_class *pdint_class;

typedef struct _pdint
{
    t_object x_obj;
    t_float x_f;
} t_pdint;

static void *pdint_new(t_float f)
{
    t_pdint *x = (t_pdint *)pd_new(pdint_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f);
    return (x);
}

static void pdint_bang(t_pdint *x)
{
    outlet_float(x->x_obj.te_outlet, (t_float)(int)(x->x_f));
}

static void pdint_float(t_pdint *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (t_float)(int)(x->x_f = f));
}

static void pdint_send(t_pdint *x, t_symbol *s)
{
    if (s->s_thing)
        pd_float(s->s_thing, (t_float)(int)x->x_f);
    else post_error ("%s: no such object", s->s_name);
}

void pdint_setup(void)
{
    pdint_class = class_new (sym_int, (t_newmethod)pdint_new, 0,
        sizeof(t_pdint), 0, A_DEFFLOAT, 0);
    class_addCreator((t_newmethod)pdint_new, sym_i, A_DEFFLOAT, 0);
    class_addMethod(pdint_class, (t_method)pdint_send, sym_send,
        A_SYMBOL, 0);
    class_addBang(pdint_class, pdint_bang);
    class_addFloat(pdint_class, pdint_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
