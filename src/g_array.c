
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_array *array_getTop (t_array *x)
{
    t_array *a = x;
    
    PD_ASSERT (a);
    
    while (gpointer_isWord (&a->a_parent)) { a = gpointer_getParentArray (&a->a_parent); }
    
    return a;
}

static t_gpointer *array_getTopParent (t_array *x)
{
    t_array *a = array_getTop (x);
    
    return &a->a_parent;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_array *array_new (t_symbol *templateIdentifier, t_gpointer *parent)
{
    t_array *x = (t_array *)PD_MEMORY_GET (sizeof (t_array));
    
    t_template *template = template_findByIdentifier (templateIdentifier);

    PD_ASSERT (template);
    
    x->a_size               = 1;
    x->a_stride             = ARRAY_WORD * template->tp_size;
    x->a_vector             = (char *)PD_MEMORY_GET (x->a_stride);
    x->a_templateIdentifier = templateIdentifier;
    x->a_master             = gpointer_masterCreateWithArray (x);
    x->a_uniqueIdentifier   = utils_unique();
    x->a_parent             = *parent;                                  /* ??? */

    word_init ((t_word *)(x->a_vector), template, parent);
    
    return x;
}

void array_free (t_array *x)
{
    t_template *template = template_findByIdentifier (x->a_templateIdentifier);
    int i;
        
    PD_ASSERT (template);
    
    gpointer_masterRelease (x->a_master);
    
    for (i = 0; i < x->a_size; i++) {
        t_word *w = (t_word *)(x->a_vector + (x->a_stride * i));
        word_free (w, template);
    }
    
    PD_MEMORY_FREE (x->a_vector);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_word *array_getData (t_array *x)
{
    return (t_word *)x->a_vector;
}

t_symbol *array_getTemplateIdentifier (t_array *x)
{
    return x->a_templateIdentifier;
}

int array_getSize (t_array *x)
{
    return x->a_size;
}

int array_getElementSize (t_array *x)
{
    return (x->a_stride / ARRAY_WORD); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_gpointer *array_getTopParentArray (t_gpointer *gp)
{
    return (array_getTopParent (gpointer_getParentArray (gp)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void array_resize (t_array *x, int n)
{
    t_template *template = template_findByIdentifier (x->a_templateIdentifier);
    
    PD_ASSERT (template);
    
    int elementSize = ARRAY_WORD * template->tp_size;
    int oldSize = x->a_size;
    int newSize = PD_MAX (1, n);

    x->a_vector = (char *)PD_MEMORY_RESIZE (x->a_vector, oldSize * elementSize, newSize * elementSize);
    x->a_size   = n;
    
    if (newSize > oldSize) {
            
        char *t = x->a_vector + (elementSize * oldSize);
        int i   = newSize - oldSize;
        
        for (; i--; t += elementSize) { word_init ((t_word *)t, template, &x->a_parent); }
    }
    
    x->a_uniqueIdentifier = utils_unique();                 /* Invalidate all current pointers. */
}

void array_redraw (t_array *x, t_glist *glist)
{
    scalar_redraw (gpointer_getScalar (array_getTopParent (x)), glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void array_resizeAndRedraw (t_array *array, t_glist *glist, int n)
{
    if (canvas_isMapped (glist)) {
        t_scalar *scalar = gpointer_getScalar (array_getTopParent (array));
        gobj_visibilityChanged (cast_gobj (scalar), glist, 0);
    }
    
    array_resize (array, n);
    
    if (canvas_isMapped (glist)) {
        t_scalar *scalar = gpointer_getScalar (array_getTopParent (array));
        gobj_visibilityChanged (cast_gobj (scalar), glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
