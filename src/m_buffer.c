
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
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
#define BUFFER_MAXIMUM_ARGUMENTS        100

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

int buffer_getSize (t_buffer *x)
{
    return x->b_size;
}

t_atom *buffer_getAtoms (t_buffer *x)
{
    return x->b_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_reset (t_buffer *x)
{
    buffer_resize (x, 0);
}

void buffer_append (t_buffer *x, int argc, t_atom *argv)
{
    if (argc > 0) {
    //
    t_atom *a = NULL;
    int i, n = x->b_size + argc;

    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));

    for (a = x->b_vector + x->b_size; argc--; a++) { *a = *(argv++); } x->b_size = n;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_post (t_buffer *x)
{
    post_atoms (x->b_size, x->b_vector);
}

void buffer_resize (t_buffer *x, int n)
{
    PD_ASSERT (n >= 0);
    
    x->b_size   = n;
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));
}

/* < http://stackoverflow.com/a/11270603 > */

void buffer_vAppend (t_buffer *x, char *fmt, ...)
{
    va_list ap;
    t_atom args[BUFFER_MAXIMUM_ARGUMENTS];
    t_atom *a = args;
    int n = 0;
    char *p = fmt;
    int k = 1;
    
    va_start (ap, fmt);
    
    while (k) {

        if (n >= BUFFER_MAXIMUM_ARGUMENTS) { PD_BUG; break; }

        switch (*p++) {
            case 'i'    : SET_FLOAT     (a, va_arg (ap, int));          break;
            case 'f'    : SET_FLOAT     (a, va_arg (ap, double));       break;
            case 's'    : SET_SYMBOL    (a, va_arg (ap, t_symbol *));   break;
            case ';'    : SET_SEMICOLON (a);                            break;
            case ','    : SET_COMMA     (a);                            break;
            default     : k = 0;
        }
        
        if (k) { a++; n++; }
    }
    
    va_end (ap);
    
    buffer_append (x, n, args);
}

void buffer_appendSemicolon (t_buffer *x)
{
    t_atom a;
    SET_SEMICOLON (&a);
    buffer_append (x, 1, &a);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int buffer_isValidCharacter (char c)
{
    return (!utils_isTokenWhitespace (c) && !utils_isTokenEnd (c));
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

void buffer_parseString (t_buffer *x, char *s, int size, int allocated)
{
    int length = 0;
    t_atom *a = NULL;
    
    const char *text = s;
    const char *tBound = s + size;
    
    PD_ASSERT (size > 0);
    
    PD_MEMORY_FREE (x->b_vector, x->b_size * sizeof (t_atom));
    x->b_vector = PD_MEMORY_GET (allocated * sizeof (t_atom));
    a = x->b_vector;
    x->b_size = length;     /* Inconsistency corrected later. */
    
    while (1) {
    //
    while (utils_isTokenWhitespace (*text) && (text != tBound)) { text++; }         /* Skip whitespaces. */
    
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
        
        lastSlash = slash; slash = utils_isTokenEscape (c);

        if (floatState >= 0) { floatState = buffer_nextState (floatState, c); }
        if (!lastSlash && text != tBound && dollar_pointsToDollarNumber (text - 1)) { dollar = 1; }
        
        if (!slash)         { p++; }
        else if (lastSlash) { p++; slash = 0; }
        //
        } while (text != tBound && p != pBound && (slash || (buffer_isValidCharacter (*text))));
                
        *p = 0;

        if (buffer_isValidState (floatState)) {
            SET_FLOAT (a, atof (buf));
                        
        } else if (dollar) {
            if (dollar_isDollarNumber (buf)) { SET_DOLLAR (a, atoi (buf + 1)); }
            else { 
                SET_DOLLARSYMBOL (a, gensym (buf));
            }
            
        } else {
            SET_SYMBOL (a, gensym (buf));
        }
    }
    
    a++;
    length++;
    
    if (text == tBound) { break; }
    
    if (length == allocated) {
        size_t oldSize = allocated * sizeof (t_atom);
        x->b_vector = PD_MEMORY_RESIZE (x->b_vector, oldSize, oldSize * 2);
        allocated = allocated * 2;
        a = x->b_vector + length;
    }
    //
    }
    
    /* Crop to truly used memory. */
    
    x->b_size   = length;
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, allocated * sizeof (t_atom), length * sizeof (t_atom));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_withString (t_buffer *x, char *s, int size)
{
    buffer_parseString (x, s, size, BUFFER_PREALLOCATED_ATOMS);
}

