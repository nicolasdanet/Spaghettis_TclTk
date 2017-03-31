
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_alloca.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define UTILS_BIND              "pd-"
#define UTILS_BIND_TEMPLATE     "_TEMPLATE_"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *utils_dollarToHash (t_symbol *s)
{
    char t[PD_STRING + 1] = { 0 };
    
    if (strlen (s->s_name) >= PD_STRING) { PD_BUG; return s; }
    else {
        string_copy (t, PD_STRING, s->s_name);
        string_replaceCharacter (t, '$', '#');
        return gensym (t);
    }
}

t_symbol *utils_hashToDollar (t_symbol *s)
{
    char t[PD_STRING + 1] = { 0 };
    
    if (strlen (s->s_name) >= PD_STRING) { PD_BUG; return s; }
    else {
        string_copy (t, PD_STRING, s->s_name);
        string_replaceCharacter (t, '#', '$');
        return gensym (t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void utils_anythingToList (t_pd *x, t_listmethod fn, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    ATOMS_ALLOCA (t, argc + 1);
    atom_copyAtomsUnchecked (argv, argc, t + 1);
    SET_SYMBOL (t, s);
    (*fn) (x, &s_anything, argc + 1, t);
    ATOMS_FREEA (t, argc + 1);
}

t_symbol *utils_gensymWithAtoms (int argc, t_atom *argv)
{
    t_symbol *s = &s_;
        
    if (argc == 1 && IS_SYMBOL (argv)) { s = GET_SYMBOL (argv); }
    else if (argc) {
        char *t = atom_atomsToString (argc, argv);
        s = gensym (t);
        PD_MEMORY_FREE (t);
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *utils_getFirstAtomOfObjectAsSymbol (t_object *x)
{
    return utils_getFirstAtomOfBufferAsSymbol (object_getBuffer (x));
}

t_symbol *utils_getFirstAtomOfBufferAsSymbol (t_buffer *x)
{
    if (x != NULL) {
    //
    int argc = buffer_size (x);
    t_atom *argv = buffer_atoms (x);
    if (argc && IS_SYMBOL (argv)) { return GET_SYMBOL (argv); }
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* A format to avoid slicing by the string parser. */

t_symbol *utils_decode (t_symbol *s)
{
    if (!s) { PD_BUG; }
    else {
    //
    char *p = s->s_name;
    
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
#pragma mark -

t_symbol *utils_dash (void)
{
    return sym___dash__;
}

t_symbol *utils_empty (void)
{
    return sym_empty;
}

t_symbol *utils_substituteIfEmpty (t_symbol *s, int asDash)
{
    if (s == &s_) { return (asDash ? utils_dash() : utils_empty()); }
    else { 
        return s;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *utils_getDefaultBindName (t_class *class, t_symbol *prefix)
{
    int i = 1;
    char t[PD_STRING] = { 0 };
    
    PD_ASSERT (prefix);
    
    while (1) {
        t_error err = string_sprintf (t, PD_STRING, "%s%d", prefix->s_name, i);
        PD_ABORT (err != PD_ERROR_NONE);
        t_symbol *name = gensym (t);
        if (!pd_getThingByClass (name, class)) { return name; }
        i++;
        PD_ABORT (i < 0);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *utils_makeBindSymbol (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, UTILS_BIND "%s", s->s_name);
    PD_ASSERT (!err);
    return gensym (t);
}

t_symbol *utils_makeTemplateIdentifier (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, UTILS_BIND_TEMPLATE "%s", s->s_name);
    PD_ASSERT (!err);
    return gensym (t);
}

t_symbol *utils_stripBindSymbol (t_symbol *s)
{
    if (string_startWith (s->s_name, UTILS_BIND)) { 
        return gensym (s->s_name + strlen (UTILS_BIND));
    }
    
    return s;
}

t_symbol *utils_stripTemplateIdentifier (t_symbol *s)
{
    if (string_startWith (s->s_name, UTILS_BIND_TEMPLATE)) {
        return gensym (s->s_name + strlen (UTILS_BIND_TEMPLATE)); 
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *utils_removeExtension (t_symbol *s)
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

int utils_isNameAllowedForWindow (t_symbol *s)
{
    if (s == sym_Patch)     { return 0; }
    if (s == sym_Scalar)    { return 0; }
    if (s == sym_Text)      { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_isTokenEnd (char c) 
{
    return (c == ',' || c == ';');
}

int utils_isTokenEscape (char c)
{
    return (c == '\\');
}

int utils_isTokenWhitespace (char c)
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_isAlphanumericOrUnderscore (char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_'));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_unique utils_unique (void)
{
    static t_unique unique = 10000;     /* Static. */
    
    unique++;
    
    return (unique == 0 ? (++unique) : unique);        
}

t_error utils_version (char *dest, size_t size)
{
    t_error err = string_sprintf (dest, size, "%s %s / %s / %s", 
                    PD_NAME, 
                    PD_VERSION, 
                    midi_nameNative(), 
                    audio_nameNative());
    
    #if PD_32BIT
        err |= string_add (dest, size, " / 32-bit");
    #endif
    
    #if PD_64BIT
        err |= string_add (dest, size, " / 64-bit");
    #endif
    
    #if PD_WITH_DEBUG
        err |= string_add (dest, size, " / DEBUG");
    #endif

    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
