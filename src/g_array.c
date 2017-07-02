
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_array *array_getTop (t_array *x)
{
    t_array *a = x;
    
    PD_ASSERT (a);
    
    /* For now, an array is always owned by a scalar. */
    /* Furthermore it is never nested. */
    /* Thus while loop must never execute. */
    
    while (gpointer_isWord (&a->a_parent)) {
        a = gpointer_getParentForWord (&a->a_parent);
        PD_BUG;
    }
    
    return a;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_array *array_new (t_symbol *templateIdentifier, t_gpointer *parent)
{
    t_array *x = (t_array *)PD_MEMORY_GET (sizeof (t_array));
    
    t_template *template = template_findByIdentifier (templateIdentifier);

    PD_ASSERT (template);
    
    x->a_elementSize        = template_getSize (template);
    x->a_size               = 1;
    x->a_elements           = (t_word *)PD_MEMORY_GET (x->a_elementSize * sizeof (t_word));
    x->a_templateIdentifier = templateIdentifier;
    x->a_holder             = gmaster_createWithArray (x);
    x->a_uniqueIdentifier   = utils_unique();
    
    gpointer_setByCopy (&x->a_parent, parent);

    word_init (x->a_elements, template, parent);
    
    return x;
}

void array_free (t_array *x)
{
    t_template *template = template_findByIdentifier (x->a_templateIdentifier);
    int i;
        
    PD_ASSERT (template);
    
    gpointer_unset (&x->a_parent);
    gmaster_reset (x->a_holder);
    
    for (i = 0; i < x->a_size; i++) {
        t_word *w = x->a_elements + (x->a_elementSize * i);
        word_free (w, template);
    }
    
    PD_MEMORY_FREE (x->a_elements);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void array_serialize (t_array *x, t_buffer *b)
{
    t_template *template = array_getTemplate (x);
    int i, j;
            
    for (i = 0; i < array_getSize (x); i++) {
    //
    t_word *w = array_getElementAtIndex (x, i);
    
    for (j = 0; j < x->a_elementSize; j++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (template, j);
    
    if (template_fieldIsFloat (template, fieldName)) {
        t_atom t;
        SET_FLOAT (&t, word_getFloat (w, template, fieldName));
        buffer_appendAtom (b, &t);
        
    } else if (template_fieldIsSymbol (template, fieldName)) {
        t_atom t;
        SET_SYMBOL (&t, word_getSymbol (w, template, fieldName));
        buffer_appendAtom (b, &t);
    
    } else {
        PD_BUG;     /* Not implemented, yet. */
    }
    //
    }
    
    buffer_appendSemicolon (b);
    //
    }
}

void array_deserialize (t_array *x, t_iterator *iter)
{
    t_template *template = array_getTemplate (x);
    t_atom *atoms = NULL;
    int count, n = 0;
    
    while ((count = iterator_next (iter, &atoms))) {
    //
    int j;
    t_word *w = NULL;
    
    array_resize (x, n + 1);
    w = array_getElementAtIndex (x, n);
    n++;
    
    for (j = 0; j < x->a_elementSize; j++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (template, j);
    
    if (template_fieldIsFloat (template, fieldName)) {
        if (count) { word_setFloat (w, template, fieldName, atom_getFloat (atoms));     atoms++; count--; }
        
    } else if (template_fieldIsSymbol (template, fieldName)) {
        if (count) { word_setSymbol (w, template, fieldName, atom_getSymbol (atoms));   atoms++; count--; }
        
    } else {
        PD_BUG;     /* Not implemented, yet. */
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_word *array_getElements (t_array *x)
{
    return x->a_elements;
}

t_word *array_getElementAtIndex (t_array *x, int n)
{
    int offset = x->a_elementSize * PD_CLAMP (n, 0, x->a_size - 1);
        
    return (array_getElements (x) + offset);
}

t_symbol *array_getTemplateIdentifier (t_array *x)
{
    return x->a_templateIdentifier;
}

t_template *array_getTemplate (t_array *x)
{
    t_template *template = template_findByIdentifier (x->a_templateIdentifier);
    
    PD_ASSERT (template);
    
    return template;
}

int array_getSize (t_array *x)
{
    return x->a_size;
}

int array_getElementSize (t_array *x)
{
    return x->a_elementSize; 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float array_getFloatAtIndex (t_array *x, int n, t_symbol *fieldName)
{
    return word_getFloat (array_getElementAtIndex (x, n), array_getTemplate (x), fieldName);
}

void array_setFloatAtIndex (t_array *x, int n, t_symbol *fieldName, t_float f)
{
    word_setFloat (array_getElementAtIndex (x, n), array_getTemplate (x), fieldName, f);
}

t_float array_getFloatAtIndexByDescriptor (t_array *x, int n, t_fielddescriptor *fd)
{
    return word_getFloatByDescriptor (array_getElementAtIndex (x, n), array_getTemplate (x), fd);
}

void array_setFloatAtIndexByDescriptor (t_array *x, int n, t_fielddescriptor *fd, t_float f)
{
    word_setFloatByDescriptor (array_getElementAtIndex (x, n), array_getTemplate (x), fd, f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_gpointer *array_getTopParent (t_array *x)
{
    t_array *a = array_getTop (x);
    
    return &a->a_parent;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void array_resize (t_array *x, int n)
{
    t_template *template = template_findByIdentifier (x->a_templateIdentifier);
    
    PD_ASSERT (template);
    
    int oldSize = x->a_size;
    int newSize = PD_MAX (1, n);

    int m = (int)(x->a_elementSize * sizeof (t_word));
    
    /* Release. */
    
    if (newSize < oldSize) {
        t_word *t = x->a_elements + (x->a_elementSize * newSize);
        int count = oldSize - newSize;
        for (; count--; t += x->a_elementSize) { word_free (t, template); }
    }
    
    /* Reallocate. */
    
    x->a_elements = (t_word *)PD_MEMORY_RESIZE (x->a_elements, oldSize * m, newSize * m);
    x->a_size     = n;
    
    /* Initialize. */
    
    if (newSize > oldSize) {
        t_word *t = x->a_elements + (x->a_elementSize * oldSize);
        int count = newSize - oldSize;
        for (; count--; t += x->a_elementSize) { word_init (t, template, &x->a_parent); }
    }
    
    /* Invalidate all existent pointers. */
    
    x->a_uniqueIdentifier = utils_unique();
}

void array_redraw (t_array *x, t_glist *glist)
{
    scalar_redraw (gpointer_getScalar (array_getTopParent (x)), glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void array_resizeAndRedraw (t_array *array, t_glist *glist, int n)
{
    if (glist_isOnScreen (glist)) {
        t_scalar *scalar = gpointer_getScalar (array_getTopParent (array));
        gobj_visibilityChanged (cast_gobj (scalar), glist, 0);
    }
    
    array_resize (array, n);
    
    if (glist_isOnScreen (glist)) {
        t_scalar *scalar = gpointer_getScalar (array_getTopParent (array));
        gobj_visibilityChanged (cast_gobj (scalar), glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
