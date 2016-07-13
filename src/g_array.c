
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
    
    t_template *template = template_findbyname (templateIdentifier);

    PD_ASSERT (template);
    
    x->a_size               = 1;
    x->a_elementSize        = sizeof (t_word) * template->tp_size;
    x->a_vector             = (char *)PD_MEMORY_GET (x->a_elementSize);
    x->a_templateIdentifier = templateIdentifier;
    x->a_master             = gpointer_masterCreateWithArray (x);
    x->a_uniqueIdentifier   = utils_unique();
    x->a_parent             = *parent;                                  /* ??? */

    word_init ((t_word *)(x->a_vector), template, parent);
    
    return x;
}

void array_free (t_array *x)
{
    t_template *template = template_findbyname (x->a_templateIdentifier);
    int i;
        
    PD_ASSERT (template);
    
    gpointer_masterRelease (x->a_master);
    
    for (i = 0; i < x->a_size; i++) {
        t_word *w = (t_word *)(x->a_vector + (x->a_elementSize * i));
        word_free (w, template);
    }
    
    PD_MEMORY_FREE (x->a_vector);
    PD_MEMORY_FREE (x);
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

void array_redraw (t_array *x, t_glist *glist)
{
    scalar_redraw (gpointer_getScalar (array_getTopParent (x)), glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* routine to get screen coordinates of a point in an array */
void array_getcoordinate(t_glist *glist,
    char *elem, int xonset, int yonset, int wonset, int indx,
    t_float basex, t_float basey, t_float xinc,
    t_fielddescriptor *xfielddesc, t_fielddescriptor *yfielddesc, t_fielddescriptor *wfielddesc,
    t_float *xp, t_float *yp, t_float *wp)
{
    t_float xval, yval, ypix, wpix;
    if (xonset >= 0)
        xval = *(t_float *)(elem + xonset);
    else xval = indx * xinc;
    if (yonset >= 0)
        yval = *(t_float *)(elem + yonset);
    else yval = 0;
    ypix = canvas_valueToPositionY(glist, basey +
        fielddesc_cvttocoord(yfielddesc, yval));
    if (wonset >= 0)
    {
            /* found "w" field which controls linewidth. */
        t_float wval = *(t_float *)(elem + wonset);
        wpix = canvas_valueToPositionY(glist, basey + 
            fielddesc_cvttocoord(yfielddesc, yval) +
                fielddesc_cvttocoord(wfielddesc, wval)) - ypix;
        if (wpix < 0)
            wpix = -wpix;
    }
    else wpix = 1;
    *xp = canvas_valueToPositionX(glist, basex +
        fielddesc_cvttocoord(xfielddesc, xval));
    *yp = ypix;
    *wp = wpix;
}

void array_resize(t_array *x, int n)
{
    int elemsize, oldn;
    t_gpointer *gp;
    t_template *template = template_findbyname(x->a_templateIdentifier);
    if (n < 1)
        n = 1;
    oldn = x->a_size;
    elemsize = sizeof(t_word) * template->tp_size;

    x->a_vector = (char *)PD_MEMORY_RESIZE(x->a_vector, oldn * elemsize, n * elemsize);
    x->a_size = n;
    if (n > oldn)
    {
        char *cp = x->a_vector + elemsize * oldn;
        int i = n - oldn;
        for (; i--; cp += elemsize)
        {
            t_word *wp = (t_word *)cp;
            word_init(wp, template, &x->a_parent);
        }
    }
    x->a_uniqueIdentifier = utils_unique();
}

void array_resize_and_redraw(t_array *array, t_glist *glist, int n)
{
    t_array *a2 = array_getTop (array);
    int vis = canvas_isMapped(glist);
    if (vis) {
        t_scalar *scalar = gpointer_getScalar (&a2->a_parent);
        gobj_visibilityChanged (cast_gobj (scalar), glist, 0);
    }
    array_resize(array, n);
    if (vis) {
        t_scalar *scalar = gpointer_getScalar (&a2->a_parent);
        gobj_visibilityChanged (cast_gobj (scalar), glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
