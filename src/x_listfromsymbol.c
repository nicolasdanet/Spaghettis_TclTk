
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
#include "m_alloca.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;

t_class *list_fromsymbol_class;

typedef struct _list_fromsymbol
{
    t_object x_obj;
} t_list_fromsymbol;

void *list_fromsymbol_new( void)
{
    t_list_fromsymbol *x = (t_list_fromsymbol *)pd_new(list_fromsymbol_class);
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void list_fromsymbol_symbol(t_list_fromsymbol *x, t_symbol *s)
{
    t_atom *outv;
    int n, outc = strlen(s->s_name);
    ATOMS_ALLOCA(outv, outc);
    for (n = 0; n < outc; n++)
        SET_FLOAT(outv + n, (unsigned char)s->s_name[n]);
    outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
    ATOMS_FREEA(outv, outc);
}

void list_fromsymbol_setup(void)
{
    list_fromsymbol_class = class_new(sym_list__space__fromsymbol,
        (t_newmethod)list_fromsymbol_new, 0, sizeof(t_list_fromsymbol), 0, 0);
    class_addSymbol(list_fromsymbol_class, list_fromsymbol_symbol);
    class_setHelpName(list_fromsymbol_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

