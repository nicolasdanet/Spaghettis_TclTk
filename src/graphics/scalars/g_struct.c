
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *struct_class;                          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _struct {
    t_object    x_obj;                          /* MUST be the first. */
    int         x_inhibit;
    t_template  *x_template;
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void struct_dismiss (t_struct *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void struct_notify (t_struct *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->x_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist *struct_getOwner (t_struct *x)
{
    return x->x_owner;
}

/* Get unexpanded names (if any) in the object's text buffer. */

t_symbol *struct_getUnexpandedName (t_struct *x)
{
    t_buffer *b = object_getBuffer (cast_object (x));
    
    if (buffer_getSize (b) > 1) {
    //
    t_atom *a = x->x_inhibit ? buffer_getAtomAtIndexChecked (b, 2) : buffer_getAtomAtIndexChecked (b, 1);
    if (a && IS_DOLLARSYMBOL (a)) {
        char t[PD_STRING] = { 0 }; if (!(atom_toString (a, t, PD_STRING))) { return gensym (t); }
    }
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int struct_hasInhibit (t_struct *x)
{
    return x->x_inhibit;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void struct_functionDismiss (t_gobj *z)
{
    struct_dismiss ((t_struct *)z);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_struct *struct_newInstance (t_template *tmpl)
{
    t_struct *x = (t_struct *)pd_new (struct_class);
    
    x->x_template = tmpl;
    x->x_owner    = instance_contextGetCurrent();
    x->x_outlet   = outlet_newAnything (cast_object (x));
    
    template_registerInstance (x->x_template, x);

    return x;
}

static t_struct *struct_newEmpty (void)
{
    t_struct *x = (t_struct *)pd_new (struct_class);
    
    x->x_template = NULL;
    x->x_owner    = instance_contextGetCurrent();
    x->x_outlet   = outlet_newAnything (cast_object (x));
    
    return x;
}

static t_struct *struct_newProceed (int argc, t_atom *argv)
{
    t_symbol *templateName = atom_getSymbolAtIndex (0, argc, argv);
    
    if (templateName == &s_) { return struct_newEmpty(); }
    else {
    //
    t_symbol *templateIdentifier = symbol_makeTemplateIdentifier (templateName);
    
    /* For now forbid multiple instantiation. */
    
    t_template *tmpl = template_findByIdentifier (templateIdentifier);
    
    if (tmpl && template_hasInstance (tmpl)) { error_alreadyExists (templateName); return NULL; }
    else {
        if (argc >= 1) { argc--; argv++; }
        if (!tmpl) { tmpl = template_new (templateIdentifier, argc, argv); }
        if (tmpl)  {
            return struct_newInstance (tmpl);
        }
        
        return NULL;
    }
    //
    }
}

static void *struct_new (t_symbol *s, int argc, t_atom *argv)
{
    int inhibit = 0;    /* Disable the value entry in the popup menu. */
    
    if (atom_getSymbolAtIndex (0, argc, argv) == sym___dash__inhibit) { inhibit = 1; argc--; argv++; }
    
    if (error__options (s, argc, argv)) { return NULL; }
    else {
    //
    t_struct *x = struct_newProceed (argc, argv);
    
    if (x) { x->x_inhibit = inhibit; }
    
    return x;
    //
    }
}

static void struct_dismiss (t_struct *x)
{
    if (x->x_template) { template_forgetPendingInstance (x->x_template, x); x->x_template = NULL; }
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
    
    class_setDismissFunction (c, struct_functionDismiss);
    class_requirePending (c);
    
    struct_class = c;
}

void struct_destroy (void)
{
    class_free (struct_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
