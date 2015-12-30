
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

typedef t_pd *(*t_newgimme)(t_symbol *s, int argc, t_atom *argv);
typedef void (*t_messgimme)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MESSAGE_FLOATARGS   t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -
typedef t_pd *(*t_fun0)(                                            MESSAGE_FLOATARGS);
typedef t_pd *(*t_fun1)(t_int,                                      MESSAGE_FLOATARGS);
typedef t_pd *(*t_fun2)(t_int, t_int,                               MESSAGE_FLOATARGS);
typedef t_pd *(*t_fun3)(t_int, t_int, t_int,                        MESSAGE_FLOATARGS);
typedef t_pd *(*t_fun4)(t_int, t_int, t_int, t_int,                 MESSAGE_FLOATARGS);
typedef t_pd *(*t_fun5)(t_int, t_int, t_int, t_int, t_int,          MESSAGE_FLOATARGS);
typedef t_pd *(*t_fun6)(t_int, t_int, t_int, t_int, t_int, t_int,   MESSAGE_FLOATARGS);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MESSAGE_HASH_SIZE           1024                /* Must be a power of two. */
#define MESSAGE_MAXIMUM_RECURSIVE   1000
#define MESSAGE_MAXIMUM_ARGUMENTS   10

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
        pd_message (dummy, s, argc, argv);
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

