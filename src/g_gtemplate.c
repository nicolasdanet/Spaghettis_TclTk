
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

extern t_class          *garray_class;
extern t_class          *scalar_class;
extern t_class          *canvas_class;
extern t_pdinstance     *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class          *gtemplate_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _gtemplate {
    t_object    x_obj;                          /* MUST be the first. */
    t_template  *x_template;
    t_glist     *x_owner;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *gtemplate_getView (t_gtemplate *x)
{
    return x->x_owner;
}

void gtemplate_notify (t_gtemplate *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (cast_object (x)->te_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *gtemplate_create (t_template *template, int argc, t_atom *argv)
{
    t_gtemplate *x = (t_gtemplate *)pd_new (gtemplate_class);
    
    x->x_template  = template;
    x->x_owner     = canvas_getCurrent();
    
    canvas_paintAllScalarsByTemplate (x->x_template, SCALAR_ERASE);
    x->x_template->tp_owner = x;
    canvas_paintAllScalarsByTemplate (x->x_template, SCALAR_DRAW);

    outlet_new (cast_object (x), &s_anything);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *gtemplate_new (t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *templateIdentifier = utils_makeBindSymbol (atom_getSymbolAtIndex (0, argc, argv));
    t_template *template = template_findByIdentifier (templateIdentifier);
    
    /* For now forbid multiple instantiation. */
    
    if (template && template->tp_owner) { 
        post_error (PD_TRANSLATE ("struct: %s already exists"), templateIdentifier->s_name); 
        return NULL;

    } else {
        if (argc >= 1) { argc--; argv++; }
        if (!template) { template = template_new (templateIdentifier, argc, argv); }
        return (gtemplate_create (template, argc, argv));
    }
}

static void gtemplate_free (t_gtemplate *x)
{
    canvas_paintAllScalarsByTemplate (x->x_template, SCALAR_ERASE);
    x->x_template->tp_owner = NULL;
    canvas_paintAllScalarsByTemplate (x->x_template, SCALAR_DRAW);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gtemplate_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_struct,
            (t_newmethod)gtemplate_new,
            (t_method)gtemplate_free,
            sizeof (t_gtemplate),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
                
    gtemplate_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
