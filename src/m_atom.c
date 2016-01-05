
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

void atom_toString (t_atom *a, char *buf, unsigned int bufsize)
{
    char tbuf[30];
    switch(a->a_type)
    {
    case A_SEMICOLON: strcpy(buf, ";"); break;
    case A_COMMA: strcpy(buf, ","); break;
    case A_POINTER:
        strcpy(buf, "(pointer)");
        break;
    case A_FLOAT:
        sprintf(tbuf, "%g", a->a_w.w_float);
        if (strlen(tbuf) < bufsize-1) strcpy(buf, tbuf);
        else if (a->a_w.w_float < 0) strcpy(buf, "-");
        else  strcpy(buf, "+");
        break;
    case A_SYMBOL:
    {
        char *sp;
        unsigned int len;
        int quote;
        for (sp = a->a_w.w_symbol->s_name, len = 0, quote = 0; *sp; sp++, len++)
            if (*sp == ';' || *sp == ',' || *sp == '\\' || 
                (*sp == '$' && sp[1] >= '0' && sp[1] <= '9'))
                quote = 1;
        if (quote)
        {
            char *bp = buf, *ep = buf + (bufsize-2);
            sp = a->a_w.w_symbol->s_name;
            while (bp < ep && *sp)
            {
                if (*sp == ';' || *sp == ',' || *sp == '\\' ||
                    (*sp == '$' && sp[1] >= '0' && sp[1] <= '9'))
                        *bp++ = '\\';
                *bp++ = *sp++;
            }
            if (*sp) *bp++ = '*';
            *bp = 0;
            /* post("quote %s -> %s", a->a_w.w_symbol->s_name, buf); */
        }
        else
        {
            if (len < bufsize-1) strcpy(buf, a->a_w.w_symbol->s_name);
            else
            {
                strncpy(buf, a->a_w.w_symbol->s_name, bufsize - 2);
                strcpy(buf + (bufsize - 2), "*");
            }
        }
    }
        break;
    case A_DOLLAR:
        sprintf(buf, "$%d", a->a_w.w_index);
        break;
    case A_DOLLARSYMBOL:
        strncpy(buf, a->a_w.w_symbol->s_name, bufsize);
        buf[bufsize-1] = 0;
        break;
    default:
        PD_BUG;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
