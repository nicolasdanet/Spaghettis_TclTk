
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int dollar_isDollarNumber (char *s)
{
    if (*s != '$') { return 0; } while (*(++s)) { if (*s < '0' || *s > '9') { return 0; } }
    
    return 1;
}

int dollar_pointsToDollarNumber (char *s)
{
    PD_ASSERT (s[0] != 0);
    
    if (s[0] != '$' || s[1] < '0' || s[1] > '9') { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int dollar_substitute (char *s, char *buf, int size, int argc, t_atom *argv)
{
    int n = (int)atol (s);      /* Note that atol return zero for an invalid number. */
    char *ptr = s;
    char c = 0;
    int length = 0;
    
    *buf = 0;
    
    c = *ptr;
    
    while (c && (c >= '0') && (c <= '9')) { c = *(++ptr); length++; }

    /* Dollar number argument is out of bound. */
    /* Dollar expansion invalid (like "$bla"). */
    /* Dollar zero expansion. */
    /* Dollar number expansion. */
    
    if (n < 0 || n > argc) { return 0; }

    if (ptr == s) {                                       
        utils_snprintf (buf, size, "$");            /* Unsubstituted dollars are preserved. */
        return 0;

    } else if (n == 0) {                                    
        t_atom a;
        SET_FLOAT (&a, canvas_getdollarzero());
        atom_toString (&a, buf, size);
        PD_ASSERT (length == 1);
        
    } else {                                                
        atom_toString (argv + (n - 1), buf, size);
    }
    
    return length;
}

t_symbol *dollar_substituteDollarSymbol (t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *substituted = NULL;
    
    char t[PD_STRING] = { 0 };
    char result[PD_STRING] = { 0 };
    char *str = s->s_name;
    char *substr = NULL;
    int next = 0;
    int err = 0;
    
    if (s && s->s_name) { post_log ("? %s", s->s_name); } else { PD_BUG; }
    
    substr = strchr (str, '$');
    
    if (!substr) { return (s); }
    else {
        err |= utils_strncat (result, PD_STRING, str, (substr - str));
        str = substr + 1;
    }

    while (!err && ((next = dollar_substitute (str, t, PD_STRING, argc, argv)) >= 0)) {
    //
    if ((next == 0) && (*t == 0)) { return NULL; }          /* Dollar number argument is out of bound. */

    err |= utils_strnadd (result, PD_STRING, t);
    str += next;
    
    substr = strchr (str, '$');
    
    if (substr) { err |= utils_strncat (result, PD_STRING, str, (substr - str)); str = substr + 1; }
    else {
        err |= utils_strnadd (result, PD_STRING, str);
        break;
    }
    //
    }
    
    PD_ASSERT (!err);
    
    substituted = gensym (result);
    
    if (substituted && substituted->s_name) { post_log ("! %s", substituted->s_name); } else { PD_BUG; }
    
    return (substituted);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
