
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

struct _gmaster {
    union {
        t_glist     *gm_glist;
        t_array     *gm_array;
    } gm_un;
    int             gm_type;
    int             gm_count;
    };
    
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

static void gpointer_masterIncrement (t_gmaster *master)
{
    master->gm_count++;
}

static void gpointer_masterDecrement (t_gmaster *master)
{
    int count = --master->gm_count;
    
    PD_ASSERT (count >= 0);
    
    if (count == 0 && master->gm_type == POINTER_NONE) { PD_MEMORY_FREE (master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_init (t_gpointer *gp)
{
    gp->gp_un.gp_scalar = NULL;
    gp->gp_master       = NULL;
    gp->gp_identifier   = 0;
}

void gpointer_setScalar (t_gpointer *gp, t_glist *glist, t_scalar *scalar)
{
    if (gp->gp_master) { gpointer_masterDecrement (gp->gp_master); }
    
    gpointer_init (gp);
    
    gp->gp_un.gp_scalar = scalar;
    gp->gp_master       = glist->gl_master;
    gp->gp_identifier   = glist->gl_identifier;

    gpointer_masterIncrement (gp->gp_master);
}

void gpointer_setWord (t_gpointer *gp, t_array *array, t_word *w)
{
    if (gp->gp_master) { gpointer_masterDecrement (gp->gp_master); }
    
    gpointer_init (gp);
    
    gp->gp_un.gp_w      = w;
    gp->gp_master       = array->a_master;
    gp->gp_identifier   = array->a_identifier;

    gpointer_masterIncrement (gp->gp_master);
}

void gpointer_unset (t_gpointer *gp)
{
    if (gp->gp_master) { gpointer_masterDecrement (gp->gp_master); }
    
    gpointer_init (gp);
}

void gpointer_retain (t_gpointer *gp)
{
    if (gp->gp_master) { gpointer_masterIncrement (gp->gp_master); }
    else {
        PD_BUG;
    }
}

int gpointer_isSet (t_gpointer *gp)
{
    return (gp->gp_master != NULL);
}

int gpointer_isValid (t_gpointer *gp, int nullPointerIsValid)
{
    if (gpointer_isSet (gp)) {
    //
    t_gmaster *master = gp->gp_master;
    
    if (master->gm_type == POINTER_ARRAY) {
        if (!nullPointerIsValid && !gp->gp_un.gp_w)                     { PD_BUG; return 0; }
        if (master->gm_un.gm_array->a_identifier == gp->gp_identifier)  { return 1; }
        
    } else if (master->gm_type == POINTER_GLIST) {
        if (!nullPointerIsValid && !gp->gp_un.gp_scalar)                { return 0; }
        if (master->gm_un.gm_glist->gl_identifier == gp->gp_identifier) { return 1; }
        
    } else {
        PD_ASSERT (master->gm_type == POINTER_NONE);
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gpointer_copy (t_gpointer *src, t_gpointer *dest)
{
    gpointer_unset (dest);
    
    *dest = *src;
    
    if (dest->gp_master) { gpointer_masterIncrement (dest->gp_master); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gpointer_isScalar (t_gpointer *gp)
{
    return (gp->gp_master->gm_type == POINTER_GLIST);
}

int gpointer_isWord (t_gpointer *gp)
{
    return (gp->gp_master->gm_type == POINTER_ARRAY);
}

int gpointer_getIdentifier (t_gpointer *gp)
{
    return (gp->gp_identifier);
}

t_array *gpointer_getParentArray (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_master->gm_type == POINTER_ARRAY);
    
    return (gp->gp_master->gm_un.gm_array);
}

t_glist *gpointer_getParentGlist (t_gpointer *gp)
{
    PD_ASSERT (gp->gp_master->gm_type == POINTER_GLIST);
    
    return (gp->gp_master->gm_un.gm_glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *gpointer_getTemplate (t_gpointer *gp)
{
    t_symbol *s = NULL;
    t_gmaster *master = gp->gp_master;
    
    PD_ASSERT (gpointer_isValid (gp, 1));
    
    if (master->gm_type == POINTER_GLIST) {
        if (gp->gp_un.gp_scalar) { s = gp->gp_un.gp_scalar->sc_template; }
        
    } else {
        s = master->gm_un.gm_array->a_template;
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
