
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void snippet_renameArrays (t_buffer *x, t_glist *glist)
{
    t_iterator *iter = iterator_new (buffer_getSize (x), buffer_getAtoms (x));
    t_atom *atoms = NULL;
    int count;
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    if (count >= 8) {
    if (atom_getSymbolAtIndex (0, count, atoms) == sym___hash__N) {
    if (atom_getSymbolAtIndex (1, count, atoms) == sym_canvas) {

        t_atom *t1  = atoms + 6;
        t_atom *t2  = NULL;
        t_atom *t3  = NULL;
        t_symbol *s = NULL;
        
        while ((count = iterator_next (iter, &atoms))) {
            if ((count > 2) && atom_getSymbolAtIndex (1, count, atoms) == sym_array) {
                s  = atom_getDollarSymbolAtIndex (2, count, atoms);
                t2 = atoms + 2;
            }
            if ((count > 5) && atom_getSymbolAtIndex (1, count, atoms) == sym_restore) {
                t3 = atoms + 5;
                break;
            }
        }
        
        if (s && t1 && t2 && t3) {
            while (symbol_getThingByClass (dollar_expandSymbol (s, glist), garray_class)) {
                s = symbol_withCopySuffix (s);
                SET_SYMBOL (t1, s);
                SET_SYMBOL (t2, s);
                SET_SYMBOL (t3, s);
            }
        }
    }
    }
    }
    //
    }
    
    iterator_free (iter);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void snippet_addOffsetToLines (t_buffer *x, int i)
{
    t_iterator *iter = iterator_new (buffer_getSize (x), buffer_getAtoms (x));
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
