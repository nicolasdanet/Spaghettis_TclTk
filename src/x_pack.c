
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

static t_class *pack_class;

typedef struct _pack
{
    t_object x_obj;
    t_int x_n;              /* number of args */
    t_atom *x_vec;          /* input values */
    t_int x_nptr;           /* number of pointers */
    t_gpointer *x_gpointer; /* the pointers */
    t_atom *x_outvec;       /* space for output values */
} t_pack;

static void *pack_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pack *x = (t_pack *)pd_new(pack_class);
    t_atom defarg[2], *ap, *vec, *vp;
    t_gpointer *gp;
    int nptr = 0;
    int i;
    if (!argc)
    {
        argv = defarg;
        argc = 2;
        SET_FLOAT(&defarg[0], 0);
        SET_FLOAT(&defarg[1], 0);
    }

    x->x_n = argc;
    vec = x->x_vec = (t_atom *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
    x->x_outvec = (t_atom *)PD_MEMORY_GET(argc * sizeof(*x->x_outvec));

    for (i = argc, ap = argv; i--; ap++)
        if (ap->a_type == A_SYMBOL && *ap->a_w.w_symbol->s_name == 'p')
            nptr++;

    gp = x->x_gpointer = (t_gpointer *)PD_MEMORY_GET(nptr * sizeof (*gp));
    x->x_nptr = nptr;

    for (i = 0, vp = x->x_vec, ap = argv; i < argc; i++, ap++, vp++)
    {
        if (ap->a_type == A_FLOAT)
        {
            *vp = *ap;
            if (i) inlet_newFloat(&x->x_obj, &vp->a_w.w_float);
        }
        else if (ap->a_type == A_SYMBOL)
        {
            char c = *ap->a_w.w_symbol->s_name;
            if (c == 's')
            {
                SET_SYMBOL(vp, &s_symbol);
                if (i) inlet_newSymbol(&x->x_obj, &vp->a_w.w_symbol);
            }
            else if (c == 'p')
            {
                vp->a_type = A_POINTER;
                vp->a_w.w_gpointer = gp;
                gpointer_init(gp);
                if (i) inlet_newPointer(&x->x_obj, gp);
                gp++;
            }
            else
            {
                if (c != 'f') post_error ("pack: %s: bad type",
                    ap->a_w.w_symbol->s_name);
                SET_FLOAT(vp, 0);
                if (i) inlet_newFloat(&x->x_obj, &vp->a_w.w_float);
            }
        }
    }
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void pack_bang(t_pack *x)
{
    int i, reentered = 0, size = x->x_n * sizeof (t_atom);
    t_gpointer *gp;
    t_atom *outvec;
    for (i = x->x_nptr, gp = x->x_gpointer; i--; gp++)
        if (!gpointer_isValidNullAllowed(gp))
    {
        post_error ("pack: stale pointer");
        return;
    }
        /* reentrancy protection.  The first time through use the pre-allocated
        x_outvec; if we're reentered we have to allocate new memory. */
    if (!x->x_outvec)
    {
            /* LATER figure out how to deal with reentrancy and pointers... */
        if (x->x_nptr)
            post("pack_bang: warning: reentry with pointers unprotected");
        outvec = PD_MEMORY_GET(size);
        reentered = 1;
    }
    else
    {
        outvec = x->x_outvec;
        x->x_outvec = 0;
    }
    memcpy(outvec, x->x_vec, size);
    outlet_list(x->x_obj.te_outlet, x->x_n, outvec);
    if (reentered)
        PD_MEMORY_FREE(outvec);
    else x->x_outvec = outvec;
}

static void pack_pointer(t_pack *x, t_gpointer *gp)
{
    if (x->x_vec->a_type == A_POINTER)
    {
        gpointer_setByCopy (gp, x->x_gpointer);
        pack_bang(x);
    }
    else post_error ("pack_pointer: wrong type");
}

static void pack_float(t_pack *x, t_float f)
{
    if (x->x_vec->a_type == A_FLOAT)
    {
        x->x_vec->a_w.w_float = f;
        pack_bang(x);
    }
    else post_error ("pack_float: wrong type");
}

static void pack_symbol(t_pack *x, t_symbol *s)
{
    if (x->x_vec->a_type == A_SYMBOL)
    {
        x->x_vec->a_w.w_symbol = s;
        pack_bang(x);
    }
    else post_error ("pack_symbol: wrong type");
}

static void pack_list(t_pack *x, t_symbol *s, int ac, t_atom *av)
{
    object_list(&x->x_obj, 0, ac, av);
}

static void pack_anything(t_pack *x, t_symbol *s, int ac, t_atom *av)
{
    t_atom *av2 = (t_atom *)PD_MEMORY_GET((ac + 1) * sizeof(t_atom));
    int i;
    for (i = 0; i < ac; i++)
        av2[i + 1] = av[i];
    SET_SYMBOL(av2, s);
    object_list(&x->x_obj, 0, ac+1, av2);
    PD_MEMORY_FREE(av2);
}

static void pack_free(t_pack *x)
{
    t_gpointer *gp;
    int i;
    for (gp = x->x_gpointer, i = x->x_nptr; i--; gp++)
        gpointer_unset(gp);
    PD_MEMORY_FREE(x->x_vec);
    PD_MEMORY_FREE(x->x_outvec);
    PD_MEMORY_FREE(x->x_gpointer);
}

void pack_setup(void)
{
    pack_class = class_new(sym_pack, (t_newmethod)pack_new,
        (t_method)pack_free, sizeof(t_pack), 0, A_GIMME, 0);
    class_addBang(pack_class, pack_bang);
    class_addFloat(pack_class, pack_float);
    class_addSymbol(pack_class, pack_symbol);
    class_addPointer(pack_class, pack_pointer);
    class_addList(pack_class, pack_list);
    class_addAnything(pack_class, pack_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
