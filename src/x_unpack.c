
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

static t_class *unpack_class;

typedef struct unpackout
{
    t_atomtype u_type;
    t_outlet *u_outlet;
} t_unpackout;

typedef struct _unpack
{
    t_object x_obj;
    t_int x_n;
    t_unpackout *x_vec;
} t_unpack;

static void *unpack_new(t_symbol *s, int argc, t_atom *argv)
{
    t_unpack *x = (t_unpack *)pd_new(unpack_class);
    t_atom defarg[2], *ap;
    t_unpackout *u;
    int i;
    if (!argc)
    {
        argv = defarg;
        argc = 2;
        SET_FLOAT(&defarg[0], 0);
        SET_FLOAT(&defarg[1], 0);
    }
    x->x_n = argc;
    x->x_vec = (t_unpackout *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
    for (i = 0, ap = argv, u = x->x_vec; i < argc; u++, ap++, i++)
    {
        t_atomtype type = ap->a_type;
        if (type == A_SYMBOL)
        {
            char c = *ap->a_w.w_symbol->s_name;
            if (c == 's')
            {
                u->u_type = A_SYMBOL;
                u->u_outlet = outlet_new(&x->x_obj, &s_symbol);
            }
            else if (c == 'p')
            {
                u->u_type =  A_POINTER;
                u->u_outlet = outlet_new(&x->x_obj, &s_pointer);
            }
            else
            {
                if (c != 'f') post_error ("unpack: %s: bad type",
                    ap->a_w.w_symbol->s_name);
                u->u_type = A_FLOAT;
                u->u_outlet = outlet_new(&x->x_obj, &s_float);
            }
        }
        else
        {
            u->u_type =  A_FLOAT;
            u->u_outlet = outlet_new(&x->x_obj, &s_float);
        }
    }
    return (x);
}

static void unpack_list(t_unpack *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *ap;
    t_unpackout *u;
    int i;
    if (argc > x->x_n) argc = x->x_n;
    for (i = argc, u = x->x_vec + i, ap = argv + i; u--, ap--, i--;)
    {
        t_atomtype type = u->u_type;
        if (type != ap->a_type)
            post_error ("unpack: type mismatch");
        else if (type == A_FLOAT)
            outlet_float(u->u_outlet, ap->a_w.w_float);
        else if (type == A_SYMBOL)
            outlet_symbol(u->u_outlet, ap->a_w.w_symbol);
        else outlet_pointer(u->u_outlet, ap->a_w.w_gpointer);
    }
}

static void unpack_anything(t_unpack *x, t_symbol *s, int ac, t_atom *av)
{
    t_atom *av2 = (t_atom *)PD_MEMORY_GET((ac + 1) * sizeof(t_atom));
    int i;
    for (i = 0; i < ac; i++)
        av2[i + 1] = av[i];
    SET_SYMBOL(av2, s);
    unpack_list(x, 0, ac+1, av2);
    PD_MEMORY_FREE(av2);
}

static void unpack_free(t_unpack *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

void unpack_setup(void)
{
    unpack_class = class_new(sym_unpack, (t_newmethod)unpack_new,
        (t_method)unpack_free, sizeof(t_unpack), 0, A_GIMME, 0);
    class_addList(unpack_class, unpack_list);
    class_addAnything(unpack_class, unpack_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
