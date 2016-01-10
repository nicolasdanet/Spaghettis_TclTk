
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pd pd_objectMaker;
extern t_pd pd_canvasMaker;
extern int sys_defaultfont;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BUFFER_PREALLOCATED_ATOMS       64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

enum {
    FLOAT_STATE_ERROR                   = -1,
    FLOAT_STATE_START                   =  0,
    FLOAT_STATE_MINUS                   =  1,
    FLOAT_STATE_INTEGER_DIGIT           =  2,
    FLOAT_STATE_LEADING_DOT             =  3,
    FLOAT_STATE_DOT                     =  4,
    FLOAT_STATE_FRACTIONAL_DIGIT        =  5,
    FLOAT_STATE_EXPONENTIAL             =  6,
    FLOAT_STATE_EXPONENTIAL_SIGN        =  7,
    FLOAT_STATE_EXPONENTIAL_DIGIT       =  8
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _buffer {
    int     b_size;
    t_atom  *b_vector;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_buffer *buffer_new (void)
{
    t_buffer *x = (t_buffer *)PD_MEMORY_GET (sizeof (t_buffer));
    x->b_vector = PD_MEMORY_GET (0);
    return x;
}

void buffer_free (t_buffer *x)
{
    PD_MEMORY_FREE (x->b_vector, x->b_size * sizeof (t_atom));
    PD_MEMORY_FREE (x, sizeof (t_buffer));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_clear (t_buffer *x)
{
    x->b_size = 0;
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int buffer_isValidCharacter (char c)
{
    return (!utils_isWhitespace (c) && !utils_isStatementEnd (c));
}

static int buffer_isValidState (int floatState)
{
    return (floatState == FLOAT_STATE_INTEGER_DIGIT // --
            || floatState == FLOAT_STATE_DOT
            || floatState == FLOAT_STATE_FRACTIONAL_DIGIT
            || floatState == FLOAT_STATE_EXPONENTIAL_DIGIT);
}

static int buffer_nextState (int floatState, char c)
{
    int digit       = (c >= '0' && c <= '9');
    int dot         = (c == '.');
    int minus       = (c == '-');
    int plus        = (c == '+');
    int exponential = (c == 'e' || c == 'E');
    
    int k = floatState;
    
    PD_ASSERT (k != FLOAT_STATE_ERROR);
    
    if (floatState == FLOAT_STATE_START)                    {
        if (minus)                                          { k = FLOAT_STATE_MINUS; }
        else if (digit)                                     { k = FLOAT_STATE_INTEGER_DIGIT; }
        else if (dot)                                       { k = FLOAT_STATE_LEADING_DOT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_MINUS)             {
        if (digit)                                          { k = FLOAT_STATE_INTEGER_DIGIT; }
        else if (dot)                                       { k = FLOAT_STATE_LEADING_DOT; }
        else { 
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_INTEGER_DIGIT)     {
        if (dot)                                            { k = FLOAT_STATE_DOT; }
        else if (exponential)                               { k = FLOAT_STATE_EXPONENTIAL; }
        else if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_LEADING_DOT)       {
        if (digit)                                          { k = FLOAT_STATE_FRACTIONAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_DOT)               {
        if (digit)                                          { k = FLOAT_STATE_FRACTIONAL_DIGIT; }
        else if (exponential)                               { k = FLOAT_STATE_EXPONENTIAL; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_FRACTIONAL_DIGIT)  {
        if (exponential)                                    { k = FLOAT_STATE_EXPONENTIAL; }
        else if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL)       {
        if (plus || minus)                                  { k = FLOAT_STATE_EXPONENTIAL_SIGN; }
        else if (digit)                                     { k = FLOAT_STATE_EXPONENTIAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL_SIGN)  {
        if (digit)                                          { k = FLOAT_STATE_EXPONENTIAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL_DIGIT) {
        if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
    }

    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_withString (t_buffer *x, char *s, int size)
{
    int length = 0;
    int allocated = BUFFER_PREALLOCATED_ATOMS;
    t_atom *a = NULL;
    
    const char *text = s;
    const char *tBound = s + size;
    
    PD_ASSERT (size > 0);
    
    PD_MEMORY_FREE (x->b_vector, x->b_size * sizeof (t_atom));
    x->b_vector = PD_MEMORY_GET (allocated * sizeof (t_atom));
    a = x->b_vector;
    x->b_size = length;         /* Inconsistency corrected later. */
    
    while (1) {
    //
    while (utils_isWhitespace (*text) && (text != tBound)) { text++; }   /* Skip whitespaces. */
    
    if (text == tBound)    { break; }
    else if (*text == ';') { SET_SEMICOLON (a); text++; }
    else if (*text == ',') { SET_COMMA (a);     text++; }
    else {
        
        char buf[PD_STRING + 1] = { 0 };
        char *p = buf;
        char *pBound = buf + PD_STRING;
        
        int floatState = 0;
        int slash = 0;
        int lastSlash = 0;
        int dollar = 0;
        
        do {
        //
        char c = *p = *text++;
        
        lastSlash = slash; slash = utils_isEscape (c);

        if (floatState >= 0) { floatState = buffer_nextState (floatState, c); }
        if (!lastSlash && text != tBound && utils_startsWithDollarNumber (text - 1)) { dollar = 1; }
        
        if (!slash)         { p++; }
        else if (lastSlash) { p++; slash = 0; }
        //
        } while (text != tBound && p != pBound && (slash || (buffer_isValidCharacter (*text))));
                
        *p = 0;

        if (buffer_isValidState (floatState)) {
            SET_FLOAT (a, atof (buf));
                        
        } else if (dollar) {
            if (utils_isDollarNumber (buf)) { SET_DOLLAR (a, atoi (buf + 1)); }
            else { 
                SET_DOLLARSYMBOL (a, gensym (buf));
            }
            
        } else {
            SET_SYMBOL (a, gensym (buf));
        }
    }
    
    a++;
    length++;
    
    if (length == allocated) {
        size_t oldSize = allocated * sizeof (t_atom);
        x->b_vector = PD_MEMORY_RESIZE (x->b_vector, oldSize, oldSize * 2);
        allocated = allocated * 2;
        a = x->b_vector + length;
    }
    
    if (text == tBound) { break; }
    //
    }
    
    /* Crop to truly used memory. */
    
    x->b_size   = length;
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, allocated * sizeof (t_atom), length * sizeof (t_atom));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void buffer_toStringUnzero (t_buffer *x, char **s, int *size)
{
    char *buf = PD_MEMORY_GET (0);
    int i, length = 0;

    for (i = 0; i < x->b_size; i++) {
    //
    char t[PD_STRING] = { 0 };
    int n = length;
        
    atom_toString (x->b_vector + i, t, PD_STRING);
    n += strlen (t) + 1;
    buf = PD_MEMORY_RESIZE (buf, length, n);
    strcpy (buf + length, t);
    length = n;
    if (IS_SEMICOLON (x->b_vector + i)) { buf[length - 1] = '\n'; }
    else { 
        buf[length - 1] = ' ';
    }
    //
    }
    
    if (length && buf[length - 1] == ' ') { buf = PD_MEMORY_RESIZE (buf, length, length - 1); length--; }
    
    *s = buf;
    *size = length;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* LATER improve the out-of-space behavior below.  Also fix this so that
writing to file doesn't buffer everything together. */

void binbuf_add(t_buffer *x, int argc, t_atom *argv)
{
    int newsize = x->b_size + argc, i;
    t_atom *ap;
    if (ap = PD_MEMORY_RESIZE(x->b_vector, x->b_size * sizeof(*x->b_vector),
        newsize * sizeof(*x->b_vector)))
            x->b_vector = ap;
    else
    {
        post_error ("binbuf_addmessage: out of space");
        return;
    }
#if 0
    post("binbuf_add: ");
    post_atoms(argc, argv);
#endif
    for (ap = x->b_vector + x->b_size, i = argc; i--; ap++)
        *ap = *(argv++);
    x->b_size = newsize;
}

#define MAXADDMESSV 100
void binbuf_addv(t_buffer *x, char *fmt, ...)
{
    va_list ap;
    t_atom arg[MAXADDMESSV], *at =arg;
    int nargs = 0;
    char *fp = fmt;

    va_start(ap, fmt);
    while (1)
    {
        if (nargs >= MAXADDMESSV)
        {
            post_error ("binbuf_addmessv: only %d allowed", MAXADDMESSV);
            break;
        }
        switch(*fp++)
        {
        case 'i': SET_FLOAT(at, va_arg(ap, int)); break;
        case 'f': SET_FLOAT(at, va_arg(ap, double)); break;
        case 's': SET_SYMBOL(at, va_arg(ap, t_symbol *)); break;
        case ';': SET_SEMICOLON(at); break;
        case ',': SET_COMMA(at); break;
        default: goto done;
        }
        at++;
        nargs++;
    }
done:
    va_end(ap);
    binbuf_add(x, nargs, arg);
}

/* add a binbuf to another one for saving.  Semicolons and commas go to
symbols ";", "'",; and inside symbols, characters ';', ',' and '$' get
escaped.  LATER also figure out about escaping white space */

void binbuf_addbinbuf(t_buffer *x, t_buffer *y)
{
    t_buffer *z = buffer_new();
    int i, fixit;
    t_atom *ap;
    binbuf_add(z, y->b_size, y->b_vector);
    for (i = 0, ap = z->b_vector; i < z->b_size; i++, ap++)
    {
        char tbuf[PD_STRING], *s;
        switch (ap->a_type)
        {
        case A_FLOAT:
            break;
        case A_SEMICOLON:
            SET_SYMBOL(ap, gensym(";"));
            break;
        case A_COMMA:
            SET_SYMBOL(ap, gensym(","));
            break;
        case A_DOLLAR:
            sprintf(tbuf, "$%d", ap->a_w.w_index);
            SET_SYMBOL(ap, gensym(tbuf));
            break;
        case A_DOLLARSYMBOL:
            atom_toString(ap, tbuf, PD_STRING);
            SET_SYMBOL(ap, gensym(tbuf));
            break;
        case A_SYMBOL:
            for (s = ap->a_w.w_symbol->s_name, fixit = 0; *s; s++)
                if (*s == ';' || *s == ',' || *s == '$')
                    fixit = 1;
            if (fixit)
            {
                atom_toString(ap, tbuf, PD_STRING);
                SET_SYMBOL(ap, gensym(tbuf));
            }
            break;
        default:
            PD_BUG;
        }
    }
    
    binbuf_add(x, z->b_size, z->b_vector);
}

void binbuf_addsemi(t_buffer *x)
{
    t_atom a;
    SET_SEMICOLON(&a);
    binbuf_add(x, 1, &a);
}

/* Supply atoms to a binbuf from a message, making the opposite changes
from binbuf_addbinbuf.  The symbol ";" goes to a semicolon, etc. */

void binbuf_restore(t_buffer *x, int argc, t_atom *argv)
{
    int newsize = x->b_size + argc, i;
    t_atom *ap;
    if (ap = PD_MEMORY_RESIZE(x->b_vector, x->b_size * sizeof(*x->b_vector),
        newsize * sizeof(*x->b_vector)))
            x->b_vector = ap;
    else
    {
        post_error ("binbuf_addmessage: out of space");
        return;
    }

    for (ap = x->b_vector + x->b_size, i = argc; i--; ap++)
    {
        if (argv->a_type == A_SYMBOL)
        {
            char *str = argv->a_w.w_symbol->s_name, *str2;
            if (!strcmp(str, ";")) SET_SEMICOLON(ap);
            else if (!strcmp(str, ",")) SET_COMMA(ap);
            else if ((str2 = strchr(str, '$')) && str2[1] >= '0'
                && str2[1] <= '9')
            {
                int dollsym = 0;
                if (*str != '$')
                    dollsym = 1;
                else for (str2 = str + 1; *str2; str2++)
                    if (*str2 < '0' || *str2 > '9')
                {
                    dollsym = 1;
                    break;
                }
                if (dollsym)
                    SET_DOLLARSYMBOL(ap, gensym(str));
                else
                {
                    int dollar = 0;
                    sscanf(argv->a_w.w_symbol->s_name + 1, "%d", &dollar);
                    SET_DOLLAR(ap, dollar);
                }
            }
            else if (strchr(argv->a_w.w_symbol->s_name, '\\'))
            {
                char buf[PD_STRING], *sp1, *sp2;
                int slashed = 0;
                for (sp1 = buf, sp2 = argv->a_w.w_symbol->s_name;
                    *sp2 && sp1 < buf + (PD_STRING-1);
                        sp2++)
                {
                    if (slashed)
                        *sp1++ = *sp2;
                    else if (*sp2 == '\\')
                        slashed = 1;
                    else *sp1++ = *sp2, slashed = 0;
                }
                *sp1 = 0;
                SET_SYMBOL(ap, gensym(buf));
            }
            else *ap = *argv;
            argv++;
        }
        else *ap = *(argv++);
    }
    x->b_size = newsize;
}

void binbuf_print(t_buffer *x)
{
    int i, startedpost = 0, newline = 1;
    for (i = 0; i < x->b_size; i++)
    {
        if (newline)
        {
            post("");
            startedpost = 1;
        }
        post_atoms(1, x->b_vector + i);
        if (x->b_vector[i].a_type == A_SEMICOLON)
            newline = 1;
        else newline = 0; 
    }
}

int binbuf_getnatom(t_buffer *x)
{
    return (x->b_size);
}

t_atom *binbuf_getvec(t_buffer *x)
{
    return (x->b_vector);
}

int binbuf_resize(t_buffer *x, int newsize)
{
    t_atom *new = PD_MEMORY_RESIZE(x->b_vector,
        x->b_size * sizeof(*x->b_vector), newsize * sizeof(*x->b_vector));
    if (new)
        x->b_vector = new, x->b_size = newsize;
    return (new != 0);
}

int canvas_getdollarzero( void);

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
int binbuf_expanddollsym(char*s, char*buf,t_atom dollar0, int ac, t_atom *av, int tonew)
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
t_symbol *binbuf_realizedollsym(t_symbol *s, int ac, t_atom *av, int tonew)
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

    while((next=binbuf_expanddollsym(str, buf, dollarnull, ac, av, tonew))>=0)
    {
        /*
        * JMZ: i am not sure what this means, so i might have broken it
        * it seems like that if "tonew" is set and the $arg cannot be expanded
        * (or the dollarsym is in reality a A_DOLLAR)
        * 0 is returned from binbuf_realizedollsym
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

#define SMALLMSG 5

void binbuf_eval(t_buffer *x, t_pd *target, int argc, t_atom *argv)
{
    t_atom smallstack[SMALLMSG], *mstack, *msp;
    t_atom *at = x->b_vector;
    int ac = x->b_size;
    int nargs, maxnargs = 0;
    if (ac <= SMALLMSG)
        mstack = smallstack;
    else
    {
#if 1
            /* count number of args in biggest message.  The wierd
            treatment of "pd_objectMaker" is because when the message
            goes out to objectmaker, commas and semis are passed
            on as regular args (see below).  We're tacitly assuming here
            that the pd_objectMaker target can't come up via a named
            destination in the message, only because the original "target"
            points there. */
        if (target == &pd_objectMaker)
            maxnargs = ac;
        else
        {
            int i, j = (target ? 0 : -1);
            for (i = 0; i < ac; i++)
            {
                if (at[i].a_type == A_SEMICOLON)
                    j = -1;
                else if (at[i].a_type == A_COMMA)
                    j = 0;
                else if (++j > maxnargs)
                    maxnargs = j;
            }
        }
        if (maxnargs <= SMALLMSG)
            mstack = smallstack;
        else ATOMS_ALLOCA(mstack, maxnargs);
#else
            /* just pessimistically allocate enough to hold everything
            at once.  This turned out to run slower in a simple benchmark
            I tried, perhaps because the extra memory allocation
            hurt the cache hit rate. */
        maxnargs = ac;
        ATOMS_ALLOCA(mstack, maxnargs);
#endif

    }
    msp = mstack;
    while (1)
    {
        t_pd *nexttarget;
            /* get a target. */
        while (!target)
        {
            t_symbol *s;
            while (ac && (at->a_type == A_SEMICOLON || at->a_type == A_COMMA))
                ac--,  at++;
            if (!ac) break;
            if (at->a_type == A_DOLLAR)
            {
                if (at->a_w.w_index <= 0 || at->a_w.w_index > argc)
                {
                    post_error ("$%d: not enough arguments supplied",
                            at->a_w.w_index);
                    goto cleanup; 
                }
                else if (argv[at->a_w.w_index-1].a_type != A_SYMBOL)
                {
                    post_error ("$%d: symbol needed as message destination",
                        at->a_w.w_index);
                    goto cleanup; 
                }
                else s = argv[at->a_w.w_index-1].a_w.w_symbol;
            }
            else if (at->a_type == A_DOLLARSYMBOL)
            {
                if (!(s = binbuf_realizedollsym(at->a_w.w_symbol,
                    argc, argv, 0)))
                {
                    post_error ("$%s: not enough arguments supplied",
                        at->a_w.w_symbol->s_name);
                    goto cleanup;
                }
            }
            else s = atom_getSymbol(at);
            if (!(target = s->s_thing))
            {
                post_error ("%s: no such object", s->s_name);
            cleanup:
                do at++, ac--;
                while (ac && at->a_type != A_SEMICOLON);
                    /* LATER eat args until semicolon and continue */
                continue;
            }
            else
            {
                at++, ac--;
                break;
            }
        }
        if (!ac) break;
        nargs = 0;
        nexttarget = target;
        while (1)
        {
            t_symbol *s9;
            if (!ac) goto gotmess;
            switch (at->a_type)
            {
            case A_SEMICOLON:
                    /* semis and commas in new message just get bashed to
                    a symbol.  This is needed so you can pass them to "expr." */
                if (target == &pd_objectMaker)
                {
                    SET_SYMBOL(msp, gensym(";"));
                    break;
                }
                else
                {
                    nexttarget = 0;
                    goto gotmess;
                }
            case A_COMMA:
                if (target == &pd_objectMaker)
                {
                    SET_SYMBOL(msp, gensym(","));
                    break;
                }
                else goto gotmess;
            case A_FLOAT:
            case A_SYMBOL:
                *msp = *at;
                break;
            case A_DOLLAR:
                if (at->a_w.w_index > 0 && at->a_w.w_index <= argc)
                    *msp = argv[at->a_w.w_index-1];
                else if (at->a_w.w_index == 0)
                    SET_FLOAT(msp, canvas_getdollarzero());
                else
                {
                    if (target == &pd_objectMaker)
                        SET_FLOAT(msp, 0);
                    else
                    {
                        post_error ("$%d: argument number out of range",
                            at->a_w.w_index);
                        SET_FLOAT(msp, 0);
                    }
                }
                break;
            case A_DOLLARSYMBOL:
                s9 = binbuf_realizedollsym(at->a_w.w_symbol, argc, argv,
                    target == &pd_objectMaker);
                if (!s9)
                {
                    post_error ("%s: argument number out of range", at->a_w.w_symbol->s_name);
                    SET_SYMBOL(msp, at->a_w.w_symbol);
                }
                else SET_SYMBOL(msp, s9);
                break;
            default:
                PD_BUG;
                goto broken;
            }
            msp++;
            ac--;
            at++;
            nargs++;
        }
    gotmess:
        if (nargs)
        {
            switch (mstack->a_type)
            {
            case A_SYMBOL:
                pd_message(target, mstack->a_w.w_symbol, nargs-1, mstack+1);
                break;
            case A_FLOAT:
                if (nargs == 1) pd_float(target, mstack->a_w.w_float);
                else pd_list(target, nargs, mstack);
                break;
            }
        }
        msp = mstack;
        if (!ac) break;
        target = nexttarget;
        at++;
        ac--;
    }
broken: 
    if (maxnargs > SMALLMSG)
         ATOMS_FREEA(mstack, maxnargs);
}

int binbuf_read(t_buffer *b, char *filename, char *dirname, int crflag)
{
    long length;
    int fd;
    int readret;
    char *buf;
    char namebuf[PD_STRING];
    
    namebuf[0] = 0;
    if (*dirname)
        strcat(namebuf, dirname), strcat(namebuf, "/");
    strcat(namebuf, filename);
    
    if ((fd = sys_open(namebuf, 0)) < 0)
    {
        fprintf(stderr, "open: ");
        perror(namebuf);
        return (1);
    }
    if ((length = lseek(fd, 0, SEEK_END)) < 0 || lseek(fd, 0, SEEK_SET) < 0 
        || !(buf = PD_MEMORY_GET(length)))
    {
        fprintf(stderr, "lseek: ");
        perror(namebuf);
        close(fd);
        return(1);
    }
    if ((readret = read(fd, buf, length)) < length)
    {
        fprintf(stderr, "read (%d %ld) -> %d\n", fd, length, readret);
        perror(namebuf);
        close(fd);
        PD_MEMORY_FREE(buf, length);
        return(1);
    }
        /* optionally map carriage return to semicolon */
    if (crflag)
    {
        int i;
        for (i = 0; i < length; i++)
            if (buf[i] == '\n')
                buf[i] = ';';
    }
    buffer_withString(b, buf, length);

#if 0
    post("binbuf_read "); post_atoms(b->b_size, b->b_vector);
#endif

    PD_MEMORY_FREE(buf, length);
    close(fd);
    return (0);
}

    /* read a binbuf from a file, via the search patch of a canvas */
int binbuf_read_via_canvas(t_buffer *b, char *filename, t_canvas *canvas,
    int crflag)
{
    int filedesc;
    char buf[PD_STRING], *bufptr;
    if ((filedesc = canvas_open(canvas, filename, "",
        buf, &bufptr, PD_STRING, 0)) < 0)
    {
        post_error ("%s: can't open", filename);
        return (1);
    }
    else close (filedesc);
    if (binbuf_read(b, bufptr, buf, crflag))
        return (1);
    else return (0);
}

    /* old version */
int binbuf_read_via_path(t_buffer *b, char *filename, char *dirname,
    int crflag)
{
    int filedesc;
    char buf[PD_STRING], *bufptr;
    if ((filedesc = open_via_path(
        dirname, filename, "", buf, &bufptr, PD_STRING, 0)) < 0)
    {
        post_error ("%s: can't open", filename);
        return (1);
    }
    else close (filedesc);
    if (binbuf_read(b, bufptr, buf, crflag))
        return (1);
    else return (0);
}

#define WBUFSIZE 4096
static t_buffer *binbuf_convert(t_buffer *oldb, int maxtopd);

    /* write a binbuf to a text file.  If "crflag" is set we suppress
    semicolons. */
int binbuf_write(t_buffer *x, char *filename, char *dir, int crflag)
{
    FILE *f = 0;
    char sbuf[WBUFSIZE], fbuf[PD_STRING], *bp = sbuf, *ep = sbuf + WBUFSIZE;
    t_atom *ap;
    int indx, deleteit = 0;
    int ncolumn = 0;

    fbuf[0] = 0;
    if (*dir)
        strcat(fbuf, dir), strcat(fbuf, "/");
    strcat(fbuf, filename);
    if (!strcmp(filename + strlen(filename) - 4, ".pat") ||
        !strcmp(filename + strlen(filename) - 4, ".mxt"))
    {
        x = binbuf_convert(x, 0);
        deleteit = 1;
    }
    
    if (!(f = sys_fopen(fbuf, "w")))
    {
        fprintf(stderr, "open: ");
        /* sys_unixerror(fbuf); */
        goto fail;
    }
    for (ap = x->b_vector, indx = x->b_size; indx--; ap++)
    {
        int length;
            /* estimate how many characters will be needed.  Printing out
            symbols may need extra characters for inserting backslashes. */
        if (ap->a_type == A_SYMBOL || ap->a_type == A_DOLLARSYMBOL)
            length = 80 + strlen(ap->a_w.w_symbol->s_name);
        else length = 40;
        if (ep - bp < length)
        {
            if (fwrite(sbuf, bp-sbuf, 1, f) < 1)
            {
                /* sys_unixerror(fbuf); */
                goto fail;
            }
            bp = sbuf;
        }
        if ((ap->a_type == A_SEMICOLON || ap->a_type == A_COMMA) &&
            bp > sbuf && bp[-1] == ' ') bp--;
        if (!crflag || ap->a_type != A_SEMICOLON)
        {
            atom_toString(ap, bp, (ep-bp)-2);
            length = strlen(bp);
            bp += length;
            ncolumn += length;
        }
        if (ap->a_type == A_SEMICOLON || (!crflag && ncolumn > 65))
        {
            *bp++ = '\n';
            ncolumn = 0;
        }
        else
        {
            *bp++ = ' ';
            ncolumn++;
        }
    }
    if (fwrite(sbuf, bp-sbuf, 1, f) < 1)
    {
        /* sys_unixerror(fbuf); */
        goto fail;
    }

    if (fflush(f) != 0) 
    {
        /* sys_unixerror(fbuf); */
        goto fail;
    }

    if (deleteit)
        buffer_free(x);
    fclose(f);
    return (0);
fail:
    if (deleteit)
        buffer_free(x);
    if (f)
        fclose(f);
    return (1);
}

/* The following routine attempts to convert from max to pd or back.  The
max to pd direction is working OK but you will need to make lots of 
abstractions for objects like "gate" which don't exist in Pd.  conversion
from Pd to Max hasn't been tested for patches with subpatches yet!  */

#define MAXSTACK 1000

#define ISSYMBOL(a, b) ((a)->a_type == A_SYMBOL && \
    !strcmp((a)->a_w.w_symbol->s_name, (b)))

static t_buffer *binbuf_convert(t_buffer *oldb, int maxtopd)
{
    t_buffer *newb = buffer_new();
    t_atom *vec = oldb->b_vector;
    t_int n = oldb->b_size, nextindex, stackdepth = 0, stack[MAXSTACK],
        nobj = 0, i, gotfontsize = 0;
    t_atom outmess[MAXSTACK], *nextmess;
    t_float fontsize = 10;
    if (!maxtopd)
        binbuf_addv(newb, "ss;", gensym("max"), gensym("v2"));
    for (nextindex = 0; nextindex < n; )
    {
        int endmess, natom;
        char *first, *second, *third;
        for (endmess = nextindex; endmess < n && vec[endmess].a_type != A_SEMICOLON;
            endmess++)
                ;
        if (endmess == n) break;
        if (endmess == nextindex || endmess == nextindex + 1
            || vec[nextindex].a_type != A_SYMBOL ||
                vec[nextindex+1].a_type != A_SYMBOL)
        {
            nextindex = endmess + 1;
            continue;
        }
        natom = endmess - nextindex;
        if (natom > MAXSTACK-10) natom = MAXSTACK-10;
        nextmess = vec + nextindex;
        first = nextmess->a_w.w_symbol->s_name;
        second = (nextmess+1)->a_w.w_symbol->s_name;
        if (maxtopd)
        {
                /* case 1: importing a ".pat" file into Pd. */
                
                /* dollar signs in file translate to symbols */
            for (i = 0; i < natom; i++)
            {
                if (nextmess[i].a_type == A_DOLLAR)
                {
                    char buf[100];
                    sprintf(buf, "$%d", nextmess[i].a_w.w_index);
                    SET_SYMBOL(nextmess+i, gensym(buf));
                }
                else if (nextmess[i].a_type == A_DOLLARSYMBOL)
                {
                    char buf[100];
                    sprintf(buf, "%s", nextmess[i].a_w.w_symbol->s_name);
                    SET_SYMBOL(nextmess+i, gensym(buf));
                }
            }
            if (!strcmp(first, "#N"))
            {
                if (!strcmp(second, "vpatcher"))
                {
                    if (stackdepth >= MAXSTACK)
                    {
                        post_error ("stack depth exceeded: too many embedded patches");
                        return (newb);
                    }
                    stack[stackdepth] = nobj;
                    stackdepth++;
                    nobj = 0;
                    binbuf_addv(newb, "ssfffff;", 
                        gensym("#N"), gensym("canvas"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            atom_getFloatAtIndex(4, natom, nextmess) -
                                atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(5, natom, nextmess) -
                                atom_getFloatAtIndex(3, natom, nextmess),
                            (t_float)sys_defaultfont);
                }
            }
            if (!strcmp(first, "#P"))
            {
                    /* drop initial "hidden" flag */
                if (!strcmp(second, "hidden"))
                {
                    nextmess++;
                    natom--;
                    second = (nextmess+1)->a_w.w_symbol->s_name;
                }
                if (natom >= 7 && !strcmp(second, "newobj")
                    && (ISSYMBOL(&nextmess[6], "patcher") ||
                        ISSYMBOL(&nextmess[6], "p")))
                {
                    binbuf_addv(newb, "ssffss;",
                        gensym("#X"), gensym("restore"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess),
                        gensym("pd"), atom_getSymbolAtIndex(7, natom, nextmess));
                    if (stackdepth) stackdepth--;
                    nobj = stack[stackdepth];
                    nobj++;
                }
                else if (!strcmp(second, "newex") || !strcmp(second, "newobj"))
                {
                    t_symbol *classname =
                        atom_getSymbolAtIndex(6, natom, nextmess);
                    if (classname == gensym("trigger") ||
                        classname == gensym("t"))
                    {
                        for (i = 7; i < natom; i++)
                            if (nextmess[i].a_type == A_SYMBOL &&
                                nextmess[i].a_w.w_symbol == gensym("i"))
                                    nextmess[i].a_w.w_symbol = gensym("f");
                    }
                    if (classname == gensym("table"))
                        classname = gensym("TABLE");
                    SET_SYMBOL(outmess, gensym("#X"));
                    SET_SYMBOL(outmess + 1, gensym("obj"));
                    outmess[2] = nextmess[2];
                    outmess[3] = nextmess[3];
                    SET_SYMBOL(outmess+4, classname);
                    for (i = 7; i < natom; i++)
                        outmess[i-2] = nextmess[i];
                    SET_SEMICOLON(outmess + natom - 2);
                    binbuf_add(newb, natom - 1, outmess);
                    nobj++;
                }
                else if (!strcmp(second, "message") || 
                    !strcmp(second, "comment"))
                {
                    SET_SYMBOL(outmess, gensym("#X"));
                    SET_SYMBOL(outmess + 1, gensym(
                        (strcmp(second, "message") ? "text" : "msg")));
                    outmess[2] = nextmess[2];
                    outmess[3] = nextmess[3];
                    for (i = 6; i < natom; i++)
                        outmess[i-2] = nextmess[i];
                    SET_SEMICOLON(outmess + natom - 2);
                    binbuf_add(newb, natom - 1, outmess);
                    nobj++;
                }
                else if (!strcmp(second, "button"))
                {
                    binbuf_addv(newb, "ssffs;",
                        gensym("#X"), gensym("obj"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess),
                        gensym("bng"));
                    nobj++;
                }
                else if (!strcmp(second, "number") || !strcmp(second, "flonum"))
                {
                    binbuf_addv(newb, "ssff;",
                        gensym("#X"), gensym("floatatom"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess));
                    nobj++;
                }
                else if (!strcmp(second, "slider"))
                {
                    t_float inc = atom_getFloatAtIndex(7, natom, nextmess);
                    if (inc <= 0)
                        inc = 1;
                    binbuf_addv(newb, "ssffsffffffsssfffffffff;",
                        gensym("#X"), gensym("obj"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess),
                        gensym("vsl"),
                        atom_getFloatAtIndex(4, natom, nextmess),
                        atom_getFloatAtIndex(5, natom, nextmess),
                        atom_getFloatAtIndex(6, natom, nextmess),
                        atom_getFloatAtIndex(6, natom, nextmess)
                            + (atom_getFloatAtIndex(5, natom, nextmess) - 1) * inc,
                        0., 0.,
                        gensym("empty"), gensym("empty"), gensym("empty"),
                        0., -8., 0., 8., -262144., -1., -1., 0., 1.);
                    nobj++;
                }
                else if (!strcmp(second, "toggle"))
                {
                    binbuf_addv(newb, "ssffs;",
                        gensym("#X"), gensym("obj"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess),
                        gensym("tgl"));
                    nobj++;
                }
                else if (!strcmp(second, "inlet"))
                {
                    binbuf_addv(newb, "ssffs;",
                        gensym("#X"), gensym("obj"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess),
                        gensym((natom > 5 ? "inlet~" : "inlet"))); 
                    nobj++;
                }
                else if (!strcmp(second, "outlet"))
                {
                    binbuf_addv(newb, "ssffs;",
                        gensym("#X"), gensym("obj"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess),
                        gensym((natom > 5 ? "outlet~" : "outlet"))); 
                    nobj++;
                }
                else if (!strcmp(second, "user"))
                {
                    third = (nextmess+2)->a_w.w_symbol->s_name;
                    if (!strcmp(third, "hslider"))
                    {
                        t_float range = atom_getFloatAtIndex(7, natom, nextmess);
                        t_float multiplier = atom_getFloatAtIndex(8, natom, nextmess);
                        t_float offset = atom_getFloatAtIndex(9, natom, nextmess);
                        binbuf_addv(newb, "ssffsffffffsssfffffffff;",
                                    gensym("#X"), gensym("obj"),
                                    atom_getFloatAtIndex(3, natom, nextmess),
                                    atom_getFloatAtIndex(4, natom, nextmess),
                                    gensym("hsl"),
                                    atom_getFloatAtIndex(6, natom, nextmess),
                                    atom_getFloatAtIndex(5, natom, nextmess),
                                    offset,
                                    range + offset,
                                    0., 0.,
                                    gensym("empty"), gensym("empty"), gensym("empty"),
                                    0., -8., 0., 8., -262144., -1., -1., 0., 1.); 
                   }
                    else if (!strcmp(third, "uslider"))
                    {
                        t_float range = atom_getFloatAtIndex(7, natom, nextmess);
                        t_float multiplier = atom_getFloatAtIndex(8, natom, nextmess);
                        t_float offset = atom_getFloatAtIndex(9, natom, nextmess);
                        binbuf_addv(newb, "ssffsffffffsssfffffffff;",
                                    gensym("#X"), gensym("obj"),
                                    atom_getFloatAtIndex(3, natom, nextmess),
                                    atom_getFloatAtIndex(4, natom, nextmess),
                                    gensym("vsl"),
                                    atom_getFloatAtIndex(5, natom, nextmess),
                                    atom_getFloatAtIndex(6, natom, nextmess),
                                    offset,
                                    range + offset,
                                    0., 0.,
                                    gensym("empty"), gensym("empty"), gensym("empty"),
                                    0., -8., 0., 8., -262144., -1., -1., 0., 1.);
                    }
                    else
                        binbuf_addv(newb, "ssffs;",
                                    gensym("#X"), gensym("obj"),
                                    atom_getFloatAtIndex(3, natom, nextmess),
                                    atom_getFloatAtIndex(4, natom, nextmess),
                                    atom_getSymbolAtIndex(2, natom, nextmess));
                    nobj++;
                }
                else if (!strcmp(second, "connect")||
                    !strcmp(second, "fasten"))
                {
                    binbuf_addv(newb, "ssffff;",
                        gensym("#X"), gensym("connect"),
                        nobj - atom_getFloatAtIndex(2, natom, nextmess) - 1,
                        atom_getFloatAtIndex(3, natom, nextmess),
                        nobj - atom_getFloatAtIndex(4, natom, nextmess) - 1,
                        atom_getFloatAtIndex(5, natom, nextmess)); 
                }
            }
        }
        else        /* Pd to Max */
        {
            if (!strcmp(first, "#N"))
            {
                if (!strcmp(second, "canvas"))
                {
                    t_float x, y;
                    if (stackdepth >= MAXSTACK)
                    {
                        post_error ("stack depth exceeded: too many embedded patches");
                        return (newb);
                    }
                    stack[stackdepth] = nobj;
                    stackdepth++;
                    nobj = 0;
                    if(!gotfontsize) { /* only the first canvas sets the font size */
                        fontsize = atom_getFloatAtIndex(6, natom, nextmess);
                        gotfontsize = 1;
                    }
                    x = atom_getFloatAtIndex(2, natom, nextmess);
                    y = atom_getFloatAtIndex(3, natom, nextmess);
                    binbuf_addv(newb, "ssffff;", 
                        gensym("#N"), gensym("vpatcher"),
                            x, y,
                            atom_getFloatAtIndex(4, natom, nextmess) + x,
                            atom_getFloatAtIndex(5, natom, nextmess) + y);
                }
            }
            if (!strcmp(first, "#X"))
            {
                if (natom >= 5 && !strcmp(second, "restore")
                    && (ISSYMBOL (&nextmess[4], "pd")))
                {
                    binbuf_addv(newb, "ss;", gensym("#P"), gensym("pop"));
                    SET_SYMBOL(outmess, gensym("#P"));
                    SET_SYMBOL(outmess + 1, gensym("newobj"));
                    outmess[2] = nextmess[2];
                    outmess[3] = nextmess[3];
                    SET_FLOAT(outmess + 4, 50.*(natom-5));
                    SET_FLOAT(outmess + 5, fontsize);
                    SET_SYMBOL(outmess + 6, gensym("p"));
                    for (i = 5; i < natom; i++)
                        outmess[i+2] = nextmess[i];
                    SET_SEMICOLON(outmess + natom + 2);
                    binbuf_add(newb, natom + 3, outmess);
                    if (stackdepth) stackdepth--;
                    nobj = stack[stackdepth];
                    nobj++;
                }
                else if (!strcmp(second, "obj"))
                {
                    t_symbol *classname =
                        atom_getSymbolAtIndex(4, natom, nextmess);
                    if (classname == gensym("inlet"))
                        binbuf_addv(newb, "ssfff;", gensym("#P"),
                            gensym("inlet"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            10. + fontsize);
                    else if (classname == gensym("inlet~"))
                        binbuf_addv(newb, "ssffff;", gensym("#P"),
                            gensym("inlet"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            10. + fontsize, 1.);
                    else if (classname == gensym("outlet"))
                        binbuf_addv(newb, "ssfff;", gensym("#P"),
                            gensym("outlet"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            10. + fontsize);
                    else if (classname == gensym("outlet~"))
                        binbuf_addv(newb, "ssffff;", gensym("#P"),
                            gensym("outlet"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            10. + fontsize, 1.);
                    else if (classname == gensym("bng"))
                        binbuf_addv(newb, "ssffff;", gensym("#P"),
                            gensym("button"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            atom_getFloatAtIndex(5, natom, nextmess), 0.);
                    else if (classname == gensym("tgl"))
                        binbuf_addv(newb, "ssffff;", gensym("#P"),
                            gensym("toggle"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            atom_getFloatAtIndex(5, natom, nextmess), 0.);
                    else if (classname == gensym("vsl"))
                        binbuf_addv(newb, "ssffffff;", gensym("#P"),
                            gensym("slider"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            atom_getFloatAtIndex(5, natom, nextmess),
                            atom_getFloatAtIndex(6, natom, nextmess),
                            (atom_getFloatAtIndex(8, natom, nextmess) -
                                atom_getFloatAtIndex(7, natom, nextmess)) /
                                    (atom_getFloatAtIndex(6, natom, nextmess) == 1? 1 :
                                         atom_getFloatAtIndex(6, natom, nextmess) - 1),
                            atom_getFloatAtIndex(7, natom, nextmess));
                    else if (classname == gensym("hsl")) 
                    {
                        t_float slmin = atom_getFloatAtIndex(7, natom, nextmess);
                        t_float slmax = atom_getFloatAtIndex(8, natom, nextmess);
                        binbuf_addv(newb, "sssffffffff;", gensym("#P"),
                            gensym("user"),
                            gensym("hslider"),
                            atom_getFloatAtIndex(2, natom, nextmess),
                            atom_getFloatAtIndex(3, natom, nextmess),
                            atom_getFloatAtIndex(6, natom, nextmess),
                            atom_getFloatAtIndex(5, natom, nextmess),
                            slmax - slmin + 1, /* range */
                            1.,            /* multiplier */
                            slmin,         /* offset */
                            0.);
                    }
                    else if ( (classname == gensym("trigger")) ||
                              (classname == gensym("t")) )
                    {
                        t_symbol *arg;
                        SET_SYMBOL(outmess, gensym("#P"));
                        SET_SYMBOL(outmess + 1, gensym("newex"));
                        outmess[2] = nextmess[2];
                        outmess[3] = nextmess[3];
                        SET_FLOAT(outmess + 4, 50.*(natom-4));
                        SET_FLOAT(outmess + 5, fontsize);
                        outmess[6] = nextmess[4];
                        for (i = 5; i < natom; i++) {
                            arg = atom_getSymbolAtIndex(i, natom, nextmess);
                            if (arg == gensym("a"))
                                SET_SYMBOL(outmess + i + 2, gensym("l"));
                            else if (arg == gensym("anything"))
                                SET_SYMBOL(outmess + i + 2, gensym("l"));
                            else if (arg == gensym("bang"))
                                SET_SYMBOL(outmess + i + 2, gensym("b"));
                            else if (arg == gensym("float"))
                                SET_SYMBOL(outmess + i + 2, gensym("f"));
                            else if (arg == gensym("list"))
                                SET_SYMBOL(outmess + i + 2, gensym("l"));
                            else if (arg == gensym("symbol"))
                                SET_SYMBOL(outmess + i + 2, gensym("s"));
                            else 
                                outmess[i+2] = nextmess[i];
                        }
                        SET_SEMICOLON(outmess + natom + 2);
                        binbuf_add(newb, natom + 3, outmess);
                    }
                    else
                    {
                        SET_SYMBOL(outmess, gensym("#P"));
                        SET_SYMBOL(outmess + 1, gensym("newex"));
                        outmess[2] = nextmess[2];
                        outmess[3] = nextmess[3];
                        SET_FLOAT(outmess + 4, 50.*(natom-4));
                        SET_FLOAT(outmess + 5, fontsize);
                        for (i = 4; i < natom; i++)
                            outmess[i+2] = nextmess[i];
                        if (classname == gensym("osc~"))
                            SET_SYMBOL(outmess + 6, gensym("cycle~"));
                        SET_SEMICOLON(outmess + natom + 2);
                        binbuf_add(newb, natom + 3, outmess);
                    }
                    nobj++;
                
                }
                else if (!strcmp(second, "msg") || 
                    !strcmp(second, "text"))
                {
                    SET_SYMBOL(outmess, gensym("#P"));
                    SET_SYMBOL(outmess + 1, gensym(
                        (strcmp(second, "msg") ? "comment" : "message")));
                    outmess[2] = nextmess[2];
                    outmess[3] = nextmess[3];
                    SET_FLOAT(outmess + 4, 50.*(natom-4));
                    SET_FLOAT(outmess + 5, fontsize);
                    for (i = 4; i < natom; i++)
                        outmess[i+2] = nextmess[i];
                    SET_SEMICOLON(outmess + natom + 2);
                    binbuf_add(newb, natom + 3, outmess);
                    nobj++;
                }
                else if (!strcmp(second, "floatatom"))
                {
                    t_float width = atom_getFloatAtIndex(4, natom, nextmess)*fontsize;
                    if(width<8) width = 150; /* if pd width=0, set it big */
                    binbuf_addv(newb, "ssfff;",
                        gensym("#P"), gensym("flonum"),
                        atom_getFloatAtIndex(2, natom, nextmess),
                        atom_getFloatAtIndex(3, natom, nextmess),
                        width);
                    nobj++;
                }
                else if (!strcmp(second, "connect"))
                {
                    binbuf_addv(newb, "ssffff;",
                        gensym("#P"), gensym("connect"),
                        nobj - atom_getFloatAtIndex(2, natom, nextmess) - 1,
                        atom_getFloatAtIndex(3, natom, nextmess),
                        nobj - atom_getFloatAtIndex(4, natom, nextmess) - 1,
                        atom_getFloatAtIndex(5, natom, nextmess)); 
                }
            }
        }
        nextindex = endmess + 1;
    }
    if (!maxtopd)
        binbuf_addv(newb, "ss;", gensym("#P"), gensym("pop"));
#if 0
    binbuf_write(newb, "import-result.pd", "/tmp", 0);
#endif
    return (newb);
}

/* LATER make this evaluate the file on-the-fly. */
/* LATER figure out how to log errors */
void binbuf_evalfile(t_symbol *name, t_symbol *dir)
{
    t_buffer *b = buffer_new();
    int import = !strcmp(name->s_name + strlen(name->s_name) - 4, ".pat") ||
        !strcmp(name->s_name + strlen(name->s_name) - 4, ".mxt");
    int dspstate = canvas_suspend_dsp();
        /* set filename so that new canvases can pick them up */
    glob_setfilename(0, name, dir);
    if (binbuf_read(b, name->s_name, dir->s_name, 0))
        post_error ("%s: read failed; %s", name->s_name, strerror(errno));
    else
    {
            /* save bindings of symbols #N, #A (and restore afterward) */
        t_pd *bounda = gensym("#A")->s_thing, *boundn = s__N.s_thing;
        gensym("#A")->s_thing = 0;
        s__N.s_thing = &pd_canvasMaker;
        if (import)
        {
            t_buffer *newb = binbuf_convert(b, 1);
            buffer_free(b);
            b = newb;
        }
        binbuf_eval(b, 0, 0, 0);
        gensym("#A")->s_thing = bounda;
        s__N.s_thing = boundn;
    }
    glob_setfilename(0, &s_, &s_);
    buffer_free(b);
    canvas_resume_dsp(dspstate);
}

void global_open(void *dummy, t_symbol *name, t_symbol *dir)
{
    t_pd *x = 0;
        /* even though binbuf_evalfile appears to take care of dspstate,
        we have to do it again here, because canvas_startdsp() assumes
        that all toplevel canvases are visible.  LATER check if this
        is still necessary -- probably not. */

    int dspstate = canvas_suspend_dsp();
    t_pd *boundx = s__X.s_thing;
        s__X.s_thing = 0;       /* don't save #X; we'll need to leave it bound
                                for the caller to grab it. */
    binbuf_evalfile(name, dir);
    while ((x != s__X.s_thing) && s__X.s_thing) 
    {
        x = s__X.s_thing;
        pd_vMessage(x, gensym("pop"), "i", 1);
    }
    pd_performLoadbang();
    canvas_resume_dsp(dspstate);
    s__X.s_thing = boundx;
}

    /* save a text object to a binbuf for a file or copy buf */
void binbuf_savetext(t_buffer *bfrom, t_buffer *bto)
{
    int k, n = binbuf_getnatom(bfrom);
    t_atom *ap = binbuf_getvec(bfrom), at;
    for (k = 0; k < n; k++)
    {
        if (ap[k].a_type == A_FLOAT ||
            ap[k].a_type == A_SYMBOL &&
                !strchr(ap[k].a_w.w_symbol->s_name, ';') &&
                !strchr(ap[k].a_w.w_symbol->s_name, ',') &&
                !strchr(ap[k].a_w.w_symbol->s_name, '$'))
                    binbuf_add(bto, 1, &ap[k]);
        else
        {
            char buf[PD_STRING+1];
            atom_toString(&ap[k], buf, PD_STRING);
            SET_SYMBOL(&at, gensym(buf));
            binbuf_add(bto, 1, &at);
        }
    }
    binbuf_addsemi(bto);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
