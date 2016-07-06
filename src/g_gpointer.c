
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include <stdlib.h>
#include <string.h>
#include <stdio.h>      /* for read/write to files */
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"


/* call this to verify that a pointer is fresh, i.e., that it either
points to real data or to the head of a list, and that in either case
the object hasn't disappeared since this pointer was generated. 
Unless "headok" is set,  the routine also fails for the head of a list. */

int gpointer_check(const t_gpointer *gp, int headok)
{
    t_gstub *gs = gp->gp_stub;
    if (!gs) return (0);
    if (gs->gs_type == POINTER_ARRAY)
    {
        if (gs->gs_un.gs_array->a_valid != gp->gp_magic) return (0);
        else return (1);
    }
    else if (gs->gs_type == POINTER_GLIST)
    {
        if (!headok && !gp->gp_un.gp_scalar) return (0);
        else if (gs->gs_un.gs_glist->gl_magic != gp->gp_magic) return (0);
        else return (1);
    }
    else return (0);
}

/* get the template for the object pointer to.  Assumes we've already checked
freshness. */

t_symbol *gpointer_gettemplatesym (const t_gpointer *gp)
{
    t_gstub *gs = gp->gp_stub;
    if (gs->gs_type == POINTER_GLIST)
    {
        t_scalar *sc = gp->gp_un.gp_scalar;
        if (sc)
            return (sc->sc_template);
        else return (0);
    }
    else
    {
        t_array *a = gs->gs_un.gs_array;
        return (a->a_template);
    }
}

    /* copy a pointer to another, assuming the second one hasn't yet been
    initialized.  New gpointers should be initialized either by this
    routine or by gpointer_init below. */
void gpointer_copy(const t_gpointer *gpfrom, t_gpointer *gpto)
{
    *gpto = *gpfrom;
    if (gpto->gp_stub)
        gpto->gp_stub->gs_count++;
    else { PD_BUG; }
}

    /* clear a gpointer that was previously set, releasing the associted
    gstub if this was the last reference to it. */
void gpointer_unset(t_gpointer *gp)
{
    t_gstub *gs;
    if (gs = gp->gp_stub)
    {
        gstub_dis(gs);
        gp->gp_stub = 0;
    }
}

void gpointer_setglist(t_gpointer *gp, t_glist *glist, t_scalar *x)
{
    t_gstub *gs;
    if (gs = gp->gp_stub) gstub_dis(gs);
    gp->gp_stub = gs = glist->gl_stub;
    gp->gp_magic = glist->gl_magic;
    gp->gp_un.gp_scalar = x;
    gs->gs_count++;
}

void gpointer_setarray(t_gpointer *gp, t_array *array, t_word *w)
{
    t_gstub *gs;
    if (gs = gp->gp_stub) gstub_dis(gs);
    gp->gp_stub = gs = array->a_stub;
    gp->gp_magic = array->a_valid;
    gp->gp_un.gp_w = w;
    gs->gs_count++;
}

void gpointer_init(t_gpointer *gp)
{
    gp->gp_stub = 0;
    gp->gp_magic = 0;
    gp->gp_un.gp_scalar = 0;
}