void buffer_toStringUnzeroed (t_buffer *x, char **s, int *size)
{
    char *buf = PD_MEMORY_GET (0);
    int i, length = 0;

    for (i = 0; i < x->b_size; i++) {

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
    }
    
    if (length && buf[length - 1] == ' ') { 
        buf = PD_MEMORY_RESIZE (buf, length, length - 1); length--; 
    }
    
    *s = buf;
    *size = length;
}

void buffer_toString (t_buffer *x, char **s, int *size)
{
    char *buf = NULL;
    int n, length = 0;
    
    buffer_toStringUnzeroed (x, &buf, &length);
    n = length + 1; buf = PD_MEMORY_RESIZE (buf, length, n); buf[n - 1] = 0;
    
    *s = buf; 
    *size = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_serialize (t_buffer *x, t_buffer *y)
{
    t_buffer *copy = buffer_new();
    int i;

    buffer_append (copy, y->b_size, y->b_vector);
    
    for (i = 0; i < copy->b_size; i++) {
    //
    t_atom *a = copy->b_vector + i;
    
    PD_ASSERT (!IS_POINTER (a));
    
    if (!IS_FLOAT (a)) {
        char buf[PD_STRING] = { 0 };
        atom_toString (a, buf, PD_STRING);
        SET_SYMBOL (a, gensym (buf));
    }
    //
    }
    
    buffer_append (x, copy->b_size, copy->b_vector);
}

void buffer_deserialize (t_buffer *x, int argc, t_atom *argv)
{
    int i, n = x->b_size + argc;

    PD_ASSERT (argc >= 0);
    
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));
    
    for (i = 0; i < argc; i++) {
    //
    t_atom *a = x->b_vector + x->b_size + i;
    
    if (!IS_SYMBOL (argv + i)) { *a = *(argv + i); }
    else {
        char *s = GET_SYMBOL (argv + i)->s_name;
        atom_withString (a, s, strlen (s));
        PD_ASSERT (!IS_NULL (a));
    }
    //
    }
    
    x->b_size = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
                if (!(s = dollar_substituteDollarSymbol(at->a_w.w_symbol,
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
                s9 = dollar_substituteDollarSymbol(at->a_w.w_symbol, argc, argv,
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
    
    /*if (!strcmp(filename + strlen(filename) - 4, ".pat") ||
        !strcmp(filename + strlen(filename) - 4, ".mxt"))
    {
        x = binbuf_convert(x, 0);
        deleteit = 1;
    }*/
    
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

/* LATER make this evaluate the file on-the-fly. */
/* LATER figure out how to log errors */
void binbuf_evalfile(t_symbol *name, t_symbol *dir)
{
    t_buffer *b = buffer_new();
    /*int import = !strcmp(name->s_name + strlen(name->s_name) - 4, ".pat") ||
        !strcmp(name->s_name + strlen(name->s_name) - 4, ".mxt");*/
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
        /*if (import)
        {
            t_buffer *newb = binbuf_convert(b, 1);
            buffer_free(b);
            b = newb;
        }*/
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
    int k, n = buffer_getSize(bfrom);
    t_atom *ap = buffer_getAtoms(bfrom), at;
    for (k = 0; k < n; k++)
    {
        if (ap[k].a_type == A_FLOAT ||
            ap[k].a_type == A_SYMBOL &&
                !strchr(ap[k].a_w.w_symbol->s_name, ';') &&
                !strchr(ap[k].a_w.w_symbol->s_name, ',') &&
                !strchr(ap[k].a_w.w_symbol->s_name, '$'))
                    buffer_append(bto, 1, &ap[k]);
        else
        {
            char buf[PD_STRING+1];
            atom_toString(&ap[k], buf, PD_STRING);
            SET_SYMBOL(&at, gensym(buf));
            buffer_append(bto, 1, &at);
        }
    }
    buffer_appendSemicolon(bto);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
