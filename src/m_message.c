
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

#include <stdarg.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MESSAGE_HASH_SIZE           1024                /* Must be a power of two. */
#define MESSAGE_MAXIMUM_RECURSIVE   1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd pd_objectMaker;
extern t_pd pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_symbol s_pointer  = { "pointer"   , NULL, NULL };         /* Shared. */
t_symbol s_float    = { "float"     , NULL, NULL };
t_symbol s_symbol   = { "symbol"    , NULL, NULL };
t_symbol s_bang     = { "bang"      , NULL, NULL };
t_symbol s_list     = { "list"      , NULL, NULL };
t_symbol s_anything = { "anything"  , NULL, NULL };
t_symbol s_signal   = { "signal"    , NULL, NULL };
t_symbol s__N       = { "#N"        , NULL, NULL };
t_symbol s__X       = { "#X"        , NULL, NULL };
t_symbol s_x        = { "x"         , NULL, NULL };
t_symbol s_y        = { "y"         , NULL, NULL };
t_symbol s_         = { ""          , NULL, NULL };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int message_recursiveDepth;                          /* Shared. */

static t_symbol *message_hashTable[MESSAGE_HASH_SIZE];      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.cse.yorku.ca/~oz/hash.html > */

t_symbol *generateSymbol (const char *s, t_symbol *alreadyAllocatedSymbol)
{
    t_symbol **sym1 = NULL;
    t_symbol *sym2  = NULL;
    unsigned int hash = 5381;
    size_t length = 0;
    const char *s2 = s;
    
    while (*s2) {
        hash = ((hash << 5) + hash) + *s2;      /* Bernstein djb2 hash algorithm. */
        length++;
        s2++;
    }
    
    PD_ASSERT (length < PD_STRING);
    
    sym1 = message_hashTable + (hash & (MESSAGE_HASH_SIZE - 1));
    
    while (sym2 = *sym1) {
        if (!strcmp (sym2->s_name, s)) { return sym2; }
        sym1 = &sym2->s_next;
    }
    
    if (alreadyAllocatedSymbol) { sym2 = alreadyAllocatedSymbol; }
    else {
        sym2 = (t_symbol *)getbytes (sizeof (t_symbol));
        sym2->s_name  = (char *)getbytes (length + 1);
        sym2->s_next  = NULL;
        sym2->s_thing = NULL;
        strcpy (sym2->s_name, s);
    }
    
    *sym1 = sym2; 
    
    return sym2;
}

