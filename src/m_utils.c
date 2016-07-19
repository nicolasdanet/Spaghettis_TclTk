
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define UTILS_BIND  "pd-"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *utils_getFirstAtomOfObjectAsSymbol (t_object *x)
{
    return utils_getFirstAtomOfBufferAsSymbol (x->te_buffer);
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
    
    return (gensym (t));
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

t_symbol *utils_makeBindSymbol (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, UTILS_BIND "%s", s->s_name);
    PD_ASSERT (!err);
    return (gensym (t));
}

t_symbol *utils_stripBindSymbol (t_symbol *s)
{
    if (string_containsAtStart (s->s_name, UTILS_BIND)) { return gensym (s->s_name + strlen (UTILS_BIND)); }
    
    return s;
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
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')|| (c >='a' && c <='z') || (c == '_'));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_unique utils_unique (void)
{
    static t_unique unique = 10000;     /* Shared. */
    
    unique++;
    
    return (unique == 0 ? ++unique : unique);        
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