void message_initialize (void)
{
    t_symbol *symbols[12] = 
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
    
    if (pd_objectMaker) { PD_BUG; }
    else {
    //
    int i;
    for (i = 0; i < 12; i++) { generateSymbol (symbols[i]->s_name, symbols[i]); }
        
    pd_objectMaker = class_new (gensym ("objectmaker"), NULL, NULL, sizeof (t_pd), CLASS_DEFAULT, A_NULL);
    pd_canvasMaker = class_new (gensym ("classmaker"),  NULL, NULL, sizeof (t_pd), CLASS_DEFAULT, A_NULL);
    
    class_addAnything (pd_objectMaker, (t_method)new_anything);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_message (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_class *c = *x;
    t_methodentry *m = NULL;
    int i;
    
    if (s == &s_float) {
        if (!argc) { (*c->c_methodFloat) (x, 0.0); }
        else if (IS_FLOAT (argv)) { (*c->c_methodFloat) (x, GET_FLOAT (argv)); }
        else { 
            goto err; 
        }
        return;
        
    } else if (s == &s_bang)   {
        (*c->c_methodBang) (x); return;
        
    } else if (s == &s_list)   {
        (*c->c_methodList) (x, s, argc, argv); return;
        
    } else if (s == &s_symbol) {
        if (argc && IS_SYMBOL (argv)) { (*c->c_methodSymbol) (x, GET_SYMBOL (argv)); }
        else {
            (*c->c_methodSymbol) (x, &s_);
        }
        return;
    }
    
    /* Note that "pointer" is not catched. */
    /* A pointer value is not required to create a pointer object. */

    for (i = c->c_methodsSize, m = c->c_methods; i--; m++) {
    //
    if (m->me_name == s) {
    //
    t_atomtype t;
    t_gotfn f = m->me_function;
    t_int ai[PD_ARGUMENTS + 1] = { 0 };
    t_floatarg af[PD_ARGUMENTS + 1] = { 0 };
    t_int *ip = ai;
    t_floatarg *fp = af;
    int n = 0;
    t_pd *o = NULL;
    
    t_atomtype *p = m->me_arguments;
    
    if (*p == A_GIMME) {
        if (x == &pd_objectMaker) { pd_newest = (*((t_newgimme)m->me_function)) (s, argc, argv); }
        else { 
            (*((t_messgimme)m->me_function)) (x, s, argc, argv); 
        }
        return;
    }
    
    if (x != &pd_objectMaker) { *(ip++) = (t_int)x; n++;     }
    if (argc > PD_ARGUMENTS)  { PD_BUG; argc = PD_ARGUMENTS; }
        
    while (t = *p++) {
    //
    switch (t) {
    //
    case A_POINTER   :  if (!argc) { goto err; }
                        else {
                            if (IS_POINTER (argv)) { *ip = (t_int)GET_POINTER (argv); }
                            else { 
                                goto err; 
                            }
                            argc--; argv++;
                        }
                        n++; ip++; break;
                        
    case A_FLOAT     :  if (!argc) { goto err;  }               /* Notice missing break. */
    case A_DEFFLOAT  :  if (!argc) { *fp = 0.0; }
                        else {
                            if (IS_FLOAT (argv)) { *fp = GET_FLOAT (argv); }
                            else { 
                                goto err; 
                            }
                            argc--; argv++;
                        }
                        fp++; break;
                        
    case A_SYMBOL    :  if (!argc) { goto err; }                /* Ditto. */
    case A_DEFSYMBOL :  if (!argc) { *ip = (t_int)&s_; }
                        else {
                            if (IS_SYMBOL (argv)) { *ip = (t_int)GET_SYMBOL (argv); }
                            else if (x == &pd_objectMaker && IS_FLOAT (argv) && GET_FLOAT (argv) == 0.0) {
                                *ip = (t_int)&s_;
                            } else { 
                                goto err;
                            }
                            argc--; argv++;
                        }
                        n++; ip++; break;
                            
    default          :  goto err;
    //
    }
    //
    }

    switch (n) {
    //
    case 0 : o = (*(t_fun0)f) (af[0], af[1], af[2], af[3], af[4]);
             break;
    case 1 : o = (*(t_fun1)f) (ai[0], af[0], af[1], af[2], af[3], af[4]);
             break;
    case 2 : o = (*(t_fun2)f) (ai[0], ai[1], af[0], af[1], af[2], af[3], af[4]);
             break;
    case 3 : o = (*(t_fun3)f) (ai[0], ai[1], ai[2], af[0], af[1], af[2], af[3], af[4]);
             break;
    case 4 : o = (*(t_fun4)f) (ai[0], ai[1], ai[2], ai[3], af[0], af[1], af[2], af[3], af[4]);
             break;
    case 5 : o = (*(t_fun5)f) (ai[0], ai[1], ai[2], ai[3], ai[4], af[0], af[1], af[2], af[3], af[4]);
             break;
    case 6 : o = (*(t_fun6)f) (ai[0], ai[1], ai[2], ai[3], ai[4], ai[5], af[0], af[1], af[2], af[3], af[4]);
             break;
    //
    }
    
    if (x == &pd_objectMaker) { pd_newest = o; }
    
    return;
    //
    }
    //
    }
    
    (*c->c_methodAny) (x, s, argc, argv);
    
    return;
    
err:
    post_error ("%s: bad arguments for method \"%s\"", c->c_name->s_name, s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://stackoverflow.com/a/11270603 > */

void pd_variadicMessage (t_pd *x, t_symbol *s, char *fmt, ...)
{
    va_list ap;
    t_atom arg[MESSAGE_MAXIMUM_ARGUMENTS] = { 0 };
    t_atom *a = arg;
    int n = 0;
    char *p = fmt;
    int k = 1; 
    
    va_start (ap, fmt);
    
    while (k) {
    
        if (n >= MESSAGE_MAXIMUM_ARGUMENTS) { PD_BUG; break; }
        
        switch (*p++) {
            case 'f'    : SET_FLOAT   (a, va_arg (ap, double));       break;
            case 's'    : SET_SYMBOL  (a, va_arg (ap, t_symbol *));   break;
            case 'i'    : SET_FLOAT   (a, va_arg (ap, t_int));        break;       
            case 'p'    : SET_POINTER (a, va_arg (ap, t_gpointer *)); break;
            default     : k = 0;
        }
        
        if (k) { a++; n++; }
    }
    
    va_end (ap);
    
    pd_message (x, s, n, arg);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_nothing (void *x)
{
    (void)x;
}

t_gotfn getfn (t_pd *x, t_symbol *s)
{
    t_class *c = *x;
    t_methodentry *m;
    int i;

    for (i = c->c_methodsSize, m = c->c_methods; i--; m++) {
        if (m->me_name == s) { return (m->me_function); }
    }
    
    PD_BUG;
    
    return pd_nothing;
}

t_gotfn zgetfn (t_pd *x, t_symbol *s)
{
    t_class *c = *x;
    t_methodentry *m;
    int i;

    for (i = c->c_methodsSize, m = c->c_methods; i--; m++) {
        if (m->me_name == s) { return (m->me_function); }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
