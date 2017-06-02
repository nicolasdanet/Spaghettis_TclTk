
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *set_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _setvariable {
    t_symbol        *sv_fieldName;
    t_word          sv_w;
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
            gpointer_setSymbol (&x->x_gpointer, s, WORD_SYMBOL (&x->x_fields[i].sv_w));
        } else if (!x->x_asSymbol && gpointer_fieldIsFloat (&x->x_gpointer, s)) {
            gpointer_setFloat (&x->x_gpointer, s, WORD_FLOAT (&x->x_fields[i].sv_w));
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void set_float (t_set *x, t_float f)
{
    if (x->x_asSymbol) { error_mismatch (sym_set, sym_type); }
    else {
        WORD_FLOAT (&x->x_fields[0].sv_w) = f;
        set_bang (x);
    }
}

static void set_symbol (t_set *x, t_symbol *s)
{
    if (!x->x_asSymbol) { error_mismatch (sym_set, sym_type); }
    else {
        WORD_SYMBOL (&x->x_fields[0].sv_w) = s; 
        set_bang (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void set_set (t_set *x, t_symbol *templateName, t_symbol *fieldName)
{
    if (x->x_fieldsSize != 1) { error_canNotSetMultipleFields (sym_set); }
    else {
        x->x_templateIdentifier     = template_makeIdentifierWithWildcard (templateName); 
        x->x_fields[0].sv_fieldName = fieldName;
       
        if (x->x_asSymbol) {
            WORD_SYMBOL (&x->x_fields[0].sv_w) = &s_;
        } else {
            WORD_FLOAT (&x->x_fields[0].sv_w)  = (t_float)0.0;
        }
    }
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
        if (t == sym___dash__s || t == sym___dash__symbol) {
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
            WORD_SYMBOL (&x->x_fields[i].sv_w) = &s_;
            if (i) { inlet_newSymbol (cast_object (x), (t_symbol **)&x->x_fields[i].sv_w); }
        }
        
    } else {
        int i;
        for (i = 0; i < x->x_fieldsSize; i++) {
            x->x_fields[i].sv_fieldName  = atom_getSymbolAtIndex (i + 1, argc, argv);
            WORD_FLOAT (&x->x_fields[i].sv_w) = (t_float)0.0;
            if (i) { inlet_newFloat (cast_object (x), (t_float *)&x->x_fields[i].sv_w); }
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

    class_addMethod (c, (t_method)set_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL); 
    
    set_class = c;
}

void set_destroy (void)
{
    CLASS_FREE (set_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
