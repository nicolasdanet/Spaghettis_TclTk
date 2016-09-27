
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

static t_class *pdsymbol_class;

typedef struct _pdsymbol
{
    t_object x_obj;
    t_symbol *x_s;
} t_pdsymbol;

/* Called by the t_symbolmethod of the object maker class. */

static void *pdsymbol_new(t_pd *dummy, t_symbol *s)
{
    t_pdsymbol *x = (t_pdsymbol *)pd_new(pdsymbol_class);
    x->x_s = s;
    outlet_new(&x->x_obj, &s_symbol);
    inlet_newSymbol(&x->x_obj, &x->x_s);
    pd_newest = &x->x_obj.te_g.g_pd;
    return (x);
}

static void pdsymbol_bang(t_pdsymbol *x)
{
    outlet_symbol(x->x_obj.te_outlet, x->x_s);
}

static void pdsymbol_symbol(t_pdsymbol *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.te_outlet, x->x_s = s);
}

static void pdsymbol_anything(t_pdsymbol *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_symbol(x->x_obj.te_outlet, x->x_s = s);
}

    /* For "list" message don't just output "list"; if empty, we want to
    bang the symbol and if it starts with a symbol, we output that.
    Otherwise it's not clear what we should do so we just go for the
    "anything" method.  LATER figure out if there are other places where
    empty lists aren't equivalent to "bang"  Should Pd's message passer
    always check and call the more specific method, or should it be the 
    object's responsibility?  Dunno... */
static void pdsymbol_list(t_pdsymbol *x, t_symbol *s, int ac, t_atom *av)
{
    if (!ac)
        pdsymbol_bang(x);
    else if (av->a_type == A_SYMBOL)
        pdsymbol_symbol(x, av->a_w.w_symbol);
    else pdsymbol_anything(x, s, ac, av);
}

void pdsymbol_setup(void)
{
    pdsymbol_class = class_new (&s_symbol, (t_newmethod)pdsymbol_new, 0,
        sizeof(t_pdsymbol), 0, A_SYMBOL, 0);
    class_addBang(pdsymbol_class, pdsymbol_bang);
    class_addSymbol(pdsymbol_class, pdsymbol_symbol);
    class_addAnything(pdsymbol_class, pdsymbol_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
