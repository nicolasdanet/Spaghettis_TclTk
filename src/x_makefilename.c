
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *makefilename_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _makefilename {
    t_object        x_obj;                          /* Must be the first. */
    t_atomtype      x_typeRequired;
    int             x_isIntegerCastRequired;
    t_symbol        *x_format;
    t_outlet        *x_outlet;
    } t_makefilename;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Note that complex and multiple substitutions are rejected. */

static t_error makefilename_scanFormat (t_makefilename *x)
{
    t_error err = PD_ERROR;
    
    if (x->x_format) {
    //
    int k = 0; char *s = NULL;
    
    x->x_typeRequired = A_NULL; err = PD_ERROR_NONE;
    
    for (s = x->x_format->s_name; *s; s++) {
    
        if (string_containsCharacterAtStart (s, "%") && k == 0) { 
            if (x->x_typeRequired == A_NULL) { k = 1; }
            else {
                err = PD_ERROR; break;
            }
            
        } else if (k) {
            if (string_containsCharacterAtStart (s, "s")) {
                k = 0; x->x_isIntegerCastRequired = 0; x->x_typeRequired = A_SYMBOL;

            } else if (string_containsCharacterAtStart (s, "fgGeE")) {
                k = 0; x->x_isIntegerCastRequired = 0; x->x_typeRequired = A_FLOAT;
                
            } else if (string_containsCharacterAtStart (s, "xXdiouc")) {
                k = 0; x->x_isIntegerCastRequired = 1; x->x_typeRequired = A_FLOAT;
            
            } else if (string_containsCharacterAtStart (s, "%")) {
                k = 0;
                
            } else {
                err = PD_ERROR; break;
            }
        }
    }
    //
    }
    
    if (err) { x->x_typeRequired = A_NULL; }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void makefilename_float (t_makefilename *x, t_float f)
{
    t_error err = PD_ERROR;
  
    char name[PD_STRING] = { 0 };
    
    if (x->x_typeRequired == A_NULL)  { err = string_copy (name, PD_STRING, x->x_format->s_name); }
    
    if (x->x_typeRequired == A_FLOAT) {
        if (x->x_isIntegerCastRequired) { 
            err = string_sprintf (name, PD_STRING, x->x_format->s_name, (int)f); 
        } else {
            err = string_sprintf (name, PD_STRING, x->x_format->s_name, f);
        }
    } 
    
    if (!err) { outlet_symbol (x->x_outlet, gensym (name)); }
    else {
        error_invalid (sym_makefilename, sym_substitution);
    }
}

static void makefilename_symbol (t_makefilename *x, t_symbol *s)
{
    t_error err = PD_ERROR;
    
    char name[PD_STRING] = { 0 };
    
    if (x->x_typeRequired == A_NULL)   { err = string_copy (name, PD_STRING, x->x_format->s_name); }
    if (x->x_typeRequired == A_SYMBOL) {
        err = string_sprintf (name, PD_STRING, x->x_format->s_name, s->s_name);
    }
        
    if (!err) { outlet_symbol (x->x_outlet, gensym (name)); }
    else {
        error_invalid (sym_makefilename, sym_substitution);
    }
}

static void makefilename_set (t_makefilename *x, t_symbol *s)
{
    x->x_format = s; if (makefilename_scanFormat (x)) { error_invalid (sym_makefilename, sym_format); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *makefilename_new (t_symbol *s)
{
    t_makefilename *x = (t_makefilename *)pd_new (makefilename_class);
    
    PD_ASSERT (s);
    
    if (s == &s_) { s = sym_file__dot____percent__d; }
        
    x->x_typeRequired           = A_NULL;
    x->x_isIntegerCastRequired  = 0;
    x->x_format                 = s;
    x->x_outlet                 = outlet_new (cast_object (x), &s_symbol);
    
    if (makefilename_scanFormat (x)) { error_invalid (sym_makefilename, sym_format); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void makefilename_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_makefilename,
            (t_newmethod)makefilename_new,
            NULL,
            sizeof (t_makefilename),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addFloat (c, (t_method)makefilename_float);
    class_addSymbol (c, (t_method)makefilename_symbol);
    
    class_addMethod (c, (t_method)makefilename_set, sym_set, A_SYMBOL, A_NULL);
    
    makefilename_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
