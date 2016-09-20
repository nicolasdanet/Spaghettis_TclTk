
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

t_class *list_length_class;

typedef struct _list_length
{
    t_object x_obj;
} t_list_length;

void *listlength_new(t_symbol *s, int argc, t_atom *argv)
{
    t_list_length *x = (t_list_length *)pd_new(list_length_class);
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void list_length_list(t_list_length *x, t_symbol *s,
    int argc, t_atom *argv)
{
    outlet_float(x->x_obj.te_outlet, (t_float)argc);
}

static void list_length_anything(t_list_length *x, t_symbol *s,
    int argc, t_atom *argv)
{
    outlet_float(x->x_obj.te_outlet, (t_float)argc+1);
}

void listlength_setup(void)
{
    list_length_class = class_new(sym_list__space__length,
        (t_newmethod)listlength_new, 0,
        sizeof(t_list_length), CLASS_DEFAULT, A_GIMME, 0);
    class_addList(list_length_class, list_length_list);
    class_addAnything(list_length_class, list_length_anything);
    class_setHelpName(list_length_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

