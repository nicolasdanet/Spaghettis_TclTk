
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"

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

static int dollar_getDollarZero (t_glist *glist)
{
    glist = (glist == NULL) ? canvas_getCurrent() : glist;
    
    {
        t_environment *environment = (glist ? canvas_getEnvironment (glist) : NULL);
        
        if (environment) { return (environment->ce_dollarZeroValue); }
        else {
            return 0;
        }
    }
}

static int dollar_expand (char *s, char *buffer, int size, int argc, t_atom *argv, t_glist *glist)
{
    int n = (int)atol (s);      /* Note that atol return zero for an invalid number. */
    char *ptr = s;
    char c = 0;
    int length = 0;
    t_error err = PD_ERROR_NONE;
    
    *buffer = 0;
    
    c = *ptr;
    
    while (c && (c >= '0') && (c <= '9')) { c = *(++ptr); length++; }

    /* Dollar number argument is out of bound. */
    /* Dollar expansion invalid (like "$bla"). */
    /* Dollar zero expansion. */
    /* Dollar number expansion. */
    
    if (n < 0 || n > argc) { return 0; }

    if (ptr == s) {                                       
        err = string_sprintf (buffer, size, "$");                  /* Unsubstituted dollars are preserved. */
        return 0;

    } else if (n == 0) {                                    
        t_atom a;
        SET_FLOAT (&a, dollar_getDollarZero (glist));
        err = atom_toString (&a, buffer, size);
        PD_ASSERT (length == 1);
        
    } else {                                                
        err = atom_toString (argv + (n - 1), buffer, size);
    }
    
    return (err ? -1 : length);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Dollar symbol expansion (e.g. '$1-foo' to 'bar-foo'). */

t_symbol *dollar_expandDollarSymbol (t_symbol *s, int argc, t_atom *argv, t_glist *glist)
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

    while (!err && ((next = dollar_expand (str, t, PD_STRING, argc, argv, glist)) >= 0)) {
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
    
    if (err) { error_invalid (&s_, sym_expansion); return NULL; }
    else {
        return gensym (result);
    }
}

/* Dollar expansion (e.g. '$1' to 'foo'). */

void dollar_expandDollarNumber (t_atom *dollar, t_atom *a, int argc, t_atom *argv, t_glist *glist)
{
    int n = GET_DOLLAR (dollar);
        
    PD_ASSERT (IS_DOLLAR (dollar));
    
    if (n > 0 && n <= argc) { *a = *(argv + n - 1); }
    else if (n == 0)        { SET_FLOAT (a, dollar_getDollarZero (glist)); }
    else {
        error_invalid (&s_, sym_expansion); SET_FLOAT (a, 0.0);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dollar_copyExpandAtomsByEnvironment (t_atom *src, int m, t_atom *dest, int n, t_glist *glist)
{
    t_environment *environment = NULL;
    
    if (glist) { environment = canvas_getEnvironment (glist); }

    if (!environment) { dollar_copyExpandAtoms (src, m, dest, n, 0, NULL, glist); }
    else {
        dollar_copyExpandAtoms (src, m, dest, n, environment->ce_argc, environment->ce_argv, glist);
    }
}
                                                            
void dollar_copyExpandAtoms (t_atom *src, int m, t_atom *dest, int n, int argc, t_atom *argv, t_glist *glist)
{
    int i;
    int size = PD_MIN (m, n);
    
    for (i = 0; i < size; i++) {

        t_atom *a = src + i;
        t_atom *b = dest + i;
        
        if (IS_FLOAT (a) || IS_SYMBOL (a)) { *b = *a; }
        else if (IS_DOLLAR (a))            { dollar_expandDollarNumber (a, b, argc, argv, glist); }
        else if (IS_DOLLARSYMBOL (a))      {
            t_symbol *s = dollar_expandDollarSymbol (GET_DOLLARSYMBOL (a), argc, argv, glist);
            if (s) { SET_SYMBOL (b, s); } else { SET_SYMBOL (b, GET_DOLLARSYMBOL (a)); }
        } else { 
            PD_BUG; 
        }
    }
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *dollar_toHash (t_symbol *s)
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
    
    return gensym (t);
    //
    }
}

t_symbol *dollar_fromHash (t_symbol *s)
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
    
    return gensym (t);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
