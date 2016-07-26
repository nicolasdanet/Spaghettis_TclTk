
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

static t_class *set_class;

typedef struct _setvariable
{
    t_symbol *gv_sym;
    union word gv_w;
} t_setvariable;

typedef struct _set
{
    t_object x_obj;
    t_gpointer x_gp;
    t_symbol *x_templatesym;
    int x_nin;
    int x_issymbol;
    t_setvariable *x_variables;
} t_set;

static void *set_new(t_symbol *why, int argc, t_atom *argv)
{
    t_set *x = (t_set *)pd_new(set_class);
    int i, varcount;
    t_setvariable *sp;
    t_atom at, *varvec;
    if (argc && (argv[0].a_type == A_SYMBOL) &&
        !strcmp(argv[0].a_w.w_symbol->s_name, "-symbol"))
    {
        x->x_issymbol = 1;
        argc--;
        argv++;
    }
    else x->x_issymbol = 0;
    x->x_templatesym = template_makeIdentifierWithWildcard(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_setvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nin = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        if (x->x_issymbol)
            sp->gv_w.w_symbol = &s_;
        else sp->gv_w.w_float = 0;
        if (i)
        {
            if (x->x_issymbol)
                inlet_newSymbol(&x->x_obj, &sp->gv_w.w_symbol);
            else inlet_newFloat(&x->x_obj, &sp->gv_w.w_float);
        }
    }
    inlet_newPointer(&x->x_obj, &x->x_gp);
    gpointer_init(&x->x_gp);
    return (x);
}

static void set_set(t_set *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nin != 1)
        post_error ("set: cannot set multiple fields.");
    else
    {
       x->x_templatesym = template_makeIdentifierWithWildcard(templatesym); 
       x->x_variables->gv_sym = field;
       if (x->x_issymbol)
           x->x_variables->gv_w.w_symbol = &s_;
       else
           x->x_variables->gv_w.w_float = 0;
    }
}

static void set_bang(t_set *x)
{
    int nitems = x->x_nin, i;
    t_symbol *templatesym;
    t_template *template;
    t_setvariable *vp;
    t_gpointer *gp = &x->x_gp;
    t_word *vec;
    if (!gpointer_isValid(gp))
    {
        post_error ("set: empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) != gpointer_getTemplateIdentifier(gp))
        {
            post_error ("set %s: got wrong template (%s)",
                templatesym->s_name, gpointer_getTemplateIdentifier(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_getTemplateIdentifier(gp);
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("set: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!nitems)
        return;
    vec = gpointer_getData (gp);
    if (x->x_issymbol)
        for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
            word_setSymbol(vec, template, vp->gv_sym, vp->gv_w.w_symbol);
    else for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
        word_setFloat(vec, template, vp->gv_sym, vp->gv_w.w_float);
    scalar_redrawByPointer (gp);
}

static void set_float(t_set *x, t_float f)
{
    if (x->x_nin && !x->x_issymbol)
    {
        x->x_variables[0].gv_w.w_float = f;
        set_bang(x);
    }
    else post_error ("type mismatch or no field specified");
}

static void set_symbol(t_set *x, t_symbol *s)
{
    if (x->x_nin && x->x_issymbol)
    {
        x->x_variables[0].gv_w.w_symbol = s;
        set_bang(x);
    }
    else post_error ("type mismatch or no field specified");
}

static void set_free(t_set *x)
{
    PD_MEMORY_FREE(x->x_variables);
    gpointer_unset(&x->x_gp);
}

void set_setup(void)
{
    set_class = class_new(sym_set, (t_newmethod)set_new,
        (t_method)set_free, sizeof(t_set), 0, A_GIMME, 0);
    class_addFloat(set_class, set_float); 
    class_addSymbol(set_class, set_symbol); 
    class_addBang(set_class, set_bang); 
    class_addMethod(set_class, (t_method)set_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
