
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

/* ------------- gstubs and gpointers - safe pointing --------------- */

/* create a gstub which is "owned" by a glist (gl) or an array ("a"). */

t_gstub *gstub_new(t_glist *gl, t_array *a)
{
    t_gstub *gs = PD_MEMORY_GET(sizeof(*gs));
    if (gl)
    {
        gs->gs_type = POINTER_GLIST;
        gs->gs_un.gs_glist = gl;
    }
    else
    {
        gs->gs_type = POINTER_ARRAY;
        gs->gs_un.gs_array = a;
    }
    gs->gs_count = 0;
    return (gs);
}

/* when a "gpointer" is set to point to this stub (so we can later chase
down the owner) we increase a reference count.  The following routine is called
whenever a gpointer is unset from pointing here.  If the owner is
gone and the refcount goes to zero, we can free the gstub safely. */

void gstub_dis(t_gstub *gs)
{
    int refcount = --gs->gs_count;
    if ((!refcount) && gs->gs_type == POINTER_NONE)
        PD_MEMORY_FREE(gs);
    else if (refcount < 0) { PD_BUG; }
}

/* this routing is called by the owner to inform the gstub that it is
being deleted.  If no gpointers are pointing here, we can free the gstub;
otherwise we wait for the last gstub_dis() to free it. */

void gstub_cutoff(t_gstub *gs)
{
    gs->gs_type = POINTER_NONE;
    if (gs->gs_count < 0) { PD_BUG; }
    if (!gs->gs_count) PD_MEMORY_FREE(gs);
}


