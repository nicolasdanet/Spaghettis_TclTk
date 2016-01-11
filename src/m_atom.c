
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"

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
    if (n >= 0 && n < argc) { argv += n; return atom_getFloat (argv); }
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
    if (n >= 0 && n < argc) { argv += n; return atom_getSymbol (argv); }
    else {
        return (&s_);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int atom_symbolToQuotedString (t_atom *a, char *s, int size)
{
    char *p = NULL;
    int quote = 0;
    int err = 0;
    
    if (size < 2) { err = 1; }
    else {
    //
    for (p = GET_SYMBOL (a)->s_name; *p; p++) {
        if (utils_isTokenEnd (*p) || utils_isTokenEscape (*p) || utils_startsWithDollarNumber (p)) {
            quote = 1; break; 
        }
    }
            
    if (quote) {
    
        char *base = s;
        char *last = s + (size - 2);
        
        p = GET_SYMBOL (a)->s_name;
        
        while (base < last && *p) {
        //
        if (utils_isTokenEnd (*p) || utils_isTokenEscape (*p) || utils_startsWithDollarNumber (p)) {
            *base++ = '\\';
        }
        *base++ = *p++;
        //
        }
        
        if (*p) { err = 1; }
        
        *base = 0;

    } else {
        err = utils_strncpy (s, size, GET_SYMBOL (a)->s_name);
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void atom_withString (t_atom *a, char *s, int size)
{/*
    t_buffer *t = buffer_new();
    
    buffer_parseString (t, s, size, 1);
    
    if (buffer_getSize (t) != 1) { SET_NULL (a); }
    else {
        *a = *(buffer_getAtoms (t));
    }
    
    buffer_free (t);*/
}

void atom_toString (t_atom *a, char *s, int size)
{
    int err = 1;
    
    PD_ASSERT (size > 0);
    
    switch (a->a_type) {
        case A_SYMBOL       : err = atom_symbolToQuotedString (a, s, size);                     break;
        case A_FLOAT        : err = utils_snprintf (s, (size_t)size, "%g", GET_FLOAT (a));      break;
        case A_DOLLAR       : err = utils_snprintf (s, (size_t)size, "$%d", GET_DOLLAR (a));    break;
        case A_DOLLARSYMBOL : err = utils_strncpy (s,  (size_t)size, GET_SYMBOL (a)->s_name);   break;
        case A_SEMICOLON    : err = utils_strncpy (s,  (size_t)size, ";");                      break;
        case A_COMMA        : err = utils_strncpy (s,  (size_t)size, ",");                      break;
        case A_POINTER      : err = utils_strncpy (s,  (size_t)size, s_pointer.s_name);         break;
    }

    PD_ASSERT (!err);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
