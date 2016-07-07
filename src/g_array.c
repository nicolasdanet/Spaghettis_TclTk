
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
#include "s_system.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int canvas_identifier;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Pure arrays have no a priori graphical capabilities.
They are instantiated by "garrays" below or can be elements of other
scalars (g_scalar.c); their graphical behavior is defined accordingly. */

t_array *array_new (t_symbol *templatesym, t_gpointer *parent)
{
    t_array *x = (t_array *)PD_MEMORY_GET(sizeof (*x));
    t_template *template;
    t_gpointer *gp;
    template = template_findbyname(templatesym);
    x->a_template = templatesym;
    x->a_size = 1;
    x->a_elementSize = sizeof(t_word) * template->tp_size;
    x->a_vector = (char *)PD_MEMORY_GET(x->a_elementSize);
        /* note here we blithely copy a gpointer instead of "setting" a
        new one; this gpointer isn't accounted for and needn't be since
        we'll be deleted before the thing pointed to gets deleted anyway;
        see array_free. */
    x->a_gpointer = *parent;
    x->a_master = gpointer_masterCreateWithArray (x);
    word_init((t_word *)(x->a_vector), template, parent);
    return (x);
}

void array_redraw(t_array *a, t_glist *glist)
{
    while (a->a_gpointer.gp_master->gm_type == POINTER_ARRAY)
        a = a->a_gpointer.gp_master->gm_un.gm_array;
    scalar_redraw(a->a_gpointer.gp_un.gp_scalar, glist);
}

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
    t_template *template = template_findbyname(x->a_template);
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
            word_init(wp, template, &x->a_gpointer);
        }
    }
    x->a_identifier = ++canvas_identifier;
}

void array_resize_and_redraw(t_array *array, t_glist *glist, int n)
{
    t_array *a2 = array;
    int vis = canvas_isMapped(glist);
    while (a2->a_gpointer.gp_master->gm_type == POINTER_ARRAY)
        a2 = a2->a_gpointer.gp_master->gm_un.gm_array;
    if (vis)
        gobj_visibilityChanged(&a2->a_gpointer.gp_un.gp_scalar->sc_g, glist, 0);
    array_resize(array, n);
    if (vis)
        gobj_visibilityChanged(&a2->a_gpointer.gp_un.gp_scalar->sc_g, glist, 1);
}

void array_free(t_array *x)
{
    int i;
    t_template *scalartemplate = template_findbyname(x->a_template);
    gpointer_masterRelease (x->a_master);
    for (i = 0; i < x->a_size; i++)
    {
        t_word *wp = (t_word *)(x->a_vector + x->a_elementSize * i);
        word_free(wp, scalartemplate);
    }
    PD_MEMORY_FREE(x->a_vector);
    PD_MEMORY_FREE(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
