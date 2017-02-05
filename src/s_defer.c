
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _guiqueue {
    t_drawfn            gq_fn;
    void                *gq_p;
    t_glist             *gq_glist;
    struct _guiqueue    *gq_next;
    } t_guiqueue;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ! ( PD_WITH_NOGUI )

static t_guiqueue   *defer_queue;       /* Shared. */

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void defer_addTask (void *owner, t_glist *glist, t_drawfn f)    /* Add draw job if not already there. */
{
    #if ! ( PD_WITH_NOGUI )
    
    t_guiqueue **qNext = NULL;
    t_guiqueue *q = NULL;
    
    if (!defer_queue) { qNext = &defer_queue; }
    else {
        t_guiqueue *t = NULL;
        for (t = defer_queue; t->gq_next; t = t->gq_next) { if (t->gq_p == owner) { return; } }
        if (t->gq_p == owner) { return; }
        else {
            qNext = &t->gq_next;
        }
    }
    
    q = (t_guiqueue *)PD_MEMORY_GET (sizeof (t_guiqueue));
    
    q->gq_fn    = f;
    q->gq_p     = owner;
    q->gq_glist = glist;
    q->gq_next  = NULL;
    
    *qNext = q;
    
    #endif
}

void defer_removeTask (void *owner)
{
    #if ! ( PD_WITH_NOGUI )
    
    while (defer_queue && defer_queue->gq_p == owner) {
        t_guiqueue *first = defer_queue;
        defer_queue = defer_queue->gq_next;
        PD_MEMORY_FREE (first);
    }
    
    if (defer_queue) {
        t_guiqueue *q1 = NULL;
        t_guiqueue *q2 = NULL;
        for ((q1 = defer_queue); (q2 = q1->gq_next); (q1 = q2)) {
            if (q2->gq_p == owner) { q1->gq_next = q2->gq_next; PD_MEMORY_FREE (q2); break; }
        }
    }
    
    #endif
}

#if ! ( PD_WITH_NOGUI )

int defer_flushQueue (void)
{
    if (defer_queue) {
    
        while (defer_queue) {
        //
        t_guiqueue *first = defer_queue;
        defer_queue = defer_queue->gq_next;
        (*first->gq_fn) (first->gq_p, first->gq_glist);
        PD_MEMORY_FREE (first);
        //
        }

        return 1;
    }
    
    return 0;
}

#endif // !PD_WITH_NOGUI

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void defer_release (void)
{
    #if ! ( PD_WITH_NOGUI )
    
    while (defer_queue) {
    //
    t_guiqueue *first = defer_queue;
    defer_queue = defer_queue->gq_next;
    PD_MEMORY_FREE (first);
    //
    }
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
