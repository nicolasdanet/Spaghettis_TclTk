
/*
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _hello {
    t_object    ob_;
    } t_hello;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *hello_new (void);
void hello_bang (t_hello *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *hello_class;

PD_STUB t_error hello_setup (t_symbol *s)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("hello"), hello_new, NULL, sizeof (t_hello), CLASS_DEFAULT, A_NULL); 
    class_addBang (c, (t_method)hello_bang); 
    
    hello_class = c;
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *hello_new (void)
{
    t_hello *x = (t_hello *)pd_new (hello_class);

    return x;
}

void hello_bang (t_hello *x)
{
    post ("Hello world!");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
