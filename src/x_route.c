
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

static t_class *route_class;

typedef struct _routeelement
{
    t_word e_w;
    t_outlet *e_outlet;
} t_routeelement;

typedef struct _route
{
    t_object x_obj;
    t_atomtype x_type;
    t_int x_nelement;
    t_routeelement *x_vec;
    t_outlet *x_rejectout;
} t_route;

static void route_anything(t_route *x, t_symbol *sel, int argc, t_atom *argv)
{
    t_routeelement *e;
    int nelement;
    if (x->x_type == A_SYMBOL) 
    {
        for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            if (e->e_w.w_symbol == sel)
        {
            if (argc > 0 && argv[0].a_type == A_SYMBOL)
                outlet_anything(e->e_outlet, argv[0].a_w.w_symbol,
                    argc-1, argv+1);
            else outlet_list(e->e_outlet, argc, argv);
            return;
        }
    }
    outlet_anything(x->x_rejectout, sel, argc, argv);
}

static void route_list(t_route *x, t_symbol *sel, int argc, t_atom *argv)
{
    t_routeelement *e;
    int nelement;
    if (x->x_type == A_FLOAT)
    {
        t_float f;
        if (!argc) return;
        if (argv->a_type != A_FLOAT)
            goto rejected;
        f = atom_getFloat(argv);
        for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            if (e->e_w.w_float == f)
        {
            if (argc > 1 && argv[1].a_type == A_SYMBOL)
                outlet_anything(e->e_outlet, argv[1].a_w.w_symbol,
                    argc-2, argv+2);
            else outlet_list(e->e_outlet, argc-1, argv+1);
            return;
        }
    }
    else    /* symbol arguments */
    {
        if (argc > 1)       /* 2 or more args: treat as "list" */
        {
            for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            {
                if (e->e_w.w_symbol == &s_list)
                {
                    if (argc > 0 && argv[0].a_type == A_SYMBOL)
                        outlet_anything(e->e_outlet, argv[0].a_w.w_symbol,
                            argc-1, argv+1);
                    else outlet_list(e->e_outlet, argc, argv);
                    return;
                }
            }
        }
        else if (argc == 0)         /* no args: treat as "bang" */
        {
            for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            {
                if (e->e_w.w_symbol == &s_bang)
                {
                    outlet_bang(e->e_outlet);
                    return;
                }
            }
        }
        else if (argv[0].a_type == A_FLOAT)     /* one float arg */
        {
            for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            {
                if (e->e_w.w_symbol == &s_float)
                {
                    outlet_float(e->e_outlet, argv[0].a_w.w_float);
                    return;
                }
            }
        }
        else
        {
            for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            {
                if (e->e_w.w_symbol == &s_symbol)
                {
                    outlet_symbol(e->e_outlet, argv[0].a_w.w_symbol);
                    return;
                }
            }
        }
    }
 rejected:
    outlet_list(x->x_rejectout, argc, argv);
}


static void route_free(t_route *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

static void *route_new(t_symbol *s, int argc, t_atom *argv)
{
    int n;
    t_routeelement *e;
    t_route *x = (t_route *)pd_new(route_class);
    t_atom a;
    if (argc == 0)
    {
        argc = 1;
        SET_FLOAT(&a, 0);
        argv = &a;
    }
    x->x_type = argv[0].a_type;
    x->x_nelement = argc;
    x->x_vec = (t_routeelement *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
    for (n = 0, e = x->x_vec; n < argc; n++, e++)
    {
        e->e_outlet = outlet_new(&x->x_obj, &s_list);
        if (x->x_type == A_FLOAT)
            e->e_w.w_float = atom_getFloatAtIndex(n, argc, argv);
        else e->e_w.w_symbol = atom_getSymbolAtIndex(n, argc, argv);
    }
    if (argc == 1)
    {
        if (argv->a_type == A_FLOAT)
            inlet_newFloat(&x->x_obj, &x->x_vec->e_w.w_float);
        else inlet_newSymbol(&x->x_obj, &x->x_vec->e_w.w_symbol);
    }
    x->x_rejectout = outlet_new(&x->x_obj, &s_list);
    return (x);
}

void route_setup(void)
{
    route_class = class_new(sym_route, (t_newmethod)route_new,
        (t_method)route_free, sizeof(t_route), 0, A_GIMME, 0);
    class_addList(route_class, route_list);
    class_addAnything(route_class, route_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
