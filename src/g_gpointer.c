
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
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Weak pointer machinery. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_gmaster *gpointer_masterCreateWithGlist (t_glist *glist)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (glist);
    
    master->gm_type         = POINTER_GLIST;
    master->gm_un.gm_glist  = glist;
    master->gm_count        = 0;
    
    return master;
}

t_gmaster *gpointer_masterCreateWithArray (t_array *array)
{
    t_gmaster *master = PD_MEMORY_GET (sizeof (t_gmaster));
    
    PD_ASSERT (array);
    
    master->gm_type         = POINTER_ARRAY;
    master->gm_un.gm_array  = array;
    master->gm_count        = 0;
    
    return master;
}

void gpointer_masterRelease (t_gmaster *master)
{
    PD_ASSERT (master->gm_count >= 0);
    
    master->gm_type = POINTER_NONE; if (master->gm_count == 0) { PD_MEMORY_FREE (master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gpointer_decrementMaster (t_gmaster *master)
{
    int count = --master->gm_count;
    
    PD_ASSERT (count >= 0);
    
    if (count == 0 && master->gm_type == POINTER_NONE) { PD_MEMORY_FREE (master); }
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
        if (gs->gm_un.gm_array->a_magic != gp->gp_magic) return (0);
        else return (1);
    }
    else if (gs->gm_type == POINTER_GLIST)
    {
        if (!headok && !gp->gp_un.gp_scalar) return (0);
        else if (gs->gm_un.gm_glist->gl_magic != gp->gp_magic) return (0);
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
        t_array *a = gs->gm_un.gm_array;
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
        gpointer_decrementMaster(gs);
        gp->gp_master = 0;
    }
}

void gpointer_setglist(t_gpointer *gp, t_glist *glist, t_scalar *x)
{
    t_gmaster *gs;
    if (gs = gp->gp_master) gpointer_decrementMaster(gs);
    gp->gp_master = gs = glist->gl_master;
    gp->gp_magic = glist->gl_magic;
    gp->gp_un.gp_scalar = x;
    gs->gm_count++;
}

void gpointer_setarray(t_gpointer *gp, t_array *array, t_word *w)
{
    t_gmaster *gs;
    if (gs = gp->gp_master) gpointer_decrementMaster(gs);
    gp->gp_master = gs = array->a_master;
    gp->gp_magic = array->a_magic;
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
