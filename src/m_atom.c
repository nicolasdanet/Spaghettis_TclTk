
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int dollar_getDollarZero (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error atom_symbolToQuotedString (t_atom *a, char *s, int size)
{
    char *p = NULL;
    int quote = 0;
    t_error err = PD_ERROR_NONE;
    
    if (size < 2) { err = PD_ERROR; }
    else {
    //
    for (p = GET_SYMBOL (a)->s_name; *p; p++) {
        if (char_isEnd (*p) || char_isEscape (*p) || string_startWithOneDollarAndOneNumber (p)) {
            quote = 1; break; 
        }
    }
            
    if (quote) {
    
        char *base = s;
        char *last = s + (size - 2);
        
        p = GET_SYMBOL (a)->s_name;
        
        while (base < last && *p) {
        //
        if (char_isEnd (*p) || char_isEscape (*p) || string_startWithOneDollarAndOneNumber (p)) {
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
// MARK: -

t_float atom_getFloat (t_atom *a)
{
    if (IS_FLOAT (a)) { return GET_FLOAT (a); }
    else {
        return (t_float)0.0;
    }
}

t_float atom_getFloatAtIndex (int n, int argc, t_atom *argv)
{
    if (n >= 0 && n < argc) { return atom_getFloat (argv + n); }
    else {
        return (t_float)0.0;
    }
}

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

t_symbol *atom_getDollarSymbol (t_atom *a)
{
    if ((IS_SYMBOL (a) || IS_DOLLARSYMBOL (a))) { return GET_SYMBOL (a); }
    else { 
        return (&s_);
    }
}

t_symbol *atom_getDollarSymbolAtIndex (int n, int argc, t_atom *argv)
{
    if (n >= 0 && n < argc) { return atom_getDollarSymbol (argv + n); }
    else {
        return (&s_);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_atomtype atom_getType (t_atom *a)
{
    return a->a_type;
}

int atom_typesAreEquals (t_atom *a, t_atom *b)
{
    return (atom_getType (a) == atom_getType (b));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atom_copyAtoms (t_atom *src, int m, t_atom *dest, int n)
{
    int size = PD_MIN (m, n); int i; for (i = 0; i < size; i++) { dest[i] = src[i]; }
}

int atom_copyAtomsZeroExpanded (t_atom *src, int m, t_atom *dest, int n, t_glist *glist)
{
    int i;
    int size = PD_MIN (m, n);
    int expanded = 0;
    
    for (i = 0; i < size; i++) {
    //
    t_atom *a = src + i; t_atom *b = dest + i;
    
    if (IS_DOLLARSYMBOL (a) && string_contains (GET_SYMBOL (a)->s_name, "$0")) {
        t_symbol *s = dollar_expandSymbolWithArguments (GET_SYMBOL (a), glist, 0, NULL);
        if (s) { SET_SYMBOL (b, s); expanded = 1; }
        else {
            SET_DOLLARSYMBOL (b, GET_SYMBOL (a));
        }
    } else if (IS_DOLLAR (a) && (GET_DOLLAR (a) == 0)) {
        SET_FLOAT (b, dollar_getDollarZero (glist)); expanded = 1;
    } else {
        *b = *a;
    }
    //
    }
    
    return expanded;
}

int atom_copyAtomsExpandedWithArguments (t_atom *src,
    int m,
    t_atom *dest,
    int n,
    t_glist *glist,
    int argc,
    t_atom *argv)
{
    int i;
    int size = PD_MIN (m, n);
    int expanded = 0;
    
    for (i = 0; i < size; i++) {
    //
    t_atom *a = src + i; t_atom *b = dest + i;
    
    if (IS_DOLLARSYMBOL (a)) {
        t_symbol *s = dollar_expandSymbolWithArguments (GET_SYMBOL (a), glist, argc, argv);
        if (s) { SET_SYMBOL (b, s); expanded = 1; }
        else {
            SET_DOLLARSYMBOL (b, GET_SYMBOL (a));
        }
    } else if (IS_DOLLAR (a)) {
        expanded |= dollar_expandWithArguments (a, b, glist, argc, argv);
    } else {
        *b = *a;
    }
    //
    }
    
    return expanded;
}

int atom_copyAtomsExpanded (t_atom *src, int m, t_atom *dest, int n, t_glist *glist)
{
    t_environment *e = NULL;
    
    if (glist) { e = glist_getEnvironment (glist); }

    if (!e) {
        return atom_copyAtomsExpandedWithArguments (src, m, dest, n, glist, 0, NULL);
    } else {
        return atom_copyAtomsExpandedWithArguments (src,
            m,
            dest,
            n,
            glist,
            environment_getNumberOfArguments (e),
            environment_getArguments (e));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error atom_withStringUnzeroed (t_atom *a, char *s, int size)
{
    t_buffer *t = buffer_new();
    
    buffer_withStringUnzeroed (t, s, size);
    
    if (buffer_getSize (t) != 1) { SET_NULL (a); }
    else {
        *a = *(buffer_getAtoms (t));
    }
    
    buffer_free (t);
    
    if (IS_NULL (a)) { return PD_ERROR; } else { return PD_ERROR_NONE; }
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
// MARK: -

t_atom *atom_substituteIfPointer (t_atom *a)
{
    if (IS_POINTER (a)) { SET_SYMBOL (a, &s_pointer); } 
    
    return a;
}

char *atom_atomsToString (int argc, t_atom *argv)
{
    char *s = NULL;
    t_buffer *t = buffer_new();
    
    buffer_append (t, argc, argv);
    s = buffer_toString (t);
    buffer_free (t);
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
