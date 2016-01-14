
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

#define BUFFER_MAXIMUM_ARGUMENTS    64

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

void buffer_log (t_buffer *x)       /* Handy to debug. */
{
    if (x->b_size) {
    //
    char *s = NULL;
    int size = 0;
    
    buffer_toString (x, &s, &size);
    post_log ("%s", s);
    
    PD_MEMORY_FREE (s, size);
    //
    }
}

void buffer_post (t_buffer *x)
{
    post_atoms (x->b_size, x->b_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
