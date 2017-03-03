
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_gimme)     (t_pd *, t_symbol *, int, t_atom *);
typedef t_pd *(*t_newgimme) (t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define METHOD_FLOATS   t_float, t_float, t_float, t_float, t_float

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_method0) (                                                 METHOD_FLOATS);     // --
typedef void (*t_method1) (t_int,                                           METHOD_FLOATS);
typedef void (*t_method2) (t_int, t_int,                                    METHOD_FLOATS);
typedef void (*t_method3) (t_int, t_int, t_int,                             METHOD_FLOATS);
typedef void (*t_method4) (t_int, t_int, t_int, t_int,                      METHOD_FLOATS);
typedef void (*t_method5) (t_int, t_int, t_int, t_int, t_int,               METHOD_FLOATS);
typedef void (*t_method6) (t_int, t_int, t_int, t_int, t_int, t_int,        METHOD_FLOATS);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef t_pd *(*t_newmethod0) (                                             METHOD_FLOATS);     // --
typedef t_pd *(*t_newmethod1) (t_int,                                       METHOD_FLOATS);
typedef t_pd *(*t_newmethod2) (t_int, t_int,                                METHOD_FLOATS);
typedef t_pd *(*t_newmethod3) (t_int, t_int, t_int,                         METHOD_FLOATS);
typedef t_pd *(*t_newmethod4) (t_int, t_int, t_int, t_int,                  METHOD_FLOATS);
typedef t_pd *(*t_newmethod5) (t_int, t_int, t_int, t_int, t_int,           METHOD_FLOATS);
typedef t_pd *(*t_newmethod6) (t_int, t_int, t_int, t_int, t_int, t_int,    METHOD_FLOATS);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define METHOD_MAXIMUM_ARGUMENTS    10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd pd_objectMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void method_executeObject (t_pd *x, t_method f, int n, t_int *ai, t_float *af)
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

static void method_executeMaker (t_pd *x, t_method f, int n, t_int *ai, t_float *af)
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

static void method_execute (t_pd *x, t_method f, int n, t_int *ai, t_float *af)
{
    if (x == &pd_objectMaker) { method_executeMaker (x, f, n, ai, af); }
    else {
        method_executeObject (x, f, n, ai, af);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error method_typed (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    t_class *c = pd_class (x);
        
    if (s == &s_float) {
        if (argc && IS_FLOAT (argv)) { (*c->c_methodFloat) (x, GET_FLOAT (argv)); }
        else {
            if (!argc) { (*c->c_methodFloat) (x, (t_float)0.0); }
            else {
                err = PD_ERROR;
            }
        }
        
    } else if (s == &s_bang)   {
        (*c->c_methodBang) (x);
        
    } else if (s == &s_list)   {
        (*c->c_methodList) (x, s, argc, argv);
        
    } else if (s == &s_symbol) {
        if (argc && IS_SYMBOL (argv)) { (*c->c_methodSymbol) (x, GET_SYMBOL (argv)); }
        else {
            if (!argc) { (*c->c_methodSymbol) (x, &s_); }
            else {
                err = PD_ERROR;
            }
        }
    }
    
    if (!err && x == &pd_objectMaker) {
    //
    if (argc > 0 && s == &s_bang)       { warning_unusedArguments (s, argc, argv); }
    else if (argc > 1 && s != &s_list)  { warning_unusedArguments (s, argc - 1, argv + 1); }
    //
    }
    
    return err;
}

/* Note that A_FLOAT arguments are always passed at last. */
/* The A_GIMME signature should always be prefered now. */

static t_error method_untyped (t_entry *m, t_pd *x, t_symbol *s, int argc, t_atom *argv)
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
        
    while ((t = *p++)) {
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
                        
    case A_FLOAT     :  if (!argc) { return PD_ERROR; }         /* Break is missing deliberately. */
    case A_DEFFLOAT  :  if (!argc) { *fp = (t_float)0.0; }
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

    method_execute (x, f, n, ai, af);
    
    if (x == &pd_objectMaker) { 
    if (argc) { 
        warning_unusedArguments (s, argc, argv); 
    }
    }
    
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
    /* It aims to let the pointer object be A_GIMME initialized. */

    PD_ASSERT (s != &s_pointer || x == &pd_objectMaker);
    
    if (s == &s_bang || s == &s_float || s == &s_symbol || s == &s_list) {
        err = method_typed (x, s, argc, argv);
        if (!err) { 
            return; 
        }  
    }
        
    if (!err) {
    
        t_entry *m = NULL;
        int i;
            
        for (i = c->c_methodsSize, m = c->c_methods; i--; m++) {
            if (m->me_name == s) {
                err = method_untyped (m, x, s, argc, argv); 
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

    error_invalidArgumentsForMethod (class_getName (c), s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://stackoverflow.com/a/11270603 > */

void pd_vMessage (t_pd *x, t_symbol *s, char *fmt, ...)
{
    va_list ap;
    t_atom args[METHOD_MAXIMUM_ARGUMENTS];
    t_atom *a = args;
    int n = 0;
    char *p = fmt;
    int k = 1; 
    
    va_start (ap, fmt);
    
    while (k) {
    
        if (n >= METHOD_MAXIMUM_ARGUMENTS) { PD_BUG; break; }
        
        switch (*p++) {
            case 'f' : SET_FLOAT   (a, (t_float)va_arg (ap, double));   break;
            case 's' : SET_SYMBOL  (a, va_arg (ap, t_symbol *));        break;
            case 'i' : SET_FLOAT   (a, (t_float)va_arg (ap, t_int));    break;       
            case 'p' : SET_POINTER (a, va_arg (ap, t_gpointer *));      break;
            default  : k = 0;
        }
        
        if (k) { a++; n++; }
    }
    
    va_end (ap);
    
    pd_message (x, s, n, args);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
