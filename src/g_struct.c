
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *struct_class;                          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _struct {
    t_object    x_obj;                          /* MUST be the first. */
    t_template  *x_template;
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void struct_notify (t_struct *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->x_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist *struct_getView (t_struct *x)
{
    return x->x_owner;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *struct_newInstance (t_template *template)
{
    t_struct *x = (t_struct *)pd_new (struct_class);
    
    x->x_template = template;
    x->x_owner    = instance_contextGetCurrent();
    x->x_outlet   = outlet_new (cast_object (x), &s_anything);
    
    template_registerInstance (x->x_template, x);

    return x;
}

static void *struct_newEmpty (void)
{
    t_struct *x = (t_struct *)pd_new (struct_class);
    
    x->x_template = NULL;
    x->x_owner    = instance_contextGetCurrent();
    x->x_outlet   = outlet_new (cast_object (x), &s_anything);
    
    return x;
}

static void *struct_new (t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *templateName = atom_getSymbolAtIndex (0, argc, argv);
    
    if (templateName == &s_) { return struct_newEmpty(); }
    else {
    //
    t_symbol *templateIdentifier = symbol_makeTemplateIdentifier (templateName);
    
    /* For now forbid multiple instantiation. */
    
    t_template *template = template_findByIdentifier (templateIdentifier);
    
    if (template && template_hasInstance (template)) { error_alreadyExists (templateName); return NULL; } 
    else {
        if (argc >= 1) { argc--; argv++; }
        if (!template) { template = template_new (templateIdentifier, argc, argv); }
        if (template)  {
            return struct_newInstance (template);
        }
        
        return NULL;
    }
    //
    }
}

static void struct_free (t_struct *x)
{
    if (x->x_template) { template_unregisterInstance (x->x_template, x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void struct_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_struct,
            (t_newmethod)struct_new,
            (t_method)struct_free,
            sizeof (t_struct),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
                
    struct_class = c;
}

void struct_destroy (void)
{
    class_free (struct_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
