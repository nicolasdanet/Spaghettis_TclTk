
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

/* -------------------------- int ------------------------------ */
static t_class *pdint_class;

typedef struct _pdint
{
    t_object x_obj;
    t_float x_f;
} t_pdint;

static void *pdint_new(t_float f)
{
    t_pdint *x = (t_pdint *)pd_new(pdint_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f);
    return (x);
}

static void pdint_bang(t_pdint *x)
{
    outlet_float(x->x_obj.te_outlet, (t_float)(int)(x->x_f));
}

static void pdint_float(t_pdint *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, (t_float)(int)(x->x_f = f));
}

static void pdint_send(t_pdint *x, t_symbol *s)
{
    if (s->s_thing)
        pd_float(s->s_thing, (t_float)(int)x->x_f);
    else post_error ("%s: no such object", s->s_name);
}

void pdint_setup(void)
{
    pdint_class = class_new (sym_int, (t_newmethod)pdint_new, 0,
        sizeof(t_pdint), 0, A_DEFFLOAT, 0);
    class_addCreator((t_newmethod)pdint_new, sym_i, A_DEFFLOAT, 0);
    class_addMethod(pdint_class, (t_method)pdint_send, sym_send,
        A_SYMBOL, 0);
    class_addBang(pdint_class, pdint_bang);
    class_addFloat(pdint_class, pdint_float);
}

/* -------------------------- float ------------------------------ */
static t_class *pdfloat_class;

typedef struct _pdfloat
{
    t_object x_obj;
    t_float x_f;
} t_pdfloat;

    /* "float," "symbol," and "bang" are special because
    they're created by short-circuited messages to the "new"
    object which are handled specially in pd_message(). */

static void *pdfloat_new(t_pd *dummy, t_float f)
{
    t_pdfloat *x = (t_pdfloat *)pd_new(pdfloat_class);
    x->x_f = f;
    outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f);
    pd_newest = &x->x_obj.te_g.g_pd;
    return (x);
}

static void *pdfloat_new2(t_float f)
{
    return (pdfloat_new(0, f));
}

static void pdfloat_bang(t_pdfloat *x)
{
    outlet_float(x->x_obj.te_outlet, x->x_f);
}

static void pdfloat_float(t_pdfloat *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, x->x_f = f);
}

static void pdfloat_send(t_pdfloat *x, t_symbol *s)
{
    if (s->s_thing)
        pd_float(s->s_thing, x->x_f);
    else post_error ("%s: no such object", s->s_name);
}

void pdfloat_setup(void)
{
    pdfloat_class = class_new(&s_float, (t_newmethod)pdfloat_new, 0,
        sizeof(t_pdfloat), 0, A_FLOAT, 0);
    class_addCreator((t_newmethod)pdfloat_new2, sym_f, A_DEFFLOAT, 0);
    class_addMethod(pdfloat_class, (t_method)pdfloat_send, sym_send,
        A_SYMBOL, 0);
    class_addBang(pdfloat_class, pdfloat_bang);
    class_addFloat(pdfloat_class, (t_method)pdfloat_float);
}

/* -------------------------- symbol ------------------------------ */
static t_class *pdsymbol_class;

typedef struct _pdsymbol
{
    t_object x_obj;
    t_symbol *x_s;
} t_pdsymbol;

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

/* -------------------------- bang ------------------------------ */
static t_class *bang_class;

typedef struct _bang
{
    t_object x_obj;
} t_bang;

static void *bang_new(t_pd *dummy)
{
    t_bang *x = (t_bang *)pd_new(bang_class);
    outlet_new(&x->x_obj, &s_bang);
    pd_newest = &x->x_obj.te_g.g_pd;
    return (x);
}

static void *bang_new2(t_bang f)
{
    return (bang_new(0));
}

static void bang_bang(t_bang *x)
{
    outlet_bang(x->x_obj.te_outlet);
}

void bang_setup(void)
{
    bang_class = class_new (&s_bang, (t_newmethod)bang_new, 0,
        sizeof(t_bang), 0, 0);
    class_addCreator((t_newmethod)bang_new2, sym_b, 0);
    class_addBang(bang_class, bang_bang);
    class_addFloat(bang_class, bang_bang);
    class_addSymbol(bang_class, bang_bang);
    class_addList(bang_class, bang_bang);
    class_addAnything(bang_class, bang_bang);
}

/* -------------------- send ------------------------------ */

static t_class *send_class;

typedef struct _send
{
    t_object x_obj;
    t_symbol *x_sym;
} t_send;

static void send_bang(t_send *x)
{
    if (x->x_sym->s_thing) pd_bang(x->x_sym->s_thing);
}

static void send_float(t_send *x, t_float f)
{
    if (x->x_sym->s_thing) pd_float(x->x_sym->s_thing, f);
}

static void send_symbol(t_send *x, t_symbol *s)
{
    if (x->x_sym->s_thing) pd_symbol(x->x_sym->s_thing, s);
}

static void send_pointer(t_send *x, t_gpointer *gp)
{
    if (x->x_sym->s_thing) pd_pointer(x->x_sym->s_thing, gp);
}

