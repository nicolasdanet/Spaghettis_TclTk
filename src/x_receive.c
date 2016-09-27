
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

static t_class *receive_class;

typedef struct _receive
{
    t_object x_obj;
    t_symbol *x_sym;
} t_receive;

static void receive_bang(t_receive *x)
{
    outlet_bang(x->x_obj.te_outlet);
}

static void receive_float(t_receive *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, f);
}

static void receive_symbol(t_receive *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.te_outlet, s);
}

static void receive_pointer(t_receive *x, t_gpointer *gp)
{
    outlet_pointer(x->x_obj.te_outlet, gp);
}

static void receive_list(t_receive *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_obj.te_outlet, argc, argv);
}

static void receive_anything(t_receive *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything(x->x_obj.te_outlet, s, argc, argv);
}

static void *receive_new(t_symbol *s)
{
    t_receive *x = (t_receive *)pd_new(receive_class);
    x->x_sym = s;
    pd_bind(&x->x_obj.te_g.g_pd, s);
    outlet_new(&x->x_obj, 0);
    return (x);
}

static void receive_free(t_receive *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_sym);
}

void receive_setup(void)
{
    receive_class = class_new(sym_receive, (t_newmethod)receive_new, 
        (t_method)receive_free, sizeof(t_receive), CLASS_NOINLET, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)receive_new, sym_r, A_DEFSYMBOL, 0);
    class_addBang(receive_class, receive_bang);
    class_addFloat(receive_class, (t_method)receive_float);
    class_addSymbol(receive_class, receive_symbol);
    class_addPointer(receive_class, receive_pointer);
    class_addList(receive_class, receive_list);
    class_addAnything(receive_class, receive_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
