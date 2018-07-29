
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

static t_symbol *slots_name (t_atom *key)
{
    if (key) {
        if (IS_SYMBOL (key)) { return GET_SYMBOL (key); }
        else {
            return symbol_addPrefix (symbol_withAtoms (1, key), sym___arrobe__);
        }
    }
    
    return sym___arrobe__;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int slots_fetch (t_slots *x, t_atom *key)
{
    t_atom *atoms = NULL;
    int count;
    
    iterator_set (x->sl_iter, buffer_getSize (x->sl_buffer), buffer_getAtoms (x->sl_buffer));
    
    while ((count = iterator_next (x->sl_iter, &atoms))) {
    //
    if (count == SLOTS_SIZE) {
        if (atom_getSymbol (atoms + SLOTS_TAG)  == sym__SLOT_) {
        if (atom_getSymbol (atoms + SLOTS_NAME) == slots_name (key)) {
            if (IS_FLOAT (atoms + SLOTS_COUNT)) {
                PD_ASSERT (IS_SEMICOLON (atoms + SLOTS_SEMICOLON));
                return (iterator_get (x->sl_iter) - SLOTS_SIZE);
            }
        }
        }
    }
    //
    }
    
    return -1;
}

static void slots_append (t_slots *x, t_atom *key, t_buffer *data)
{
    buffer_appendSymbol (x->sl_buffer, sym__SLOT_);
    buffer_appendSymbol (x->sl_buffer, slots_name (key));
    buffer_appendFloat (x->sl_buffer, buffer_getSize (data));
    buffer_appendSemicolon (x->sl_buffer);
    buffer_appendBuffer (x->sl_buffer, data);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void slots_clear (t_slots *x)
{
    buffer_clear (x->sl_buffer);
}

t_error slots_remove (t_slots *x, t_atom *key)
{
    int i = slots_fetch (x, key);
        
    if (i >= 0) {
    //
    int size = atom_getFloat (buffer_getAtoms (x->sl_buffer) + i + SLOTS_COUNT);
    
    PD_ASSERT (size >= 0);
    
    size += SLOTS_SIZE;
    
    return buffer_extend (x->sl_buffer, i, i + size, 0);
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void slots_set (t_slots *x, t_atom *key, t_buffer *data)
{
    slots_remove (x, key);
    slots_append (x, key, data);
}

t_error slots_get (t_slots *x, t_atom *key, t_buffer *data)
{
    int i = slots_fetch (x, key);
    
    if (i >= 0) {
    //
    int size = atom_getFloat (buffer_getAtoms (x->sl_buffer) + i + SLOTS_COUNT);
    
    PD_ASSERT (size >= 0);
    
    i += SLOTS_SIZE;
    
    if (i + size <= buffer_getSize (x->sl_buffer)) {
    //
    buffer_append (data, size, buffer_getAtoms (x->sl_buffer) + i);
        
    return PD_ERROR_NONE;
    //
    } else { PD_BUG; }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int slots_isEmpty (t_slots *x)
{
    return (buffer_getSize (x->sl_buffer) == 0);
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
