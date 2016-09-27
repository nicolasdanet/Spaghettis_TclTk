
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

static t_class *trigger_class;
#define TR_BANG 0
#define TR_FLOAT 1
#define TR_SYMBOL 2
#define TR_POINTER 3
#define TR_LIST 4
#define TR_ANYTHING 5

typedef struct triggerout
{
    int u_type;         /* outlet type from above */
    t_outlet *u_outlet;
} t_triggerout;

typedef struct _trigger
{
    t_object x_obj;
    t_int x_n;
    t_triggerout *x_vec;
} t_trigger;

static void *trigger_new(t_symbol *s, int argc, t_atom *argv)
{
    t_trigger *x = (t_trigger *)pd_new(trigger_class);
    t_atom defarg[2], *ap;
    t_triggerout *u;
    int i;
    if (!argc)
    {
        argv = defarg;
        argc = 2;
        SET_SYMBOL(&defarg[0], &s_bang);
        SET_SYMBOL(&defarg[1], &s_bang);
    }
    x->x_n = argc;
    x->x_vec = (t_triggerout *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
    for (i = 0, ap = argv, u = x->x_vec; i < argc; u++, ap++, i++)
    {
        t_atomtype thistype = ap->a_type;
        char c;
        if (thistype == TR_SYMBOL) c = ap->a_w.w_symbol->s_name[0];
        else if (thistype == TR_FLOAT) c = 'f';
        else c = 0;
        if (c == 'p')
            u->u_type = TR_POINTER,
                u->u_outlet = outlet_new(&x->x_obj, &s_pointer);
        else if (c == 'f')
            u->u_type = TR_FLOAT, u->u_outlet = outlet_new(&x->x_obj, &s_float);
        else if (c == 'b')
            u->u_type = TR_BANG, u->u_outlet = outlet_new(&x->x_obj, &s_bang);
        else if (c == 'l')
            u->u_type = TR_LIST, u->u_outlet = outlet_new(&x->x_obj, &s_list);
        else if (c == 's')
            u->u_type = TR_SYMBOL,
                u->u_outlet = outlet_new(&x->x_obj, &s_symbol);
        else if (c == 'a')
            u->u_type = TR_ANYTHING,
                u->u_outlet = outlet_new(&x->x_obj, &s_symbol);
        else
        {
            post_error ("trigger: %s: bad type", ap->a_w.w_symbol->s_name);
            u->u_type = TR_FLOAT, u->u_outlet = outlet_new(&x->x_obj, &s_float);
        }
    }
    return (x);
}

static void trigger_list(t_trigger *x, t_symbol *s, int argc, t_atom *argv)
{
    t_triggerout *u;
    int i;
    for (i = x->x_n, u = x->x_vec + i; u--, i--;)
    {
        if (u->u_type == TR_FLOAT)
            outlet_float(u->u_outlet, (argc ? atom_getFloat(argv) : 0));
        else if (u->u_type == TR_BANG)
            outlet_bang(u->u_outlet);
        else if (u->u_type == TR_SYMBOL)
            outlet_symbol(u->u_outlet,
                (argc ? atom_getSymbol(argv) : &s_symbol));
        else if (u->u_type == TR_POINTER)
        {
            if (!argc || argv->a_type != TR_POINTER)
                post_error ("unpack: bad pointer");
            else outlet_pointer(u->u_outlet, argv->a_w.w_gpointer);
        }
        else outlet_list(u->u_outlet, argc, argv);
    }
}

static void trigger_anything(t_trigger *x, t_symbol *s, int argc, t_atom *argv)
{
    t_triggerout *u;
    int i;
    for (i = x->x_n, u = x->x_vec + i; u--, i--;)
    {
        if (u->u_type == TR_BANG)
            outlet_bang(u->u_outlet);
        else if (u->u_type == TR_ANYTHING)
            outlet_anything(u->u_outlet, s, argc, argv);
        else post_error ("trigger: can only convert 's' to 'b' or 'a'");
    }
}

static void trigger_bang(t_trigger *x)
{
    trigger_list(x, 0, 0, 0);
}

static void trigger_pointer(t_trigger *x, t_gpointer *gp)
{
    t_atom at;
    SET_POINTER(&at, gp);
    trigger_list(x, 0, 1, &at);
}

static void trigger_float(t_trigger *x, t_float f)
{
    t_atom at;
    SET_FLOAT(&at, f);
    trigger_list(x, 0, 1, &at);
}

static void trigger_symbol(t_trigger *x, t_symbol *s)
{
    t_atom at;
    SET_SYMBOL(&at, s);
    trigger_list(x, 0, 1, &at);
}

static void trigger_free(t_trigger *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

void trigger_setup(void)
{
    trigger_class = class_new(sym_trigger, (t_newmethod)trigger_new,
        (t_method)trigger_free, sizeof(t_trigger), 0, A_GIMME, 0);
    class_addCreator((t_newmethod)trigger_new, sym_t, A_GIMME, 0);
    class_addList(trigger_class, trigger_list);
    class_addBang(trigger_class, trigger_bang);
    class_addPointer(trigger_class, trigger_pointer);
    class_addFloat(trigger_class, (t_method)trigger_float);
    class_addSymbol(trigger_class, trigger_symbol);
    class_addAnything(trigger_class, trigger_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
