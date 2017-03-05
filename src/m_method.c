
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_gimme)     (t_pd *, t_symbol *, int, t_atom *);
typedef t_pd *(*t_newgimme) (t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_method10)  (t_int);
typedef void (*t_method11)  (t_int, t_float);
typedef void (*t_method12)  (t_int, t_float, t_float);
typedef void (*t_method20)  (t_int, t_int);
typedef void (*t_method21)  (t_int, t_int, t_float);
typedef void (*t_method30)  (t_int, t_int, t_int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef t_pd *(*t_newmethod00)  (void);
typedef t_pd *(*t_newmethod01)  (t_float);
typedef t_pd *(*t_newmethod02)  (t_float, t_float);
typedef t_pd *(*t_newmethod10)  (t_int);
typedef t_pd *(*t_newmethod11)  (t_int, t_float);
typedef t_pd *(*t_newmethod20)  (t_int, t_int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd pd_objectMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define METHOD_COMBINE(m, n)    (((m) << 2) | (n))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void method_execute (t_pd *x, t_method f, int m, t_int *ai, int n, t_float *af)
{
    if (x == &pd_objectMaker) {
    
        t_pd *o = NULL;

        switch (METHOD_COMBINE (m, n)) {
        //
        case METHOD_COMBINE (0, 0)  : o = (*(t_newmethod00)f) ();               break;
        case METHOD_COMBINE (0, 1)  : o = (*(t_newmethod01)f) (af[0]);          break;
        case METHOD_COMBINE (0, 2)  : o = (*(t_newmethod02)f) (af[0], af[1]);   break;
        case METHOD_COMBINE (1, 0)  : o = (*(t_newmethod10)f) (ai[0]);          break;
        case METHOD_COMBINE (1, 1)  : o = (*(t_newmethod11)f) (ai[0], af[0]);   break;
        case METHOD_COMBINE (2, 0)  : o = (*(t_newmethod20)f) (ai[0], ai[1]);   break;
        default : PD_BUG; 
        //
        }
        
        pd_newest = o;

    } else { 
    
        /* In that case adress of the object is passed at first. */
        
        switch (METHOD_COMBINE (m, n)) {
        //
        case METHOD_COMBINE (1, 0)  : (*(t_method10)f) (ai[0]);                 break;
        case METHOD_COMBINE (1, 1)  : (*(t_method11)f) (ai[0], af[0]);          break;
        case METHOD_COMBINE (1, 2)  : (*(t_method12)f) (ai[0], af[0], af[1]);   break;
        case METHOD_COMBINE (2, 0)  : (*(t_method20)f) (ai[0], ai[1]);          break;
        case METHOD_COMBINE (2, 1)  : (*(t_method21)f) (ai[0], ai[1], af[0]);   break;
        case METHOD_COMBINE (3, 0)  : (*(t_method30)f) (ai[0], ai[1], ai[2]);   break;
        default : PD_BUG; 
        //
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Note that A_FLOAT arguments are always passed at last. */

static t_error method_untypedCheck (t_entry *e, t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atomtype t;
    t_atomtype *p = e->me_arguments;
    t_method f = e->me_method;
    t_int   ai[PD_ARGUMENTS + 1] = { 0 };
    t_float af[PD_ARGUMENTS] = { 0 };
    t_int   *ip = ai;
    t_float *fp = af;
    int m = 0;
    int n = 0;
    
    if (x != &pd_objectMaker) { *ip = (t_int)x; ip++; m++;   }
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
                        m++; ip++; break;
                        
    case A_FLOAT     :  if (!argc) { return PD_ERROR; }         /* Break is missing deliberately. */
    case A_DEFFLOAT  :  if (!argc) { *fp = (t_float)0.0; }
                        else {
                            if (IS_FLOAT (argv)) { *fp = GET_FLOAT (argv); }
                            else { 
                                return PD_ERROR; 
                            }
                            argc--; argv++;
                        }
                        n++; fp++; break;
                        
    case A_SYMBOL    :  if (!argc) { return PD_ERROR;  }        /* Ditto. */
    case A_DEFSYMBOL :  if (!argc) { *ip = (t_int)&s_; }
                        else {
                            if (IS_SYMBOL (argv)) { *ip = (t_int)GET_SYMBOL (argv); }
                            else { 
                                return PD_ERROR;
                            }
                            argc--; argv++;
                        }
                        m++; ip++; break;
                            
    default          :  return PD_ERROR;
    //
    }
    //
    }

    method_execute (x, f, m, ai, n, af);
    
    if (x == &pd_objectMaker) { 
    if (argc) { 
        warning_unusedArguments (s, argc, argv); 
    }
    }
    
    return PD_ERROR_NONE;
}

/* The A_GIMME signature should always be prefered now. */

static t_error method_untyped (t_entry *e, t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atomtype *p = e->me_arguments;
    
    if (*p != A_GIMME) { return method_untypedCheck (e, x, s, argc, argv); }
    else {
        if (x == &pd_objectMaker) { pd_newest = (*((t_newgimme)e->me_method)) (s, argc, argv); }
        else { 
            (*((t_gimme)e->me_method)) (x, s, argc, argv); 
        }
    }
    
    return PD_ERROR_NONE;
}

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
#pragma mark -

#define METHOD_VARIADIC_ARGUMENTS    10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* < http://stackoverflow.com/a/11270603 > */

void pd_vMessage (t_pd *x, t_symbol *s, char *fmt, ...)
{
    va_list ap;
    t_atom args[METHOD_VARIADIC_ARGUMENTS];
    t_atom *a = args;
    int n = 0;
    char *p = fmt;
    int k = 1; 
    
    va_start (ap, fmt);
    
    while (k) {
    
        if (n >= METHOD_VARIADIC_ARGUMENTS) { PD_BUG; break; }
        
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
