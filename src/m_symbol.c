
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SYMBOL_BIND             "pd-"
#define SYMBOL_BIND_TEMPLATE    "_TEMPLATE_"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *symbol_withAtoms (int argc, t_atom *argv)
{
    t_symbol *s = &s_;
        
    if (argc == 1 && IS_SYMBOL (argv)) { s = GET_SYMBOL (argv); }
    else if (argc) {
        char *t = atom_atomsToString (argc, argv);
        string_removeCharacter (t, '\\');
        s = gensym (t);
        PD_MEMORY_FREE (t);
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that usage of "empty" or "-" as nil tokens is a bad idea. */
/* But it must be kept to ensure compatibility with legacy files. */

t_symbol *symbol_nil (void)
{
    return sym_empty;
}

t_symbol *symbol_dash (void)
{
    return sym___dash__;
}

t_symbol *symbol_emptyAsNil (t_symbol *s)
{
    if (s == &s_) { return sym_empty; }
    else { 
        return s;
    }
}

t_symbol *symbol_emptyAsDash (t_symbol *s)
{
    if (s == &s_) { return sym___dash__; }
    else { 
        return s;
    }
}

int symbol_isNil (t_symbol *s)
{
    return (s == sym_empty);
}

int symbol_isNilOrDash (t_symbol *s)
{
    return (s == sym_empty || s == sym___dash__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *symbol_dollarToHash (t_symbol *s)
{
    if (strlen (s->s_name) >= PD_STRING) { PD_BUG; return s; }
    else {
        char t[PD_STRING + 1] = { 0 };
        string_copy (t, PD_STRING, s->s_name);
        string_replaceCharacter (t, '$', '#');
        return gensym (t);
    }
}

t_symbol *symbol_hashToDollar (t_symbol *s)
{
    if (strlen (s->s_name) >= PD_STRING) { PD_BUG; return s; }
    else {
        char t[PD_STRING + 1] = { 0 };
        string_copy (t, PD_STRING, s->s_name);
        string_replaceCharacter (t, '#', '$');
        return gensym (t);
    }
}

/* Merge two dollars in just one. */

t_symbol *symbol_replaceDoubleDollar (t_symbol *s)
{
    size_t size = strlen (s->s_name);
    
    if (size >= PD_STRING) { PD_BUG; return s; }
    else {
        char t[PD_STRING + 1] = { 0 };
        const char *p = s->s_name;
        int i, j = 0;
        for (i = 0; i < (int)size; i++, j++) {
            t[j] = p[i];
            if (p[i] == '$' && p[i + 1] == '$') { i++; }
        }
        return gensym (t);
    }
}

/* A format to avoid slicing by the string parser. */

t_symbol *symbol_decode (t_symbol *s)
{
    if (!s) { PD_BUG; }
    else {
    //
    const char *p = s->s_name;
    
    PD_ASSERT (strlen (s->s_name) < PD_STRING);
    
    if (*p != '@') { return s; }
    else {
    //
    int i;
    char t[PD_STRING] = { 0 };
    
    p++;
    
    for (i = 0; i < PD_STRING - 1; i++, p++) {
    //
    if (*p == 0)   { break; }
    if (*p == '@') {
        if (p[1] == '_')        { t[i] = ' '; p++; }
        else if (p[1] == '@')   { t[i] = '@'; p++; }
        else if (p[1] == 'c')   { t[i] = ','; p++; }
        else if (p[1] == 's')   { t[i] = ';'; p++; }
        else if (p[1] == 'd')   { t[i] = '$'; p++; }
        else {
            t[i] = *p;
        }
            
    } else { 
        t[i] = *p;
    }
    //
    }
    
    t[i] = 0;
    
    return gensym (t);
    //
    }
    //
    }
    
    return &s_;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *symbol_removeExtension (t_symbol *s)
{
    PD_ASSERT (s);
    
    if (s != &s_) {
    //
    char t[PD_STRING] = { 0 };
    int n = string_indexOfFirstOccurrenceFromEnd (s->s_name, ".");
    t_error err = string_copy (t, PD_STRING, s->s_name);
    PD_ASSERT (!err);
    if (!err && n >= 0) { t[n] = 0; return gensym (t); } 
    //
    }
    
    return s;
}

t_symbol *symbol_addPrefix (t_symbol *s, t_symbol *prefix)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    PD_ASSERT (prefix);
    err = string_sprintf (t, PD_STRING, "%s%s", prefix->s_name, s->s_name);
    PD_UNUSED (err); PD_ASSERT (!err);
    return gensym (t);
}

t_symbol *symbol_addSuffix (t_symbol *s, t_symbol *suffix)
{
    return symbol_addPrefix (suffix, s);
}

t_symbol *symbol_withCopySuffix (t_symbol *s)
{
    PD_ASSERT (s);
    
    int n = string_indexOfFirstOccurrenceFromEnd (s->s_name, "-");
    
    if (n >= 0) {
    //
    int size = (int)strlen (s->s_name) - (n + 1);
        
    if (size > 0) {
    //
    t_atom a; atom_withStringUnzeroed (&a, s->s_name + n + 1, size);
    
    if (IS_FLOAT (&a)) {
        t_error err = PD_ERROR_NONE;
        char t[PD_STRING] = { 0 };
        int k = (int)GET_FLOAT (&a) + 1;
        err |= string_append (t, PD_STRING, s->s_name, n);
        err |= string_addSprintf (t, PD_STRING, "-%d", k);
        PD_UNUSED (err); PD_ASSERT (!err);
        return gensym (t);
    }
    //
    }
    //
    }
    
    return symbol_addSuffix (s, sym___dash__1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *symbol_makeBind (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, SYMBOL_BIND "%s", s->s_name);
    PD_UNUSED (err); PD_ASSERT (!err);
    return gensym (t);
}

t_symbol *symbol_makeBindIfNot (t_symbol *s)
{
    if (string_startWith (s->s_name, SYMBOL_BIND)) { return s; }
    else {
        return symbol_makeBind (s);
    }
}

t_symbol *symbol_makeTemplateIdentifier (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, SYMBOL_BIND_TEMPLATE "%s", s->s_name);
    PD_UNUSED (err); PD_ASSERT (!err);
    return gensym (t);
}

t_symbol *symbol_stripBind (t_symbol *s)
{
    if (string_startWith (s->s_name, SYMBOL_BIND)) { 
        return gensym (s->s_name + strlen (SYMBOL_BIND));
    }
    
    return s;
}

t_symbol *symbol_stripTemplateIdentifier (t_symbol *s)
{
    if (string_startWith (s->s_name, SYMBOL_BIND_TEMPLATE)) {
        return gensym (s->s_name + strlen (SYMBOL_BIND_TEMPLATE)); 
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
