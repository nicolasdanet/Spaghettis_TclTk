
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error atom_symbolToQuotedString (t_atom *a, char *s, int size)
{
    char *p = NULL;
    int quote = 0;
    t_error err = PD_ERROR_NONE;
    
    if (size < 2) { err = PD_ERROR; }
    else {
    //
    for (p = GET_SYMBOL (a)->s_name; *p; p++) {
        if (utils_isTokenEnd (*p) || utils_isTokenEscape (*p) || dollar_isPointingToDollarAndNumber (p)) {
            quote = 1; break; 
        }
    }
            
    if (quote) {
    
        char *base = s;
        char *last = s + (size - 2);
        
        p = GET_SYMBOL (a)->s_name;
        
        while (base < last && *p) {
        //
        if (utils_isTokenEnd (*p) || utils_isTokenEscape (*p) || dollar_isPointingToDollarAndNumber (p)) {
            *base++ = '\\';
        }
        *base++ = *p++;
        //
        }
        
        if (*p) { err = PD_ERROR; }
        
        *base = 0;

    } else {
        err = string_copy (s, size, GET_SYMBOL (a)->s_name);
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float atom_getFloat (t_atom *a)
{
    if (IS_FLOAT (a)) { return GET_FLOAT (a); }
    else {
        return 0.0;
    }
}

t_float atom_getFloatAtIndex (int n, int argc, t_atom *argv)
{
    if (n >= 0 && n < argc) { return atom_getFloat (argv + n); }
    else {
        return 0.0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *atom_getSymbol (t_atom *a)
{
    if (IS_SYMBOL (a)) { return GET_SYMBOL (a); }
    else { 
        return (&s_);
    }
}

t_symbol *atom_getSymbolAtIndex (int n, int argc, t_atom *argv)
{
    if (n >= 0 && n < argc) { return atom_getSymbol (argv + n); }
    else {
        return (&s_);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_atomtype atom_getType (t_atom *a)
{
    return a->a_type;
}

int atom_typesAreEqual (t_atom *a, t_atom *b)
{
    return (atom_getType (a) == atom_getType (b));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void atom_copyAtomsUnchecked (int argc, t_atom *src, t_atom *dest)
{
    int i; for (i = 0; i < argc; i++) { dest[i] = src[i]; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error atom_withStringUnzeroed (t_atom *a, char *s, int size)
{
    t_buffer *t = buffer_new();
    
    buffer_withStringUnzeroed (t, s, size);
    
    if (buffer_size (t) != 1) { SET_NULL (a); }
    else {
        *a = *(buffer_atoms (t));
    }
    
    buffer_free (t);
    
    if (IS_NULL (a)) { return PD_ERROR; }
    else {
        return PD_ERROR_NONE;
    }
}

t_error atom_toString (t_atom *a, char *dest, int size)
{
    t_error err = PD_ERROR;
    
    PD_ASSERT (size > 0);
    
    switch (a->a_type) {
        case A_SYMBOL       : err = atom_symbolToQuotedString (a, dest, size);                  break;
        case A_FLOAT        : err = string_sprintf (dest, (size_t)size, "%g", GET_FLOAT (a));   break;
        case A_DOLLAR       : err = string_sprintf (dest, (size_t)size, "$%d", GET_DOLLAR (a)); break;
        case A_DOLLARSYMBOL : err = string_copy (dest,  (size_t)size, GET_SYMBOL (a)->s_name);  break;
        case A_SEMICOLON    : err = string_copy (dest,  (size_t)size, ";");                     break;
        case A_COMMA        : err = string_copy (dest,  (size_t)size, ",");                     break;
        case A_POINTER      : err = string_copy (dest,  (size_t)size, s_pointer.s_name);        break;
        default             : PD_BUG;
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_atom *atom_substituteIfPointer (t_atom *a)
{
    if (IS_POINTER (a)) { SET_SYMBOL (a, &s_pointer); } 
    
    return a;
}

char *atom_atomsToString (int argc, t_atom *argv)       /* Caller acquires string ownership. */
{
    char *s = NULL;
    t_buffer *t = buffer_new();
    
    buffer_append (t, argc, argv);
    buffer_toString (t, &s);
    buffer_free (t);
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
