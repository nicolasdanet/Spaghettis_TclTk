
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

static t_class  *append_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _appendvariable {
    t_symbol            *gv_fieldName;
    t_float             gv_f;
    } t_appendvariable;

typedef struct _append {
    t_object            x_obj;
    t_gpointer          x_gpointer;
    int                 x_fieldsSize;
    t_appendvariable    *x_fields;
    t_symbol            *x_templateIdentifier;
    } t_append;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void append_float(t_append *x, t_float f)
{
    int nitems = x->x_fieldsSize, i;
    t_symbol *templatesym = x->x_templateIdentifier;
    t_template *template;
    t_appendvariable *vp;
    t_gpointer *gp = &x->x_gpointer;
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
    x->x_fields[0].gv_f = f;

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
    for (i = 0, vp = x->x_fields; i < nitems; i++, vp++)
    {
        word_setFloat(vec, template, vp->gv_fieldName, vp->gv_f);
    }
 
    if (canvas_isMapped(canvas_getView(glist)))
        gobj_visibilityChanged(&sc->sc_g, glist, 1);
    /*  scalar_redraw(sc, glist);  ... have to do 'vis' instead here because
    redraw assumes we're already visible... */

    outlet_pointer(x->x_obj.te_outlet, gp);
}

static void append_set (t_append *x, t_symbol *templateName, t_symbol *fieldName)
{
    if (x->x_fieldsSize != 1) { post_error (PD_TRANSLATE ("append: cannot set multiple fields")); }
    else {
        x->x_templateIdentifier     = template_makeTemplateIdentifier (templateName); 
        x->x_fields[0].gv_fieldName = fieldName;
        x->x_fields[0].gv_f         = 0.0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *append_new (t_symbol *s, int argc, t_atom *argv)
{
    t_append *x = (t_append *)pd_new (append_class);
    int i, n = PD_MAX (1, argc - 1);

    gpointer_init (&x->x_gpointer);
        
    x->x_templateIdentifier = template_makeTemplateIdentifier (atom_getSymbolAtIndex (0, argc, argv));
    x->x_fieldsSize         = n;
    x->x_fields             = (t_appendvariable *)PD_MEMORY_GET (n * sizeof (t_appendvariable));

    for (i = 0; i < x->x_fieldsSize; i++) {
        x->x_fields[i].gv_fieldName = atom_getSymbolAtIndex (i + 1, argc, argv);
        x->x_fields[i].gv_f = 0.0;
        if (i) { inlet_newFloat (cast_object (x), &x->x_fields[i].gv_f); }
    }
    
    inlet_newPointer (cast_object (x), &x->x_gpointer);
    outlet_new (cast_object (x), &s_pointer);

    return x;
}

static void append_free (t_append *x)
{
    PD_MEMORY_FREE (x->x_fields);
    
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void append_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_append,
            (t_newmethod)append_new,
            (t_method)append_free,
            sizeof (t_append),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, append_float); 
    
    class_addMethod (c, (t_method)append_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL);
    
    append_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
