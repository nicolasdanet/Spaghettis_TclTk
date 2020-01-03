
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SLOTS_TAG           0
#define SLOTS_NAME          1
#define SLOTS_COUNT         2
#define SLOTS_SEMICOLON     3
#define SLOTS_SIZE          4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int slots_fetch (t_slots *x, t_symbol *name, int *size, int *index)
{
    t_atom *atoms  = NULL;
    int count, k = 0;
    
    iterator_set (x->sl_iter, buffer_getSize (x->sl_buffer), buffer_getAtoms (x->sl_buffer));
    
    while ((count = iterator_next (x->sl_iter, &atoms))) {
    //
    if (count == SLOTS_SIZE) {
        if (atom_getSymbol (atoms + SLOTS_TAG) == sym__SLOT_) {
        if (IS_FLOAT (atoms + SLOTS_COUNT)) {
            int chunk = (int)GET_FLOAT (atoms + SLOTS_COUNT);
            PD_ASSERT (chunk >= 0);
            k++; if (index) { (*index) = k; }
            if (atom_getSymbol (atoms + SLOTS_NAME) == name) {
                if (size) { (*size) = chunk + SLOTS_SIZE; } return (iterator_get (x->sl_iter) - SLOTS_SIZE);
            } else {
                iterator_skip (x->sl_iter, chunk);
            }
        }
        }
    }
    //
    }
    
    return -1;
}

static void slots_append (t_slots *x, t_symbol *name, t_buffer *data)
{
    buffer_appendSymbol (x->sl_buffer, sym__SLOT_);
    buffer_appendSymbol (x->sl_buffer, name);
    buffer_appendFloat (x->sl_buffer, buffer_getSize (data));
    buffer_appendSemicolon (x->sl_buffer);
    buffer_appendBuffer (x->sl_buffer, data);
}

static t_symbol *slots_name (t_atom *key)
{
    if (key) {
        if (IS_SYMBOL (key)) { return GET_SYMBOL (key); }
        else {
            return symbol_addPrefix (symbol_withAtom (key), sym___arrobe__);
        }
    }
    
    return sym___arrobe__;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void slots_set (t_slots *x, t_atom *key, t_buffer *data)
{
    slots_remove (x, key);
    slots_append (x, slots_name (key), data);
}

t_error slots_get (t_slots *x, t_atom *key, t_buffer *data)
{
    int size; int i = slots_fetch (x, slots_name (key), &size, NULL);
    
    if (i >= 0) {
    //
    if (i + size <= buffer_getSize (x->sl_buffer)) {
    //
    buffer_append (data, size - SLOTS_SIZE, buffer_getAtoms (x->sl_buffer) + i + SLOTS_SIZE);
        
    return PD_ERROR_NONE;
    //
    } else { PD_BUG; }
    //
    }
    
    return PD_ERROR;
}

t_error slots_remove (t_slots *x, t_atom *key)
{
    int size; int i = slots_fetch (x, slots_name (key), &size, NULL);
    
    if (i < 0) { return PD_ERROR; } else { return buffer_extend (x->sl_buffer, i, i + size, 0); }
}

void slots_clear (t_slots *x)
{
    buffer_clear (x->sl_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int slots_isEmpty (t_slots *x)
{
    return (buffer_getSize (x->sl_buffer) == 0);
}

int slots_getSize (t_slots *x)
{
    int k = 0;
    
    slots_fetch (x, NULL, NULL, &k);
    
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *slots_getRaw (t_slots *x)
{
    return x->sl_buffer;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_slots *slots_new (void)
{
    t_slots *x = (t_slots *)PD_MEMORY_GET (sizeof (t_slots));
    
    x->sl_iter   = iterator_new (0, NULL, 1);
    x->sl_buffer = buffer_new();
    
    return x;
}

void slots_free (t_slots *x)
{
    buffer_free (x->sl_buffer);
    iterator_free (x->sl_iter);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
