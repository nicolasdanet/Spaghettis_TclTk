
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

static int dollar_substitute (char *s, char *buf, int size, int argc, t_atom *argv, int tonew)
{
    int n = (int)atol (s);
    char *ptr = s;
    char c = 0;
    int length = 0;
    
    *buf = 0;
    c = *ptr;
    
    while (c && (c >= '0') && (c <= '9')) { c = *ptr++; length++; }

    /* Dollar expansion invalid (like "$bla"). */
    /* Dollar number argument is out of bound. */
    /* Dollar zero expansion. */
    /* Dollar number expansion. */
    
    if (ptr == s) {                                         
        int err = utils_snprintf (buf, size, "$");
        PD_ASSERT (!err);
        return 0;
        
    } else if (n < 0 || n > argc) {                         
        if (!tonew) { return 0; } 
        else {
            int err = utils_snprintf (buf, size, "$%d", n);
            PD_ASSERT (!err);
        }
        
    } else if (n == 0) {                                    
        t_atom a;
        SET_FLOAT (&a, canvas_getdollarzero());
        atom_toString (&a, buf, size);
        
    } else {                                                
        atom_toString (argv + (n - 1), buf, size);
    }
    
    return (length - 1);
}

t_symbol *dollar_substituteDollarSymbol (t_symbol *s, int argc, t_atom *argv, int tonew)
{
    char buf[PD_STRING] = { 0 };
    char buf2[PD_STRING] = { 0 };
    char*str=s->s_name;
    char*substr;
    int next=0, i=PD_STRING;

    while(i--)buf2[i]=0;

    t_symbol *sym = NULL;
    
    if (s && s->s_name) { post_log ("? %s", s->s_name); }
    else {
        PD_BUG;
    }
    
#if 1
    /* JMZ: currently, a symbol is detected to be A_DOLLARSYMBOL if it starts with '$'
     * the leading $ is stripped and the rest stored in "s"
     * i would suggest to NOT strip the leading $
     * and make everything a A_DOLLARSYMBOL that contains(!) a $
     *
     * whenever this happened, enable this code
     */
    substr=strchr(str, '$');
    if (!substr || substr-str >= PD_STRING)
        return (s);

    strncat(buf2, str, (substr-str));
    str=substr+1;

#endif

    while((next=dollar_substitute (str, buf, PD_STRING, argc, argv, tonew))>=0)
    {
        /*
        * JMZ: i am not sure what this means, so i might have broken it
        * it seems like that if "tonew" is set and the $arg cannot be expanded
        * (or the dollarsym is in reality a A_DOLLAR)
        * 0 is returned from dollar_substituteDollarSymbol
        * this happens, when expanding in a message-box, but does not happen
        * when the A_DOLLARSYMBOL is the name of a subpatch
        */
        if(!tonew&&(0==next)&&(0==*buf))
        {
            return 0; /* JMZ: this should mimick the original behaviour */
        }

        strncat(buf2, buf, PD_STRING/2-1);
        str+=next;
        substr=strchr(str, '$');
        if(substr)
        {
            strncat(buf2, str, (substr-str));
            str=substr+1;
        } 
        else
        {
            strcat(buf2, str);
            goto done;
        }
    }
done:
    sym = gensym (buf2);
    
    if (sym && sym->s_name) { post_log ("! %s", sym->s_name); }
    else {
        PD_BUG;
    }
    
    return (sym);
}

/*
t_symbol *dollar_substituteDollarSymbol (t_symbol *s, int argc, t_atom *argv, int tonew)
{
    char buf[PD_STRING] = { 0 };
    char result[PD_STRING] = { 0 };
    char *str = s->s_name;
    char *substr = NULL;
    int next = 0;

    substr = strchr (str, '$');
    
    if (!substr) { return s; }
    if ((substr - str) >= PD_STRING) { return s; }

    strncat (result, str, (substr - str));
    str = substr + 1;

    while ((next = dollar_substitute (str, buf, PD_STRING, argc, argv, tonew)) >= 0) {

        if(!tonew&&(0==next)&&(0==*buf))
        {
            return 0; 
        }

        strncat(result, buf, PD_STRING/2-1);
        str+=next;
        substr=strchr(str, '$');
        if(substr)
        {
            strncat(result, str, (substr-str));
            str=substr+1;
        } 
        else
        {
            strcat(result, str);
            goto done;
        }
        
    }
    
done:
    return gensym (result);
}
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
