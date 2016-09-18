
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

t_class *list_tosymbol_class;

typedef struct _list_tosymbol
{
    t_object x_obj;
} t_list_tosymbol;

void *list_tosymbol_new (void)
{
    t_list_tosymbol *x = (t_list_tosymbol *)pd_new(list_tosymbol_class);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

static void list_tosymbol_list(t_list_tosymbol *x, t_symbol *s,
    int argc, t_atom *argv)
{
    int i;
    char *str = alloca(argc + 1);
    for (i = 0; i < argc; i++)
        str[i] = (char)atom_getFloatAtIndex(i, argc, argv);
    str[argc] = 0;
    outlet_symbol(x->x_obj.te_outlet, gensym (str));
}

void list_tosymbol_setup(void)
{
    list_tosymbol_class = class_new(sym_list__space__tosymbol,
        (t_newmethod)list_tosymbol_new, 0, sizeof(t_list_tosymbol), 0, 0);
    class_addList(list_tosymbol_class, list_tosymbol_list);
    class_setHelpName(list_tosymbol_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