t_symbol *gensym (const char *s)
{
    return (generateSymbol (s, NULL));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void new_anything (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int f;
    char buf[PD_STRING] = { 0 };
    char *name = NULL;
    
    if (message_recursiveDepth > MESSAGE_MAXIMUM_RECURSIVE) { PD_BUG; return; }

    pd_newest = NULL;
    
    if (sys_load_lib (canvas_getcurrent(), s->s_name)) {
        message_recursiveDepth++;
        pd_typedmess (dummy, s, argc, argv);
        message_recursiveDepth--;
        return;
    }

    if ((f = canvas_open (canvas_getcurrent(), s->s_name, ".pd", buf, &name, PD_STRING, 0)) >= 0) {
        
        close (f);
        
        if (pd_setLoadingAbstraction (s)) { 
            post_error ("%s: can't load abstraction within itself\n", s->s_name);
            
        } else {
            t_pd *t = s__X.s_thing;
            canvas_setargs (argc, argv);
            binbuf_evalfile (gensym (name), gensym (buf));
            if (s__X.s_thing && t != s__X.s_thing) { canvas_popabstraction ((t_canvas *)(s__X.s_thing)); }
            else { 
                s__X.s_thing = t; 
            }
            canvas_setargs (0, NULL);
        }

    } else { 
        pd_newest = NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void mess_init(void)
{
    t_symbol *symlist[] = 
    { 
        &s_pointer,
        &s_float,
        &s_symbol,
        &s_bang,
        &s_list,
        &s_anything,
        &s_signal,
        &s__N,
        &s__X,
        &s_x,
        &s_y,
        &s_
    };
    
    t_symbol **sp;
    int i;

    if (pd_objectMaker) return;    
    for (i = sizeof(symlist)/sizeof(*symlist), sp = symlist; i--; sp++)
        (void) generateSymbol((*sp)->s_name, *sp);
    pd_objectMaker = class_new(gensym("objectmaker"), 0, 0, sizeof(t_pd),
        CLASS_DEFAULT, A_NULL);
    pd_canvasMaker = class_new(gensym("classmaker"), 0, 0, sizeof(t_pd),
        CLASS_DEFAULT, A_NULL);
    class_addAnything(pd_objectMaker, (t_method)new_anything);
}



    /* horribly, we need prototypes for each of the artificial function
    calls in pd_typedmess(), to keep the compiler quiet. */
typedef t_pd *(*t_newgimme)(t_symbol *s, int argc, t_atom *argv);
typedef void(*t_messgimme)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

typedef t_pd *(*t_fun0)(
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun1)(t_int i1,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun2)(t_int i1, t_int i2,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun3)(t_int i1, t_int i2, t_int i3,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun4)(t_int i1, t_int i2, t_int i3, t_int i4,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun5)(t_int i1, t_int i2, t_int i3, t_int i4, t_int i5,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);
typedef t_pd *(*t_fun6)(t_int i1, t_int i2, t_int i3, t_int i4, t_int i5, t_int i6,
    t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5);

void pd_typedmess(t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_method *f;
    t_class *c = *x;
    t_methodentry *m;
    t_atomtype *wp, wanttype;
    int i;
    t_int ai[PD_ARGUMENTS+1], *ap = ai;
    t_floatarg ad[PD_ARGUMENTS+1], *dp = ad;
    int narg = 0;
    t_pd *bonzo;
    
        /* check for messages that are handled by fixed slots in the class
        structure.  We don't catch "pointer" though so that sending "pointer"
        to pd_objectMaker doesn't require that we supply a pointer value. */
    if (s == &s_float)
    {
        if (!argc) (*c->c_methodFloat)(x, 0.);
        else if (argv->a_type == A_FLOAT)
            (*c->c_methodFloat)(x, argv->a_w.w_float);
        else goto badarg;
        return;
    }
    if (s == &s_bang)
    {
        (*c->c_methodBang)(x);
        return;
    }
    if (s == &s_list)
    {
        (*c->c_methodList)(x, s, argc, argv);
        return;
    }
    if (s == &s_symbol)
    {
        if (argc && argv->a_type == A_SYMBOL)
            (*c->c_methodSymbol)(x, argv->a_w.w_symbol);
        else
            (*c->c_methodSymbol)(x, &s_);
        return;
    }
    for (i = c->c_methodsSize, m = c->c_methods; i--; m++)
        if (m->me_name == s)
    {
        wp = m->me_arguments;
        if (*wp == A_GIMME)
        {
            if (x == &pd_objectMaker)
                pd_newest = (*((t_newgimme)(m->me_function)))(s, argc, argv);
            else (*((t_messgimme)(m->me_function)))(x, s, argc, argv);
            return;
        }
        if (argc > PD_ARGUMENTS) argc = PD_ARGUMENTS;
        if (x != &pd_objectMaker) *(ap++) = (t_int)x, narg++;
        while (wanttype = *wp++)
        {
            switch (wanttype)
            {
            case A_POINTER:
                if (!argc) goto badarg;
                else
                {
                    if (argv->a_type == A_POINTER)
                        *ap = (t_int)(argv->a_w.w_gpointer);
                    else goto badarg;
                    argc--;
                    argv++;
                }
                narg++;
                ap++;
                break;
            case A_FLOAT:
                if (!argc) goto badarg;
            case A_DEFFLOAT:
                if (!argc) *dp = 0;
                else
                {
                    if (argv->a_type == A_FLOAT)
                        *dp = argv->a_w.w_float;
                    else goto badarg;
                    argc--;
                    argv++;
                }
                dp++;
                break;
            case A_SYMBOL:
                if (!argc) goto badarg;
            case A_DEFSYMBOL:
                if (!argc) *ap = (t_int)(&s_);
                else
                {
                    if (argv->a_type == A_SYMBOL)
                        *ap = (t_int)(argv->a_w.w_symbol);
                            /* if it's an unfilled "dollar" argument it appears
                            as zero here; cheat and bash it to the null
                            symbol.  Unfortunately, this lets real zeros
                            pass as symbols too, which seems wrong... */
                    else if (x == &pd_objectMaker && argv->a_type == A_FLOAT
                        && argv->a_w.w_float == 0)
                        *ap = (t_int)(&s_);
                    else goto badarg;
                    argc--;
                    argv++;
                }
                narg++;
                ap++;
                break;
            default:
                goto badarg;
            }
        }
        switch (narg)
        {
        case 0 : bonzo = (*(t_fun0)(m->me_function))
            (ad[0], ad[1], ad[2], ad[3], ad[4]); break;
        case 1 : bonzo = (*(t_fun1)(m->me_function))
            (ai[0], ad[0], ad[1], ad[2], ad[3], ad[4]); break;
        case 2 : bonzo = (*(t_fun2)(m->me_function))
            (ai[0], ai[1], ad[0], ad[1], ad[2], ad[3], ad[4]); break;
        case 3 : bonzo = (*(t_fun3)(m->me_function))
            (ai[0], ai[1], ai[2], ad[0], ad[1], ad[2], ad[3], ad[4]); break;
        case 4 : bonzo = (*(t_fun4)(m->me_function))
            (ai[0], ai[1], ai[2], ai[3],
                ad[0], ad[1], ad[2], ad[3], ad[4]); break;
        case 5 : bonzo = (*(t_fun5)(m->me_function))
            (ai[0], ai[1], ai[2], ai[3], ai[4],
                ad[0], ad[1], ad[2], ad[3], ad[4]); break;
        case 6 : bonzo = (*(t_fun6)(m->me_function))
            (ai[0], ai[1], ai[2], ai[3], ai[4], ai[5],
                ad[0], ad[1], ad[2], ad[3], ad[4]); break;
        default: bonzo = 0;
        }
        if (x == &pd_objectMaker)
            pd_newest = bonzo;
        return;
    }
    (*c->c_methodAny)(x, s, argc, argv);
    return;
badarg:
    post_error ("Bad arguments for message '%s' to object '%s'",
        s->s_name, c->c_name->s_name);
}

    /* convenience routine giving a stdarg interface to pd_typedmess().  Only
    ten args supported; it seems unlikely anyone will need more since
    longer messages are likely to be programmatically generated anyway. */
void pd_vmess(t_pd *x, t_symbol *sel, char *fmt, ...)
{
    va_list ap;
    t_atom arg[10], *at = arg;
    int nargs = 0;
    char *fp = fmt;

    va_start(ap, fmt);
    while (1)
    {
        if (nargs >= 10)
        {
            post_error ("pd_vmess: only 10 allowed");
            break;
        }
        switch(*fp++)
        {
        case 'f': SET_FLOAT(at, va_arg(ap, double)); break;
        case 's': SET_SYMBOL(at, va_arg(ap, t_symbol *)); break;
        case 'i': SET_FLOAT(at, va_arg(ap, t_int)); break;       
        case 'p': SET_POINTER(at, va_arg(ap, t_gpointer *)); break;
        default: goto done;
        }
        at++;
        nargs++;
    }
done:
    va_end(ap);
    pd_typedmess(x, sel, nargs, arg);
}

void pd_forwardmess(t_pd *x, int argc, t_atom *argv)
{
    if (argc)
    {
        t_atomtype t = argv->a_type;
        if (t == A_SYMBOL) pd_typedmess(x, argv->a_w.w_symbol, argc-1, argv+1);
        else if (t == A_POINTER)
        {
            if (argc == 1) pd_pointer(x, argv->a_w.w_gpointer);
            else pd_list(x, &s_list, argc, argv);
        }
        else if (t == A_FLOAT)
        {
            if (argc == 1) pd_float(x, argv->a_w.w_float);
            else pd_list(x, &s_list, argc, argv);
        }
        else { PD_BUG; }
    }

}

    /* am empty list calls the 'bang' method unless it's the default
    bang method -- that might turn around and call our 'list' method
    which could be an infinite recorsion.  Fall through to calling our
    'anything' method.  That had better not turn around and call us with
    an empty list.  */
    
void nullfn (void) {}

t_gotfn getfn(t_pd *x, t_symbol *s)
{
    t_class *c = *x;
    t_methodentry *m;
    int i;

    for (i = c->c_methodsSize, m = c->c_methods; i--; m++)
        if (m->me_name == s) return(m->me_function);
    post_error ("%s: no method for message '%s'", c->c_name->s_name, s->s_name);
    return((t_gotfn)nullfn);
}

t_gotfn zgetfn(t_pd *x, t_symbol *s)
{
    t_class *c = *x;
    t_methodentry *m;
    int i;

    for (i = c->c_methodsSize, m = c->c_methods; i--; m++)
        if (m->me_name == s) return(m->me_function);
    return(0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
