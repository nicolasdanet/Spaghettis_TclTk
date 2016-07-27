
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

static t_class *append_class;

typedef struct _appendvariable
{
    t_symbol *gv_sym;
    t_float gv_f;
} t_appendvariable;

typedef struct _append
{
    t_object x_obj;
    t_gpointer x_gp;
    t_symbol *x_templatesym;
    int x_nin;
    t_appendvariable *x_variables;
} t_append;

static void *append_new(t_symbol *why, int argc, t_atom *argv)
{
    t_append *x = (t_append *)pd_new(append_class);
    int varcount, i;
    t_atom at, *varvec;
    t_appendvariable *sp;

    x->x_templatesym = template_makeBindSymbolWithWildcard(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_appendvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nin = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        sp->gv_f = 0;
        if (i) inlet_newFloat(&x->x_obj, &sp->gv_f);
    }
    inlet_newPointer(&x->x_obj, &x->x_gp);
    outlet_new(&x->x_obj, &s_pointer);
    gpointer_init(&x->x_gp);
    return (x);
}

static void append_set(t_append *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nin != 1)
        post_error ("set: cannot set multiple fields.");
    else
    {
       x->x_templatesym = template_makeBindSymbolWithWildcard(templatesym); 
       x->x_variables->gv_sym = field;
       x->x_variables->gv_f = 0;
    }
}

static void append_float(t_append *x, t_float f)
{
    int nitems = x->x_nin, i;
    t_symbol *templatesym = x->x_templatesym;
    t_template *template;
    t_appendvariable *vp;
    t_gpointer *gp = &x->x_gp;
    t_word *vec;
    t_scalar *sc, *oldsc;
    t_glist *glist;
    
    if (!templatesym->s_name)
    {
        post_error ("append: no template supplied");
        return;
    }
    template = template_findByIdentifier(templatesym);
    if (!template)
    {
        post_error ("append: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!gpointer_isSet (gp))
    {
        post_error ("append: no current pointer");
        return;
    }
    if (!gpointer_isScalar (gp)) {
        post_error ("append: lists only, not arrays");
        return;
    }
    
    glist = gpointer_getParentGlist (gp);
    if (glist->gl_uniqueIdentifier != gpointer_getUniqueIdentifier (gp))    /* isValid? */
    {
        post_error ("append: stale pointer");
        return;
    }
    
    if (!nitems) return;
    x->x_variables[0].gv_f = f;

    sc = scalar_new(glist, templatesym);
    if (!sc)
    {
        post_error ("%s: couldn't create scalar", templatesym->s_name);
        return;
    }
    
    oldsc = gpointer_getScalar (gp);
    
    if (oldsc)
    {
        sc->sc_g.g_next = oldsc->sc_g.g_next;
        oldsc->sc_g.g_next = &sc->sc_g;
    }
    else
    {
        sc->sc_g.g_next = glist->gl_graphics;
        glist->gl_graphics = &sc->sc_g;
    }

    gpointer_setAsScalar (gp, glist, sc);
    //gp->gp_un.gp_scalar = sc;
    vec = sc->sc_vector;
    for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
    {
        word_setFloat(vec, template, vp->gv_sym, vp->gv_f);
    }
 
    if (canvas_isMapped(canvas_getView(glist)))
        gobj_visibilityChanged(&sc->sc_g, glist, 1);
    /*  scalar_redraw(sc, glist);  ... have to do 'vis' instead here because
    redraw assumes we're already visible... */

    outlet_pointer(x->x_obj.te_outlet, gp);
}

static void append_free(t_append *x)
{
    PD_MEMORY_FREE(x->x_variables);
    gpointer_unset(&x->x_gp);
}

void append_setup(void)
{
    append_class = class_new (sym_append, (t_newmethod)append_new,
        (t_method)append_free, sizeof(t_append), 0, A_GIMME, 0);
    class_addFloat(append_class, append_float); 
    class_addMethod(append_class, (t_method)append_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
