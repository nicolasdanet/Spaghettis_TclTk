
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void snippet_renameArrays (t_buffer *x, t_glist *glist)
{
    t_iterator *iter = iterator_new (buffer_getSize (x), buffer_getAtoms (x), 0);
    t_atom *atoms = NULL;
    int count;
    
    t_atom *t1 = NULL;
    t_atom *t2 = NULL;
    t_atom *t3 = NULL;
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    t_symbol *k1 = atom_getSymbolAtIndex (0, count, atoms);
    t_symbol *k2 = atom_getSymbolAtIndex (1, count, atoms);
    
    if (k1 == sym___hash__N) { t1 = t2 = t3 = NULL; }
    
    if (count > 6 && k1 == sym___hash__N && k2 == sym_canvas)       { t1 = atoms + 6; }
    else if (count > 2 && k1 == sym___hash__X && k2 == sym_array)   { t2 = atoms + 2; }
    else if (count > 5 && k1 == sym___hash__X && k2 == sym_restore) { t3 = atoms + 5; }
    
    if (t1 && t2 && t3) {
        t_symbol *s = atom_getSymbolOrDollarSymbol (t2);
        PD_ASSERT (atom_areEquals (t1, t2));
        PD_ASSERT (atom_areEquals (t1, t3));
        while (symbol_getThingByClass (dollar_expandSymbol (s, glist), garray_class)) {
            s = symbol_withCopySuffix (s);
            SET_SYMBOL (t1, s);
            SET_SYMBOL (t2, s);
            SET_SYMBOL (t3, s);
        }
        t1 = t2 = t3 = NULL;
    }
    //
    }
    
    iterator_free (iter);
}

void snippet_addOffsetToLines (t_buffer *x, int i)
{
    t_iterator *iter = iterator_new (buffer_getSize (x), buffer_getAtoms (x), 0);
    t_atom *atoms = NULL;
    int count;
    
    int k = 0;
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    if (count >= 2) {
    //
    t_symbol *s = atom_getSymbolAtIndex (1, count, atoms);
    
    if (s == sym_canvas)  { k++; }      /* Connections in subpatches must not be changed. */
    if (s == sym_restore) { k--; }
    if (s == sym_connect) {
    //
    if (!k && count == 6) {
    //
    t_float m = atom_getFloat (atoms + 2);
    t_float n = atom_getFloat (atoms + 4);
    SET_FLOAT (atoms + 2, m + i);
    SET_FLOAT (atoms + 4, n + i);
    //
    }
    //
    }
    //
    }
    //
    }
    
    iterator_free (iter);
}

void snippet_substractOffsetToLines (t_buffer *x, int i)
{
    snippet_addOffsetToLines (x, -i);
}

static int snippet_isObject (t_symbol *s)
{
    if (s == sym_obj)               { return 1; }
    else if (s == sym_msg)          { return 1; }
    else if (s == sym_comment)      { return 1; }
    else if (s == sym_text)         { return 1; }
    else if (s == sym_floatatom)    { return 1; }
    else if (s == sym_symbolatom)   { return 1; }
    else if (s == sym_restore)      { return 1; }
    
    return 0;
}

static t_rectangle snippet_getBoundingBoxOfObjects (t_buffer *x)
{
    t_iterator *iter = iterator_new (buffer_getSize (x), buffer_getAtoms (x), 0);
    t_atom *atoms = NULL;
    int count;
    
    int k = 0;
    
    t_rectangle r; rectangle_setNothing (&r);
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    if (count >= 2) {
    //
    t_symbol *s = atom_getSymbolAtIndex (1, count, atoms);
    
    if (s == sym_canvas)  { k++; }      /* Objects in subpatches must not be changed. */
    if (s == sym_restore) { k--; }
    
    if (!k && snippet_isObject (s)) {
        int a = atom_getFloatAtIndex (2, count, atoms);
        int b = atom_getFloatAtIndex (3, count, atoms);
        rectangle_add (&r, a, b);
    }
    //
    }
    //
    }
    
    iterator_free (iter);
    
    return r;
}

static void snippet_displaceObjects (t_buffer *x, int deltaX, int deltaY)
{
    t_iterator *iter = iterator_new (buffer_getSize (x), buffer_getAtoms (x), 0);
    t_atom *atoms = NULL;
    int count;
    
    int k = 0;
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    if (count >= 2) {
    //
    t_symbol *s = atom_getSymbolAtIndex (1, count, atoms);
    
    if (s == sym_canvas)  { k++; }      /* Objects in subpatches must not be changed. */
    if (s == sym_restore) { k--; }
    
    if (!k && snippet_isObject (s)) {
        t_float a = atom_getFloatAtIndex (2, count, atoms);
        t_float b = atom_getFloatAtIndex (3, count, atoms);
        SET_FLOAT (atoms + 2, a + deltaX);
        SET_FLOAT (atoms + 3, b + deltaY);
    }
    //
    }
    //
    }
    
    iterator_free (iter);
}

void snippet_disposeObjects (t_buffer *x, int offset)
{
    t_rectangle r = snippet_getBoundingBoxOfObjects (x);
    
    if (!rectangle_isNothing (&r)) {
    //
    int deltaX = offset - rectangle_getTopLeftX (&r);
    int deltaY = offset - rectangle_getTopLeftY (&r);
    
    snippet_displaceObjects (x, deltaX, deltaY);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
