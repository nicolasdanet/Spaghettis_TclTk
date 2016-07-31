
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

static void append_float (t_append *x, t_float f)
{
    t_template *template = template_findByIdentifier (x->x_templateIdentifier);
    
    if (!template) { post_error (PD_TRANSLATE ("append: couldn't find template")); }
    else {
    //
    if (!gpointer_isValidNullAllowed (&x->x_gpointer) || !gpointer_isScalar (&x->x_gpointer)) {
        pointer_error (sym_append); 
        
    } else {
    //
    t_scalar *scalar = scalar_new (gpointer_getView (&x->x_gpointer), x->x_templateIdentifier);
    
    if (!scalar) { PD_BUG; }
    else {
    //
    int i;
    
    x->x_fields[0].gv_f = f;
        
    for (i = 0; i < x->x_fieldsSize; i++) {
        if (scalar_fieldIsFloat (scalar, x->x_fields[i].gv_fieldName)) {
            scalar_setFloat (scalar, x->x_fields[i].gv_fieldName, x->x_fields[i].gv_f);
        }
    }
    
    canvas_addScalarNext (gpointer_getView (&x->x_gpointer), gpointer_getScalar (&x->x_gpointer), scalar);
    
    gpointer_setAsScalar (&x->x_gpointer, gpointer_getView (&x->x_gpointer), scalar);
    outlet_pointer (cast_object (x)->te_outlet, &x->x_gpointer);
    //
    }
    //
    }
    //
    }
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
