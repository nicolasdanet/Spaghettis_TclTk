
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *append_class;                   /* Shared. */
    
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
    t_outlet            *x_outlet;
    } t_append;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error append_proceed (t_append *x, t_float f, int setFields)
{
    t_template *tmpl = template_findByIdentifier (x->x_templateIdentifier);
    
    if (!tmpl) { error_canNotFind (sym_append, sym_template); }
    else {
    //
    if (gpointer_isValidOrNull (&x->x_gpointer) && gpointer_isScalar (&x->x_gpointer)) {
    //
    t_scalar *scalar = scalar_new (gpointer_getOwner (&x->x_gpointer), x->x_templateIdentifier);
    
    if (!scalar) { error_invalid (sym_append, sym_template); }
    else {
    //
    if (setFields) {
    
        int i;
        
        x->x_fields[0].gv_f = f;
        
        for (i = 0; i < x->x_fieldsSize; i++) {
        //
        if (scalar_fieldIsFloat (scalar, x->x_fields[i].gv_fieldName)) {
            scalar_setFloat (scalar, x->x_fields[i].gv_fieldName, x->x_fields[i].gv_f);
        } else if (x->x_fields[i].gv_fieldName != &s_) {
            error_mismatch (sym_append, sym_type);
        }
        //
        }
    }
    
    glist_objectAddNext (gpointer_getOwner (&x->x_gpointer),
        cast_gobj (scalar),
        cast_gobj (gpointer_getScalar (&x->x_gpointer)));
        
    gpointer_setAsScalar (&x->x_gpointer, scalar);
    
    return PD_ERROR_NONE;
    //
    }
    //
    } else { error_invalid (sym_append, &s_pointer); }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void append_bang (t_append *x)
{
    if (!append_proceed (x, 0, 0)) { outlet_pointer (x->x_outlet, &x->x_gpointer); }
}

static void append_float (t_append *x, t_float f)
{
    if (!append_proceed (x, f, 1)) { outlet_pointer (x->x_outlet, &x->x_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void append_fields (t_append *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!append_proceed (x, 0, 0)) {
    //
    gpointer_setFields (&x->x_gpointer, argc, argv); outlet_pointer (x->x_outlet, &x->x_gpointer);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *append_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    
    return b;
    //
    }
    
    return NULL;
}

static void append_restore (t_append *x)
{
    t_append *old = (t_append *)instance_pendingFetch (cast_gobj (x));
    
    if (old) { gpointer_setByCopy (&x->x_gpointer, &old->x_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *append_new (t_symbol *s, int argc, t_atom *argv)
{
    t_append *x = (t_append *)pd_new (append_class);
    int i, n = PD_MAX (1, argc - 1);

    gpointer_init (&x->x_gpointer);
        
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (atom_getSymbolAtIndex (0, argc, argv));
    x->x_fieldsSize         = n;
    x->x_fields             = (t_appendvariable *)PD_MEMORY_GET (n * sizeof (t_appendvariable));
    x->x_outlet             = outlet_newPointer (cast_object (x));
    
    for (i = 0; i < x->x_fieldsSize; i++) {
        x->x_fields[i].gv_fieldName = atom_getSymbolAtIndex (i + 1, argc, argv);
        x->x_fields[i].gv_f = 0.0;
        if (i) { inlet_newFloat (cast_object (x), &x->x_fields[i].gv_f); }
    }
    
    inlet_newPointer (cast_object (x), &x->x_gpointer);
    
    return x;
}

static void append_free (t_append *x)
{
    PD_MEMORY_FREE (x->x_fields);
    
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    class_addBang (c, (t_method)append_bang);
    class_addFloat (c, (t_method)append_float);
    
    class_addMethod (c, (t_method)append_fields,    sym_fields,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)append_restore,   sym__restore,   A_NULL);

    class_setDataFunction (c, append_functionData);
    class_requirePending (c);
    
    append_class = c;
}

void append_destroy (void)
{
    class_free (append_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
