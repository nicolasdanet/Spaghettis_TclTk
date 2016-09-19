
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

t_class *list_trim_class;

typedef struct _list_trim
{
    t_object x_obj;
} t_list_trim;

void *listtrim_new(t_symbol *s, int argc, t_atom *argv)
{
    t_list_trim *x = (t_list_trim *)pd_new(list_trim_class);
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void list_trim_list(t_list_trim *x, t_symbol *s,
    int argc, t_atom *argv)
{
    if (argc < 1 || argv[0].a_type != A_SYMBOL)
        outlet_list(x->x_obj.te_outlet, &s_list, argc, argv);
    else outlet_anything(x->x_obj.te_outlet, argv[0].a_w.w_symbol,
        argc-1, argv+1);
}

static void list_trim_anything(t_list_trim *x, t_symbol *s,
    int argc, t_atom *argv)
{
    outlet_anything(x->x_obj.te_outlet, s, argc, argv);
}

void list_trim_setup (void)
{
    list_trim_class = class_new(sym_list__space__trim,
        (t_newmethod)listtrim_new, 0,
        sizeof(t_list_trim), CLASS_DEFAULT, A_GIMME, 0);
    class_addList(list_trim_class, list_trim_list);
    class_addAnything(list_trim_class, list_trim_anything);
    class_setHelpName(list_trim_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

