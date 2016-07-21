
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

static t_class  *gtemplate_class;               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _gtemplate {
    t_object    x_obj;                          /* MUST be the first. */
    t_template  *x_template;
    t_glist     *x_owner;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gtemplate_notify (t_gtemplate *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (cast_object (x)->te_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *gtemplate_getView (t_gtemplate *x)
{
    return x->x_owner;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *gtemplate_newInstance (t_template *template, int argc, t_atom *argv)
{
    t_gtemplate *x = (t_gtemplate *)pd_new (gtemplate_class);
    
    x->x_template  = template;
    x->x_owner     = canvas_getCurrent();
    
    template_registerInstance (x->x_template, x);

    outlet_new (cast_object (x), &s_anything);
    
    return x;
}

static void *gtemplate_new (t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *templateIdentifier = utils_makeBindSymbol (atom_getSymbolAtIndex (0, argc, argv));
    
    /* For now forbid multiple instantiation. */
    
    t_template *template = template_findByIdentifier (templateIdentifier);
    
    if (template && template_hasInstance (template)) { 
        post_error (PD_TRANSLATE ("struct: %s already exists"), templateIdentifier->s_name); 
        return NULL;

    } else {
        if (argc >= 1) { argc--; argv++; }
        if (!template) { template = template_new (templateIdentifier, argc, argv); }
        
        return (gtemplate_newInstance (template, argc, argv));
    }
}

static void gtemplate_free (t_gtemplate *x)
{
    template_unregisterInstance (x->x_template, x);
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
