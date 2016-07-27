
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

static t_class *get_class;

typedef struct _getvariable
{
    t_symbol *gv_sym;
    t_outlet *gv_outlet;
} t_getvariable;

typedef struct _get
{
    t_object x_obj;
    t_symbol *x_templatesym;
    int x_nout;
    t_getvariable *x_variables;
} t_get;

static void *get_new(t_symbol *why, int argc, t_atom *argv)
{
    t_get *x = (t_get *)pd_new(get_class);
    int varcount, i;
    t_atom at, *varvec;
    t_getvariable *sp;

    x->x_templatesym = template_makeBindSymbolWithWildcard(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_getvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nout = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        sp->gv_outlet = outlet_new(&x->x_obj, 0);
            /* LATER connect with the template and set the outlet's type
            correctly.  We can't yet guarantee that the template is there
            before we hit this routine. */
    }
    return (x);
}

static void get_set(t_get *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nout != 1)
        post_error ("get: cannot set multiple fields.");
    else
    {
        x->x_templatesym = template_makeBindSymbolWithWildcard(templatesym); 
        x->x_variables->gv_sym = field;
    }
}

static void get_pointer(t_get *x, t_gpointer *gp)
{
    int nitems = x->x_nout, i;
    t_symbol *templatesym;
    t_template *template;
    t_word *vec; 
    t_getvariable *vp;

    if (!gpointer_isValid(gp))
    {
        post_error ("get: stale or empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) != gpointer_getTemplateIdentifier(gp))
        {
            post_error ("get %s: got wrong template (%s)",
                templatesym->s_name, gpointer_getTemplateIdentifier(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_getTemplateIdentifier(gp);
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("get: couldn't find template %s", templatesym->s_name);
        return;
    }
    vec = gpointer_getData (gp);
    for (i = nitems - 1, vp = x->x_variables + i; i >= 0; i--, vp--)
    {
        int onset, type;
        t_symbol *arraytype;
        if (template_findField(template, vp->gv_sym, &onset, &type, &arraytype))
        {
            if (type == DATA_FLOAT) {
                outlet_float(vp->gv_outlet,
                    *(t_float *)(((char *)vec) + onset));
            } else if (type == DATA_SYMBOL) {
                outlet_symbol(vp->gv_outlet,
                    *(t_symbol **)(((char *)vec) + onset));
            } else {
                // post_error ("get: %s.%s is not a number or symbol", template->tp_templateIdentifier->s_name, vp->gv_sym->s_name);
            }
        } else {
            // post_error ("get: %s.%s: no such field", template->tp_templateIdentifier->s_name, vp->gv_sym->s_name);
        }
    }
}

static void get_free(t_get *x)
{
    PD_MEMORY_FREE(x->x_variables);
}

void get_setup(void)
{
    get_class = class_new (sym_get, (t_newmethod)get_new,
        (t_method)get_free, sizeof(t_get), 0, A_GIMME, 0);
    class_addPointer(get_class, get_pointer); 
    class_addMethod(get_class, (t_method)get_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
