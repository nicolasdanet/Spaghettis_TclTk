
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

int dollar_startsWithDollarNumber (char *s)
{
    PD_ASSERT (s[0] != 0);
    
    if (s[0] != '$' || s[1] < '0' || s[1] > '9') { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* JMZ:
 * s points to the first character after the $
 * (e.g. if the org.symbol is "$1-bla", then s will point to "1-bla")
 * (e.g. org.symbol="hu-$1mu", s="1mu")
 * LATER: think about more complex $args, like ${$1+3}
 *
 * the return value holds the length of the $arg (in most cases: 1)
 * buf holds the expanded $arg
 *
 * if some error occured, "-1" is returned
 *
 * e.g. "$1-bla" with list "10 20 30"
 * s="1-bla"
 * buf="10"
 * return value = 1; (s+1=="-bla")
 */
static int dollar_expand(char*s, char*buf,t_atom dollar0, int ac, t_atom *av, int tonew)
{
  int argno=atol(s);
  int arglen=0;
  char*cs=s;
  char c=*cs;
  *buf=0;

  while(c&&(c>='0')&&(c<='9')){
    c=*cs++;
    arglen++;
  }

  if (cs==s) { /* invalid $-expansion (like "$bla") */
    sprintf(buf, "$");
    return 0;
  }
  else if (argno < 0 || argno > ac) /* undefined argument */
    {
      if(!tonew)return 0;
      sprintf(buf, "$%d", argno);
    }
  else if (argno == 0){ /* $0 */
    atom_toString(&dollar0, buf, PD_STRING/2-1);
  }
  else{ /* fine! */
    atom_toString(av+(argno-1), buf, PD_STRING/2-1);
  }
  return (arglen-1);
}

/* LATER remove the dependence on the current canvas for $0; should be another
argument. */
t_symbol *dollar_substitute(t_symbol *s, int ac, t_atom *av, int tonew)
{
    char buf[PD_STRING];
    char buf2[PD_STRING];
    char*str=s->s_name;
    char*substr;
    int next=0, i=PD_STRING;
    t_atom dollarnull;
    SET_FLOAT(&dollarnull, canvas_getdollarzero());
    while(i--)buf2[i]=0;

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

    while((next=dollar_expand(str, buf, dollarnull, ac, av, tonew))>=0)
    {
        /*
        * JMZ: i am not sure what this means, so i might have broken it
        * it seems like that if "tonew" is set and the $arg cannot be expanded
        * (or the dollarsym is in reality a A_DOLLAR)
        * 0 is returned from dollar_substitute
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
    return (gensym(buf2));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
