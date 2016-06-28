
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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

typedef void (*t_gimme)     (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef t_pd *(*t_newgimme) (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MESSAGE_FLOATS  t_float, t_float, t_float, t_float, t_float

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_method0) (                                                 MESSAGE_FLOATS);
typedef void (*t_method1) (t_int,                                           MESSAGE_FLOATS);
typedef void (*t_method2) (t_int, t_int,                                    MESSAGE_FLOATS);
typedef void (*t_method3) (t_int, t_int, t_int,                             MESSAGE_FLOATS);
typedef void (*t_method4) (t_int, t_int, t_int, t_int,                      MESSAGE_FLOATS);
typedef void (*t_method5) (t_int, t_int, t_int, t_int, t_int,               MESSAGE_FLOATS);
typedef void (*t_method6) (t_int, t_int, t_int, t_int, t_int, t_int,        MESSAGE_FLOATS);

typedef t_pd *(*t_newmethod0) (                                             MESSAGE_FLOATS);
typedef t_pd *(*t_newmethod1) (t_int,                                       MESSAGE_FLOATS);
typedef t_pd *(*t_newmethod2) (t_int, t_int,                                MESSAGE_FLOATS);
typedef t_pd *(*t_newmethod3) (t_int, t_int, t_int,                         MESSAGE_FLOATS);
typedef t_pd *(*t_newmethod4) (t_int, t_int, t_int, t_int,                  MESSAGE_FLOATS);
typedef t_pd *(*t_newmethod5) (t_int, t_int, t_int, t_int, t_int,           MESSAGE_FLOATS);
typedef t_pd *(*t_newmethod6) (t_int, t_int, t_int, t_int, t_int, t_int,    MESSAGE_FLOATS);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MESSAGE_HASH_SIZE           1024                    /* Must be a power of two. */
#define MESSAGE_MAXIMUM_ARGUMENTS   10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd pd_objectMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol *message_hashTable[MESSAGE_HASH_SIZE];      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.cse.yorku.ca/~oz/hash.html > */

t_symbol *generateSymbol (const char *s, t_symbol *alreadyAllocatedSymbol)
{
    t_symbol *sym   = NULL;
    t_symbol **next = NULL;
    unsigned int hash = 5381;
    size_t length = 0;
    const char *s2 = s;
    
    PD_ASSERT (s != NULL);
    
    while (*s2) {
        hash = ((hash << 5) + hash) + *s2;      /* Bernstein djb2 hash algorithm. */
        length++;
        s2++;
    }
    
    PD_ASSERT (length < PD_STRING);
    
    next = message_hashTable + (hash & (MESSAGE_HASH_SIZE - 1));
    
    while (sym = *next) {
        if (!strcmp (sym->s_name, s)) { return sym; }
        next = &sym->s_next;
    }
    
    if (alreadyAllocatedSymbol) { sym = alreadyAllocatedSymbol; }
    else {
        sym = (t_symbol *)PD_MEMORY_GET (sizeof (t_symbol));
        sym->s_name  = (char *)PD_MEMORY_GET (length + 1);
        sym->s_next  = NULL;
        sym->s_thing = NULL;
        strcpy (sym->s_name, s);
    }
    
    *next = sym; 
    
    return sym;
}

t_symbol *gensym (const char *s)
{
    return (generateSymbol (s, NULL));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_initialize (void)
{
    t_symbol *symbols[11] = 
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
            &s__A,
            &s_
        };
    
    int i;
    for (i = 0; i < MESSAGE_HASH_SIZE; i++) { PD_ASSERT (message_hashTable[i] == NULL); }
    for (i = 0; i < 11; i++) { generateSymbol (symbols[i]->s_name, symbols[i]); }
    
    symbols_initialize();
    
    PD_ASSERT (sym___hash__N == &s__N);
    PD_ASSERT (sym___hash__X == &s__X);
    PD_ASSERT (sym___hash__A == &s__A);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int message_isStaticSymbol (t_symbol *s)
{
    t_symbol *symbols[11] = 
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
            &s__A,
            &s_
        };
    
    int i;
    for (i = 0; i < 11; i++) { if (s == symbols[i]) { return 1; } }
    
    return 0;
}

void message_release (void)
{
    t_symbol *sym1 = NULL;
    t_symbol *sym2 = NULL;
    
    int i;
    
    for (i = 0; i < MESSAGE_HASH_SIZE; i++) {
    //
    sym1 = message_hashTable[i];
    
    while ((sym2 = sym1)) {
        sym1 = sym2->s_next;
        if (!message_isStaticSymbol (sym2)) {
            PD_MEMORY_FREE (sym2->s_name); PD_MEMORY_FREE (sym2); 
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pd_messageExecuteObject (t_pd *x, t_method f, int n, t_int *ai, t_float *af)
{
    switch (n) {
    //
    case 0 : 
        (*(t_method0)f) (af[0], af[1], af[2], af[3], af[4]);
        break;
    case 1 : 
        (*(t_method1)f) (ai[0], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 2 : 
        (*(t_method2)f) (ai[0], ai[1], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 3 : 
        (*(t_method3)f) (ai[0], ai[1], ai[2], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 4 : 
        (*(t_method4)f) (ai[0], ai[1], ai[2], ai[3], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 5 : 
        (*(t_method5)f) (ai[0], ai[1], ai[2], ai[3], ai[4], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 6 : 
        (*(t_method6)f) (ai[0], ai[1], ai[2], ai[3], ai[4], ai[5], af[0], af[1], af[2], af[3], af[4]);
        break;
    //
    }
}

static void pd_messageExecuteMaker (t_pd *x, t_method f, int n, t_int *ai, t_float *af)
{
    t_pd *o = NULL;

    switch (n) {
    //
    case 0 : 
        o = (*(t_newmethod0)f) (af[0], af[1], af[2], af[3], af[4]);
        break;
    case 1 : 
        o = (*(t_newmethod1)f) (ai[0], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 2 : 
        o = (*(t_newmethod2)f) (ai[0], ai[1], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 3 : 
        o = (*(t_newmethod3)f) (ai[0], ai[1], ai[2], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 4 : 
        o = (*(t_newmethod4)f) (ai[0], ai[1], ai[2], ai[3], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 5 : 
        o = (*(t_newmethod5)f) (ai[0], ai[1], ai[2], ai[3], ai[4], af[0], af[1], af[2], af[3], af[4]);
        break;
    case 6 : 
        o = (*(t_newmethod6)f) (ai[0], ai[1], ai[2], ai[3], ai[4], ai[5], af[0], af[1], af[2], af[3], af[4]);
        break;
    //
    }
    
    pd_newest = o;
}

static void pd_messageExecute (t_pd *x, t_method f, int n, t_int *ai, t_float *af)
{
    if (x == &pd_objectMaker) { pd_messageExecuteMaker (x, f, n, ai, af); }
    else {
        pd_messageExecuteObject (x, f, n, ai, af);
    }
}

static t_error pd_messageSlots (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    t_class *c = pd_class (x);
        
    if (s == &s_float) {
        if (!argc) { (*c->c_methodFloat) (x, 0.0); }
        else if (IS_FLOAT (argv)) { (*c->c_methodFloat) (x, GET_FLOAT (argv)); }
        else { 
            err = PD_ERROR; 
        }
        
    } else if (s == &s_bang)   {
        (*c->c_methodBang) (x);
        
    } else if (s == &s_list)   {
        (*c->c_methodList) (x, s, argc, argv);
        
    } else if (s == &s_symbol) {
        if (argc && IS_SYMBOL (argv)) { (*c->c_methodSymbol) (x, GET_SYMBOL (argv)); }
        else {
            (*c->c_methodSymbol) (x, &s_);
        }
    }
    
    return err;
}

static t_error pd_messageMethods (t_entry *m, t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atomtype t;
    t_method f = m->me_method;
    t_int   ai[PD_ARGUMENTS + 1] = { 0 };
    t_float af[PD_ARGUMENTS + 1] = { 0 };
    t_int   *ip = ai;
    t_float *fp = af;
    int n = 0;
    
    t_atomtype *p = m->me_arguments;
    
    if (*p == A_GIMME) {
        if (x == &pd_objectMaker) { pd_newest = (*((t_newgimme)m->me_method)) (s, argc, argv); }
        else { 
            (*((t_gimme)m->me_method)) (x, s, argc, argv); 
        }
        return PD_ERROR_NONE;
    }
    
    if (x != &pd_objectMaker) { *ip = (t_int)x; ip++; n++;   }
    if (argc > PD_ARGUMENTS)  { PD_BUG; argc = PD_ARGUMENTS; }
        
    while (t = *p++) {
    //
    switch (t) {
    //
    case A_POINTER   :  if (!argc) { return PD_ERROR; }
                        else {
                            if (IS_POINTER (argv)) { *ip = (t_int)GET_POINTER (argv); }
                            else { 
                                return PD_ERROR; 
                            }
                            argc--; argv++;
                        }
                        n++; ip++; break;
                        
    case A_FLOAT     :  if (!argc) { return PD_ERROR; }         /* Notice that break is missing. */
    case A_DEFFLOAT  :  if (!argc) { *fp = 0.0; }
                        else {
                            if (IS_FLOAT (argv)) { *fp = GET_FLOAT (argv); }
                            else { 
                                return PD_ERROR; 
                            }
                            argc--; argv++;
                        }
                        fp++; break;
                        
    case A_SYMBOL    :  if (!argc) { return PD_ERROR;  }        /* Ditto. */
    case A_DEFSYMBOL :  if (!argc) { *ip = (t_int)&s_; }
                        else {
                            if (IS_SYMBOL (argv)) { *ip = (t_int)GET_SYMBOL (argv); }
                            else { 
                                return PD_ERROR;
                            }
                            argc--; argv++;
                        }
                        n++; ip++; break;
                            
    default          :  return PD_ERROR;
    //
    }
    //
    }

    pd_messageExecute (x, f, n, ai, af);
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_message (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;

    t_class *c = pd_class (x);
        
    /* Note that "pointer" is not catched. */

    if (s == &s_bang || s == &s_float || s == &s_symbol || s == &s_list) {
        err = pd_messageSlots (x, s, argc, argv);
        if (!err) { 
            return; 
        }  
    }
    
    if (!err) {
    
        t_entry *m = NULL;
        int i;
            
        for (i = c->c_methodsSize, m = c->c_methods; i--; m++) {
            if (m->me_name == s) {
                err = pd_messageMethods (m, x, s, argc, argv); 
                if (!err) { return; } 
                else {
                    break;
                }
            }
        }
        
        if (!err) {
            (*c->c_methodAnything) (x, s, argc, argv); 
            return; 
        }
    }

    post_error (PD_TRANSLATE ("%s: bad arguments for method %s"), class_getName (c), s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://stackoverflow.com/a/11270603 > */

void pd_vMessage (t_pd *x, t_symbol *s, char *fmt, ...)
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
