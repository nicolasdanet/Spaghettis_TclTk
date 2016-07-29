
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

static t_class *get_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _getvariable {
    t_symbol        *gv_fieldName;
    t_outlet        *gv_outlet;
    } t_getvariable;

typedef struct _get {
    t_object        x_obj;                  /* Must be the first. */
    int             x_fieldsSize;
    t_getvariable   *x_fields;
    t_symbol        *x_templateIdentifier;
    } t_get;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void get_pointer (t_get *x, t_gpointer *gp)
{
    if (!gpointer_isValid (gp)) { pointer_error(); }
    else {
    //
    t_symbol *templateIdentifier = x->x_templateIdentifier;
    
    if (templateIdentifier == &s_) {                                        /* Wildcard. */
        templateIdentifier = gpointer_getTemplateIdentifier (gp);
    }

    if (templateIdentifier == gpointer_getTemplateIdentifier (gp)) {
    //
    if (!gpointer_getTemplate (gp)) { PD_BUG; }
    else {
    //
    int i;
    for (i = x->x_fieldsSize - 1; i >= 0; i--) {
    
        t_outlet *o = x->x_fields[i].gv_outlet;
        t_symbol *s = x->x_fields[i].gv_fieldName;
        
        if (gpointer_hasField (gp, s)) {
            if (gpointer_fieldIsFloat (gp, s))       { outlet_float (o, gpointer_getFloat (gp, s)); }
            else if (gpointer_fieldIsSymbol (gp, s)) { outlet_symbol (o, gpointer_getSymbol (gp, s)); }
        }
    }
    //
    }
    //
    }
    //
    }
}

static void get_set (t_get *x, t_symbol *templateIdentifier, t_symbol *fieldName)
{
    if (x->x_fieldsSize != 1) { post_error (PD_TRANSLATE ("get: cannot set multiple fields")); }
    else {
        x->x_templateIdentifier     = template_makeBindSymbolWithWildcard (templateIdentifier); 
        x->x_fields[0].gv_fieldName = fieldName;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *get_new (t_symbol *s, int argc, t_atom *argv)
{
    t_get *x = (t_get *)pd_new (get_class);
    int i;

    x->x_fieldsSize         = PD_MAX (1, argc - 1);
    x->x_fields             = (t_getvariable *)PD_MEMORY_GET (x->x_fieldsSize * sizeof (t_getvariable));
    x->x_templateIdentifier = template_makeBindSymbolWithWildcard (atom_getSymbolAtIndex (0, argc, argv));
    
    for (i = 0; i < x->x_fieldsSize; i++) {
        x->x_fields[i].gv_fieldName = atom_getSymbolAtIndex (i + 1, argc, argv);
        x->x_fields[i].gv_outlet    = outlet_new (cast_object (x), &s_anything);
    }
    
    return x;
}

static void get_free (t_get *x)
{
    PD_MEMORY_FREE (x->x_fields);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void get_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_get,
            (t_newmethod)get_new,
            (t_method)get_free,
            sizeof (t_get),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addPointer (c, get_pointer); 
    
    class_addMethod (c, (t_method)get_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL); 
    
    get_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