static void send_list(t_send *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_sym->s_thing) pd_list(x->x_sym->s_thing, argc, argv);
}

static void send_anything(t_send *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_sym->s_thing) pd_message(x->x_sym->s_thing, s, argc, argv);
}

static void *send_new(t_symbol *s)
{
    t_send *x = (t_send *)pd_new(send_class);
    if (!*s->s_name)
        inlet_newSymbol(&x->x_obj, &x->x_sym);
    x->x_sym = s;
    return (x);
}

void send_setup(void)
{
    send_class = class_new(sym_send, (t_newmethod)send_new, 0,
        sizeof(t_send), 0, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)send_new, sym_s, A_DEFSYMBOL, 0);
    class_addBang(send_class, send_bang);
    class_addFloat(send_class, send_float);
    class_addSymbol(send_class, send_symbol);
    class_addPointer(send_class, send_pointer);
    class_addList(send_class, send_list);
    class_addAnything(send_class, send_anything);
}
/* -------------------- receive ------------------------------ */

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

/* -------------------------- select ------------------------------ */

static t_class *sel1_class;

typedef struct _sel1
{
    t_object x_obj;
    t_atom x_atom;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_sel1;

static void sel1_float(t_sel1 *x, t_float f)
{
    if (x->x_atom.a_type == A_FLOAT && f == x->x_atom.a_w.w_float)
        outlet_bang(x->x_outlet1);
    else outlet_float(x->x_outlet2, f);
}

static void sel1_symbol(t_sel1 *x, t_symbol *s)
{
    if (x->x_atom.a_type == A_SYMBOL && s == x->x_atom.a_w.w_symbol)
        outlet_bang(x->x_outlet1);
    else outlet_symbol(x->x_outlet2, s);
}

static t_class *sel2_class;

typedef struct _selectelement
{
    t_word e_w;
    t_outlet *e_outlet;
} t_selectelement;

typedef struct _sel2
{
    t_object x_obj;
    t_atomtype x_type;
    t_int x_nelement;
    t_selectelement *x_vec;
    t_outlet *x_rejectout;
} t_sel2;

static void sel2_float(t_sel2 *x, t_float f)
{
    t_selectelement *e;
    int nelement;
    if (x->x_type == A_FLOAT)
    {
        for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            if (e->e_w.w_float == f)
        {
            outlet_bang(e->e_outlet);
            return;
        }
    }
    outlet_float(x->x_rejectout, f);
}

static void sel2_symbol(t_sel2 *x, t_symbol *s)
{
    t_selectelement *e;
    int nelement;
    if (x->x_type == A_SYMBOL)
    {
        for (nelement = x->x_nelement, e = x->x_vec; nelement--; e++)
            if (e->e_w.w_symbol == s)
        {
            outlet_bang(e->e_outlet);
            return;
        }
    }
    outlet_symbol(x->x_rejectout, s);
}

static void sel2_free(t_sel2 *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

static void *select_new(t_symbol *s, int argc, t_atom *argv)
{
    t_atom a;
    if (argc == 0)
    {
        argc = 1;
        SET_FLOAT(&a, 0);
        argv = &a;
    }
    if (argc == 1)
    {
        t_sel1 *x = (t_sel1 *)pd_new(sel1_class);
        x->x_atom = *argv;
        x->x_outlet1 = outlet_new(&x->x_obj, &s_bang);
        if (argv->a_type == A_FLOAT)
        {
            inlet_newFloat(&x->x_obj, &x->x_atom.a_w.w_float);
            x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
        }
        else
        {
            inlet_newSymbol(&x->x_obj, &x->x_atom.a_w.w_symbol);
            x->x_outlet2 = outlet_new(&x->x_obj, &s_symbol);
        }
        return (x);
    }
    else
    {
        int n;
        t_selectelement *e;
        t_sel2 *x = (t_sel2 *)pd_new(sel2_class);
        x->x_nelement = argc;
        x->x_vec = (t_selectelement *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
        x->x_type = argv[0].a_type;
        for (n = 0, e = x->x_vec; n < argc; n++, e++)
        {
            e->e_outlet = outlet_new(&x->x_obj, &s_bang);
            if ((x->x_type = argv->a_type) == A_FLOAT)
                e->e_w.w_float = atom_getFloatAtIndex(n, argc, argv);
            else e->e_w.w_symbol = atom_getSymbolAtIndex(n, argc, argv);
        }
        x->x_rejectout = outlet_new(&x->x_obj, &s_float);
        return (x);
    }

}

void select_setup(void)
{
    sel1_class = class_new(sym_select, 0, 0,
        sizeof(t_sel1), 0, 0);
    class_addFloat(sel1_class, sel1_float);
    class_addSymbol(sel1_class, sel1_symbol);

    sel2_class = class_new(sym_select, 0, (t_method)sel2_free,
        sizeof(t_sel2), 0, 0);
    class_addFloat(sel2_class, sel2_float);
    class_addSymbol(sel2_class, sel2_symbol);

    class_addCreator((t_newmethod)select_new, sym_select,  A_GIMME, 0);
    class_addCreator((t_newmethod)select_new, sym_sel,  A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
