
/* Copyright (c) 1997-2020 Miller Puckette and others. */

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

static t_class *set_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _setvariable {
    t_symbol        *sv_fieldName;
    t_atom          sv_atom;
    } t_setvariable;

typedef struct _set {
    t_object        x_obj;                  /* Must be the first. */
    t_gpointer      x_gpointer;
    int             x_asSymbol;
    int             x_fieldsSize;
    t_setvariable   *x_fields;
    t_symbol        *x_templateIdentifier;
    } t_set;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void set_bang (t_set *x)
{
    if (gpointer_isValidInstanceOf (&x->x_gpointer, x->x_templateIdentifier)) { 
    //
    int i;
    
    gpointer_erase (&x->x_gpointer);
    
    for (i = 0; i < x->x_fieldsSize; i++) {
    //
    t_symbol *s = x->x_fields[i].sv_fieldName;
    
    if (gpointer_hasField (&x->x_gpointer, s)) {
        if (x->x_asSymbol && gpointer_fieldIsSymbol (&x->x_gpointer, s)) {
            gpointer_setSymbol (&x->x_gpointer, s, GET_SYMBOL (&x->x_fields[i].sv_atom));
        } else if (!x->x_asSymbol && gpointer_fieldIsFloat (&x->x_gpointer, s)) {
            gpointer_setFloat (&x->x_gpointer, s, GET_FLOAT (&x->x_fields[i].sv_atom));
        } else {
            error_mismatch (sym_set, sym_type);
        }
    } else { error_missingField (sym_set, s); }
    //
    }

    gpointer_draw (&x->x_gpointer);
    //
    } else { error_invalid (sym_set, &s_pointer); }
}

static void set_float (t_set *x, t_float f)
{
    if (x->x_asSymbol) { error_mismatch (sym_set, sym_type); }
    else {
        SET_FLOAT (&x->x_fields[0].sv_atom, f); set_bang (x);
    }
}

static void set_symbol (t_set *x, t_symbol *s)
{
    if (!x->x_asSymbol) { error_mismatch (sym_set, sym_type); }
    else {
        SET_SYMBOL (&x->x_fields[0].sv_atom, s); set_bang (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void set_fields (t_set *x, t_symbol *s, int argc, t_atom *argv)
{
    if (gpointer_isValidInstanceOf (&x->x_gpointer, x->x_templateIdentifier)) {
    //
    gpointer_setFields (&x->x_gpointer, argc, argv);
    //
    } else { error_invalid (sym_set, &s_pointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *set_functionData (t_gobj *z, int flags)
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

static void set_restore (t_set *x)
{
    t_set *old = (t_set *)instance_pendingFetch (cast_gobj (x));
    
    if (old) { gpointer_setByCopy (&x->x_gpointer, &old->x_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *set_new (t_symbol *s, int argc, t_atom *argv)
{
    t_set *x = (t_set *)pd_new (set_class);
    
    x->x_asSymbol = 0;
    
    if (argc && IS_SYMBOL (argv)) {
    
        t_symbol *t = GET_SYMBOL (argv);
        
        if (t == sym___dash__symbol) {
            x->x_asSymbol = 1;
            argc--;
            argv++;
        }
    }
    
    error__options (s, argc, argv);
    
    x->x_fieldsSize         = PD_MAX (1, argc - 1);
    x->x_fields             = (t_setvariable *)PD_MEMORY_GET (x->x_fieldsSize * sizeof (t_setvariable));
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (atom_getSymbolAtIndex (0, argc, argv));
    
    if (x->x_asSymbol) {
        int i;
        for (i = 0; i < x->x_fieldsSize; i++) {
            x->x_fields[i].sv_fieldName  = atom_getSymbolAtIndex (i + 1, argc, argv);
            SET_SYMBOL (&x->x_fields[i].sv_atom, &s_);
            if (i) { inlet_newSymbol (cast_object (x), ADDRESS_SYMBOL (&x->x_fields[i].sv_atom)); }
        }
        
    } else {
        int i;
        for (i = 0; i < x->x_fieldsSize; i++) {
            x->x_fields[i].sv_fieldName  = atom_getSymbolAtIndex (i + 1, argc, argv);
            SET_FLOAT (&x->x_fields[i].sv_atom, 0.0);
            if (i) { inlet_newFloat (cast_object (x), ADDRESS_FLOAT (&x->x_fields[i].sv_atom)); }
        }
    }
    
    gpointer_init (&x->x_gpointer);
        
    inlet_newPointer (&x->x_obj, &x->x_gpointer);
    
    return x;
}

static void set_free (t_set *x)
{
    PD_MEMORY_FREE (x->x_fields);
    
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void set_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_set,
            (t_newmethod)set_new,
            (t_method)set_free,
            sizeof (t_set),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)set_bang); 
    class_addFloat (c, (t_method)set_float); 
    class_addSymbol (c, (t_method)set_symbol); 

    class_addMethod (c, (t_method)set_fields,   sym_fields,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)set_restore,  sym__restore,   A_NULL);

    class_setDataFunction (c, set_functionData);
    class_requirePending (c);
    
    set_class = c;
}

void set_destroy (void)
{
    class_free (set_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
