
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* True if the string start with a dollar following by zero or more numbers. */

int dollar_isDollarNumber (char *s)
{
    if (*s != '$') { return 0; } while (*(++s)) { if (*s < '0' || *s > '9') { return 0; } }
    
    return 1;
}

/* True if the string start with a dollar following by one number. */

int dollar_isPointingToDollarAndNumber (char *s)
{
    PD_ASSERT (s[0] != 0);
    
    if (s[0] != '$' || s[1] < '0' || s[1] > '9') { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int dollar_expand (char *s, char *buf, int size, int argc, t_atom *argv)
{
    int n = (int)atol (s);      /* Note that atol return zero for an invalid number. */
    char *ptr = s;
    char c = 0;
    int length = 0;
    t_error err = PD_ERROR_NONE;
    
    *buf = 0;
    
    c = *ptr;
    
    while (c && (c >= '0') && (c <= '9')) { c = *(++ptr); length++; }

    /* Dollar number argument is out of bound. */
    /* Dollar expansion invalid (like "$bla"). */
    /* Dollar zero expansion. */
    /* Dollar number expansion. */
    
    if (n < 0 || n > argc) { return 0; }

    if (ptr == s) {                                       
        err = string_sprintf (buf, size, "$");                  /* Unsubstituted dollars are preserved. */
        return 0;

    } else if (n == 0) {                                    
        t_atom a;
        SET_FLOAT (&a, canvas_getdollarzero());
        err = atom_toString (&a, buf, size);
        PD_ASSERT (length == 1);
        
    } else {                                                
        err = atom_toString (argv + (n - 1), buf, size);
    }
    
    return (err ? -1 : length);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Dollar symbol expansion (e.g. '$1-foo' to 'bar-foo'). */

t_symbol *dollar_expandDollarSymbol (t_symbol *s, int argc, t_atom *argv)
{
    char t[PD_STRING] = { 0 };
    char result[PD_STRING] = { 0 };
    char *str = s->s_name;
    char *substr = NULL;
    int next = 0;
    t_error err = PD_ERROR_NONE;
    
    substr = strchr (str, '$');
    
    if (!substr) { return s; }
    else {
        err |= string_append (result, PD_STRING, str, (substr - str));
        str = substr + 1;
    }

    while (!err && ((next = dollar_expand (str, t, PD_STRING, argc, argv)) >= 0)) {
    //
    if ((next == 0) && (*t == 0)) { return NULL; }          /* Dollar number argument is out of bound. */

    err |= string_add (result, PD_STRING, t);
    str += next;
    
    substr = strchr (str, '$');
    
    if (substr) { err |= string_append (result, PD_STRING, str, (substr - str)); str = substr + 1; }
    else {
        err |= string_add (result, PD_STRING, str);
        break;
    }
    //
    }
    
    if (err) { post_error (PD_TRANSLATE ("$: invalid substitution")); return NULL; }    // --
    else {
        return gensym (result);
    }
}

/* Dollar expansion (e.g. '$1' to 'foo'). */

void dollar_expandDollarNumber (t_atom *dollar, t_atom *a, int argc, t_atom *argv)
{
    int n = GET_DOLLAR (dollar);
        
    PD_ASSERT (IS_DOLLAR (dollar));
    
    if (n > 0 && n <= argc) { *a = *(argv + n - 1); }
    else if (n == 0)        { SET_FLOAT (a, canvas_getdollarzero()); }
    else {
        post_error (PD_TRANSLATE ("$: invalid substitution"));  // --
        SET_FLOAT (a, 0.0);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *dollar_toRaute (t_symbol *s)
{
    char t[PD_STRING + 1] = { 0 };
    char *s1 = NULL;
    char *s2 = NULL;
    
    if (strlen (s->s_name) >= PD_STRING) { PD_BUG; return s; }
    else {
    //
    for (s1 = s->s_name, s2 = t; ; s1++, s2++) {
        if (*s1 == '$') { *s2 = '#'; }
        else if (!(*s2 = *s1)) {
            break;
        }
    }
    
    return (gensym (t));
    //
    }
}

t_symbol *dollar_fromRaute (t_symbol *s)
{
    char t[PD_STRING + 1] = { 0 };
    char *s1 = NULL;
    char *s2 = NULL;
    
    if (strlen (s->s_name) >= PD_STRING) { return s; }
    else {
    //
    for (s1 = s->s_name, s2 = t; ; s1++, s2++) {
        if (*s1 == '#') { *s2 = '$'; }
        else if (!(*s2 = *s1)) {
            break;
        }
    }
    
    return (gensym (t));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
