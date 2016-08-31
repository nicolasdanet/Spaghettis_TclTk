
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

static t_class  *set_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _setvariable {
    t_symbol        *sv_fieldName;
    union word      sv_w;
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
#pragma mark -

static void set_bang (t_set *x)
{
    if (gpointer_isValidInstanceOf (&x->x_gpointer, x->x_templateIdentifier)) { 
    //
    int i;
    
    for (i = 0; i < x->x_fieldsSize; i++) {
    //
    t_symbol *s = x->x_fields[i].sv_fieldName;
    
    if (gpointer_hasField (&x->x_gpointer, s)) {
        if (x->x_asSymbol && gpointer_fieldIsSymbol (&x->x_gpointer, s)) {
            gpointer_setSymbol (&x->x_gpointer, s, x->x_fields[i].sv_w.w_symbol);
        } else if (!x->x_asSymbol && gpointer_fieldIsFloat (&x->x_gpointer, s)) {
            gpointer_setFloat (&x->x_gpointer, s, x->x_fields[i].sv_w.w_float);
        } else {
            error_mismatch (sym_set, sym_type);
        }
    } else { error_missingField (sym_set, s); }
    //
    }

    gpointer_redraw (&x->x_gpointer);
    //
    } else { error_invalid (sym_set, &s_pointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void set_float (t_set *x, t_float f)
{
    if (x->x_asSymbol) { error_mismatch (sym_set, sym_type); }
    else {
        x->x_fields[0].sv_w.w_float = f;
        set_bang (x);
    }
}

static void set_symbol (t_set *x, t_symbol *s)
{
    if (!x->x_asSymbol) { error_mismatch (sym_set, sym_type); }
    else {
        x->x_fields[0].sv_w.w_symbol = s; 
        set_bang (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void set_set (t_set *x, t_symbol *templateName, t_symbol *fieldName)
{
    if (x->x_fieldsSize != 1) { error_canNotSetMultipleFields (sym_set); }
    else {
        x->x_templateIdentifier     = template_makeIdentifierWithWildcard (templateName); 
        x->x_fields[0].sv_fieldName = fieldName;
       
        if (x->x_asSymbol) {
            x->x_fields[0].sv_w.w_symbol = &s_;
        } else {
            x->x_fields[0].sv_w.w_float  = 0.0;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    x->x_fieldsSize         = PD_MAX (1, argc - 1);
    x->x_fields             = (t_setvariable *)PD_MEMORY_GET (x->x_fieldsSize * sizeof (t_setvariable));
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (atom_getSymbolAtIndex (0, argc, argv));
    
    if (x->x_asSymbol) {
        int i;
        for (i = 0; i < x->x_fieldsSize; i++) {
            x->x_fields[i].sv_fieldName  = atom_getSymbolAtIndex (i + 1, argc, argv);
            x->x_fields[i].sv_w.w_symbol = &s_;
            if (i) { inlet_newSymbol (cast_object (x), &x->x_fields[i].sv_w.w_symbol); }
        }
        
    } else {
        int i;
        for (i = 0; i < x->x_fieldsSize; i++) {
            x->x_fields[i].sv_fieldName  = atom_getSymbolAtIndex (i + 1, argc, argv);
            x->x_fields[i].sv_w.w_float  = 0.0;
            if (i) { inlet_newFloat (cast_object (x), &x->x_fields[i].sv_w.w_float); }
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
#pragma mark -

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
    
    class_addBang (c, set_bang); 
    class_addFloat (c, set_float); 
    class_addSymbol (c, set_symbol); 

    class_addMethod (c, (t_method)set_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL); 
    
    set_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
