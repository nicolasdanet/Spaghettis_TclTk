
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define BUFFER_MAXIMUM_VARIADIC     64      /* Arbitrary value. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *buffer_new (void)
{
    t_buffer *x = (t_buffer *)PD_MEMORY_GET (sizeof (t_buffer));
    x->b_vector = (t_atom *)PD_MEMORY_GET (0);
    return x;
}

void buffer_free (t_buffer *x)
{
    PD_MEMORY_FREE (x->b_vector);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int buffer_getSize (t_buffer *x)
{
    return x->b_size;
}

t_atom *buffer_getAtoms (t_buffer *x)
{
    return x->b_vector;
}

t_atom *buffer_getAtomAtIndex (t_buffer *x, int n)
{
    return x->b_vector + n;
}

t_atom *buffer_getAtomAtIndexChecked (t_buffer *x, int n)
{
    if (n >= 0 && n < buffer_getSize (x)) { return x->b_vector + n; }
    
    return NULL;
}

void buffer_clear (t_buffer *x)
{
    buffer_resize (x, 0);
}

void buffer_resize (t_buffer *x, int n)
{
    PD_ASSERT (n >= 0); n = PD_MAX (n, 0);
    
    x->b_size   = n;
    x->b_vector = (t_atom *)PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_resizeBetween (t_buffer *x, int start, int end, int n)
{
    PD_ASSERT (n >= 0); n = PD_MAX (n, 0);
    
    if ((start < 0) || (end > x->b_size) || (start > end)) { return PD_ERROR; }
    else {
    //
    size_t oldSize  = (size_t)x->b_size;
    size_t newSize  = oldSize + (size_t)(n - (end - start));
    size_t tailSize = oldSize - (size_t)(end);
    
    if (newSize > oldSize) { buffer_resize (x, (int)newSize); }
    memmove ((void *)(x->b_vector + start + n), (void *)(x->b_vector + end), sizeof (t_atom) * tailSize);
    if (newSize < oldSize) { buffer_resize (x, (int)newSize); }
    //
    }
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_append (t_buffer *x, int argc, t_atom *argv)
{
    if (argc > 0) {
    //
    t_atom *a = NULL;
    int n = x->b_size + argc;

    x->b_vector = (t_atom *)PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));

    for (a = x->b_vector + x->b_size; argc--; a++) { *a = *(argv++); } x->b_size = n;
    //
    }
}

/* < http://stackoverflow.com/a/11270603 > */

void buffer_vAppend (t_buffer *x, const char *fmt, ...)
{
    va_list ap;
    t_atom args[BUFFER_MAXIMUM_VARIADIC];
    t_atom *a = args;
    int n = 0;
    const char *p = fmt;
    int k = 1;
    
    va_start (ap, fmt);
    
    while (k) {

        if (n >= BUFFER_MAXIMUM_VARIADIC) { PD_BUG; break; }

        switch (*p++) {
            case 'i'    : SET_FLOAT     (a, (t_float)va_arg (ap, int));     break;
            case 'f'    : SET_FLOAT     (a, (t_float)va_arg (ap, double));  break;
            case 's'    : SET_SYMBOL    (a, va_arg (ap, t_symbol *));       break;
            case ';'    : SET_SEMICOLON (a);                                break;
            case ','    : SET_COMMA     (a);                                break;
            default     : k = 0;
        }
        
        if (k) { a++; n++; }
    }
    
    va_end (ap);
    
    buffer_append (x, n, args);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_appendAtom (t_buffer *x, t_atom *a)
{
    buffer_append (x, 1, a);
}

void buffer_appendBuffer (t_buffer *x, t_buffer *y)
{
    buffer_append (x, buffer_getSize (y), buffer_getAtoms (y));
}

void buffer_appendFloat (t_buffer *x, t_float f)
{
    t_atom a;
    SET_FLOAT (&a, f);
    buffer_append (x, 1, &a);
}

void buffer_appendSymbol (t_buffer *x, t_symbol *s)
{
    t_atom a;
    SET_SYMBOL (&a, s);
    buffer_append (x, 1, &a);
}

void buffer_appendSemicolon (t_buffer *x)
{
    t_atom a;
    SET_SEMICOLON (&a);
    buffer_append (x, 1, &a);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_setAtomAtIndex (t_buffer *x, int n, t_atom *a)
{
    t_atom *t = buffer_getAtomAtIndexChecked (x, n); if (t) { *t = *a; return PD_ERROR_NONE; }
    
    return PD_ERROR;
}

t_error buffer_copyAtomAtIndex (t_buffer *x, int n, t_atom *a)
{
    t_atom *t = buffer_getAtomAtIndexChecked (x, n); if (t) { *a = *t; return PD_ERROR_NONE; }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
