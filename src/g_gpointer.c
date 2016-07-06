
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

t_gmaster *gstub_new(t_glist *gl, t_array *a)
{
    t_gmaster *gs = PD_MEMORY_GET(sizeof(*gs));
    if (gl)
    {
        gs->gm_type = POINTER_GLIST;
        gs->gs_un.gm_glist = gl;
    }
    else
    {
        gs->gm_type = POINTER_ARRAY;
        gs->gs_un.gm_array = a;
    }
    gs->gm_count = 0;
    return (gs);
}

/* when a "gpointer" is set to point to this stub (so we can later chase
down the owner) we increase a reference count.  The following routine is called
whenever a gpointer is unset from pointing here.  If the owner is
gone and the refcount goes to zero, we can free the gstub safely. */

void gstub_dis(t_gmaster *gs)
{
    int refcount = --gs->gm_count;
    if ((!refcount) && gs->gm_type == POINTER_NONE)
        PD_MEMORY_FREE(gs);
    else if (refcount < 0) { PD_BUG; }
}

/* this routing is called by the owner to inform the gstub that it is
being deleted.  If no gpointers are pointing here, we can free the gstub;
otherwise we wait for the last gstub_dis() to free it. */

void gstub_cutoff(t_gmaster *gs)
{
    gs->gm_type = POINTER_NONE;
    if (gs->gm_count < 0) { PD_BUG; }
    if (!gs->gm_count) PD_MEMORY_FREE(gs);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* call this to verify that a pointer is fresh, i.e., that it either
points to real data or to the head of a list, and that in either case
the object hasn't disappeared since this pointer was generated. 
Unless "headok" is set,  the routine also fails for the head of a list. */

int gpointer_check(const t_gpointer *gp, int headok)
{
    t_gmaster *gs = gp->gp_master;
    if (!gs) return (0);
    if (gs->gm_type == POINTER_ARRAY)
    {
        if (gs->gs_un.gm_array->a_valid != gp->gp_magic) return (0);
        else return (1);
    }
    else if (gs->gm_type == POINTER_GLIST)
    {
        if (!headok && !gp->gp_un.gp_scalar) return (0);
        else if (gs->gs_un.gm_glist->gl_magic != gp->gp_magic) return (0);
        else return (1);
    }
    else return (0);
}

/* get the template for the object pointer to.  Assumes we've already checked
freshness. */

t_symbol *gpointer_gettemplatesym (const t_gpointer *gp)
{
    t_gmaster *gs = gp->gp_master;
    if (gs->gm_type == POINTER_GLIST)
    {
        t_scalar *sc = gp->gp_un.gp_scalar;
        if (sc)
            return (sc->sc_template);
        else return (0);
    }
    else
    {
        t_array *a = gs->gs_un.gm_array;
        return (a->a_template);
    }
}

    /* copy a pointer to another, assuming the second one hasn't yet been
    initialized.  New gpointers should be initialized either by this
    routine or by gpointer_init below. */
void gpointer_copy(const t_gpointer *gpfrom, t_gpointer *gpto)
{
    *gpto = *gpfrom;
    if (gpto->gp_master)
        gpto->gp_master->gm_count++;
    else { PD_BUG; }
}

    /* clear a gpointer that was previously set, releasing the associted
    gstub if this was the last reference to it. */
void gpointer_unset(t_gpointer *gp)
{
    t_gmaster *gs;
    if (gs = gp->gp_master)
    {
        gstub_dis(gs);
        gp->gp_master = 0;
    }
}

void gpointer_setglist(t_gpointer *gp, t_glist *glist, t_scalar *x)
{
    t_gmaster *gs;
    if (gs = gp->gp_master) gstub_dis(gs);
    gp->gp_master = gs = glist->gl_master;
    gp->gp_magic = glist->gl_magic;
    gp->gp_un.gp_scalar = x;
    gs->gm_count++;
}

void gpointer_setarray(t_gpointer *gp, t_array *array, t_word *w)
{
    t_gmaster *gs;
    if (gs = gp->gp_master) gstub_dis(gs);
    gp->gp_master = gs = array->a_master;
    gp->gp_magic = array->a_valid;
    gp->gp_un.gp_w = w;
    gs->gm_count++;
}

void gpointer_init(t_gpointer *gp)
{
    gp->gp_master = 0;
    gp->gp_magic = 0;
    gp->gp_un.gp_scalar = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
