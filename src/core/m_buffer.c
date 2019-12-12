
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define BUFFER_GROWTH           8
#define BUFFER_PREALLOCATED     8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *buffer_new (void)
{
    t_buffer *x = (t_buffer *)PD_MEMORY_GET (sizeof (t_buffer));
    
    x->b_allocated = 0;
    x->b_size      = 0;
    x->b_vector    = (t_atom *)PD_MEMORY_GET (0);
    
    return x;
}

t_buffer *buffer_newCopy (t_buffer *toCopy)
{
    t_buffer *x = buffer_new(); if (toCopy) { buffer_appendBuffer (x, toCopy); } return x;
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
    if (n >= 0 && n < x->b_size) { return x->b_vector + n; }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *buffer_getSymbolAt (t_buffer *x, int n)
{
    t_atom *a = buffer_getAtomAtIndex (x, n); PD_ASSERT (IS_SYMBOL (a)); return GET_SYMBOL (a);
}

t_gobj *buffer_getObjectAt (t_buffer *x, int n)
{
    t_atom *a = buffer_getAtomAtIndex (x, n); PD_ASSERT (IS_VOID (a));   return GET_OBJECT (a);
}

t_buffer *buffer_getBufferAt (t_buffer *x, int n)
{
    t_atom *a = buffer_getAtomAtIndex (x, n); PD_ASSERT (IS_VOID (a));   return GET_BUFFER (a);
}

t_clock *buffer_getClockAt (t_buffer *x, int n)
{
    t_atom *a = buffer_getAtomAtIndex (x, n); PD_ASSERT (IS_VOID (a));   return GET_CLOCK (a);
}

t_float buffer_getFloatAt (t_buffer *x, int n)
{
    t_atom *a = buffer_getAtomAtIndex (x, n); PD_ASSERT (IS_FLOAT (a));  return GET_FLOAT (a);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_getAtIndex (t_buffer *x, int n, t_atom *a)
{
    t_atom *t = buffer_getAtomAtIndexChecked (x, n); if (t && a) { *a = *t; return PD_ERROR_NONE; }
    
    return PD_ERROR;
}

t_error buffer_setAtIndex (t_buffer *x, int n, t_atom *a)
{
    t_atom *t = buffer_getAtomAtIndexChecked (x, n); if (t) { *t = *a; return PD_ERROR_NONE; }
    
    return PD_ERROR;
}

t_error buffer_insertAtIndex (t_buffer *x, int n, t_atom *a)
{
    if (n >= 0 && n <= x->b_size) {
    //
    if (n == 0)         { buffer_prepend (x, 1, a); return PD_ERROR_NONE; }
    if (n == x->b_size) { buffer_append (x, 1, a);  return PD_ERROR_NONE; }
    
    buffer_extend (x, n, n, 1);
    
    return buffer_setAtIndex (x, n, a);
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_setFloatAtIndex (t_buffer *x, int n, t_float f)
{
    t_atom a; SET_FLOAT (&a, f); return buffer_setAtIndex (x, n, &a);
}

t_error buffer_setSymbolAtIndex (t_buffer *x, int n, t_symbol *s)
{
    t_atom a; SET_SYMBOL (&a, s); return buffer_setAtIndex (x, n, &a);
}

t_float buffer_getFloatAtIndex (t_buffer *x, int n)
{
    return atom_getFloatAtIndex (n, buffer_getSize (x), buffer_getAtoms (x));
}

t_symbol *buffer_getSymbolAtIndex (t_buffer *x, int n)
{
    return atom_getSymbolAtIndex (n, buffer_getSize (x), buffer_getAtoms (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void buffer_allocate (t_buffer *x, int n)
{
    size_t oldSize = sizeof (t_atom) * x->b_allocated;
    size_t newSize = sizeof (t_atom) * n;
    
    PD_ASSERT (n >= 0);
    
    if (oldSize != newSize) {
    //
    x->b_allocated = n;
    x->b_vector    = (t_atom *)PD_MEMORY_RESIZE (x->b_vector, oldSize, newSize);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_clear (t_buffer *x)
{
    x->b_size = 0;
}

void buffer_reserve (t_buffer *x, int n)
{
    if (n > x->b_allocated) { buffer_allocate (x, n); }
}

void buffer_resize (t_buffer *x, int n)
{
    PD_ASSERT (n >= 0); n = PD_MAX (n, 0); buffer_allocate (x, n); x->b_size = n;
}

t_error buffer_extend (t_buffer *x, int start, int end, int n)
{
    PD_ASSERT (n >= 0); n = PD_MAX (n, 0);
    
    if ((start < 0) || (end > x->b_size) || (start > end)) { return PD_ERROR; }
    else {
    //
    int count    = n - (end - start);
    int oldSize  = x->b_size;
    int newSize  = oldSize + count;
    int tailSize = oldSize - end;
    
    if (newSize > oldSize) { buffer_resize (x, newSize); }
    memmove ((void *)(x->b_vector + start + n), (void *)(x->b_vector + end), sizeof (t_atom) * tailSize);
    if (count > 0) { memset ((void *)(x->b_vector + end), 0, sizeof (t_atom) * count); }
    if (newSize < oldSize) { buffer_resize (x, newSize); }
    //
    }
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_prepend (t_buffer *x, int argc, t_atom *argv)
{
    if (argc && x->b_size) { buffer_extend (x, 0, 0, argc); atom_copyAtoms (argv, argc, x->b_vector, argc); }
    else {
        buffer_append (x, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_append (t_buffer *x, int argc, t_atom *argv)
{
    if (argc > 0) {
    //
    int required = x->b_size + argc;

    while (required > x->b_allocated) {
    //
    int n = x->b_allocated * BUFFER_GROWTH; buffer_reserve (x, PD_MAX (BUFFER_PREALLOCATED, n));
    //
    }
    
    {
    //
    t_atom *a = NULL;

    for (a = x->b_vector + x->b_size; argc--; a++) { *a = *(argv++); }
    
    x->b_size = required;
    //
    }
    //
    }
}

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

void buffer_appendPointer (t_buffer *x, t_gpointer *gp)
{
    t_atom a;
    SET_POINTER (&a, gp);
    buffer_append (x, 1, &a);
}

void buffer_appendDollar (t_buffer *x, int n)
{
    t_atom a;
    SET_DOLLAR (&a, n);
    buffer_append (x, 1, &a);
}

void buffer_appendDollarSymbol (t_buffer *x, t_symbol *s)
{
    t_atom a;
    SET_DOLLARSYMBOL (&a, s);
    buffer_append (x, 1, &a);
}

void buffer_appendSemicolon (t_buffer *x)
{
    t_atom a;
    SET_SEMICOLON (&a);
    buffer_append (x, 1, &a);
}

void buffer_appendComma (t_buffer *x)
{
    t_atom a;
    SET_COMMA (&a);
    buffer_append (x, 1, &a);
}

void buffer_appendDescriptor (t_buffer *x, t_fielddescriptor *fd)
{
    if (field_isVariable (fd)) {
        buffer_appendSymbol (x, field_getVariableName (fd));
    } else {
        buffer_appendFloat (x, field_getFloatConstant (fd));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
