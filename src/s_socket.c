
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INTERFACE_GUI_BUFFER_SIZE_START     (1024 * 128)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _fdpoll {
    void        *fdp_p;
    int         fdp_fd;
    t_pollfn    fdp_fn;
    } t_fdpoll;

typedef struct _guiqueue {
    t_drawfn            gq_fn;
    void                *gq_p;
    t_glist             *gq_glist;
    struct _guiqueue    *gq_next;
    } t_guiqueue;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_receiver  *interface_guiReceiver;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_fdpoll             *interface_inPollers;                   /* Shared. */

static int                  interface_inPollersSize;                /* Shared. */
static int                  interface_inMaximumFileDescriptor;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_guiqueue           *interface_outGuiQueue;                 /* Shared. */

char                 *interface_outGuiBuffer;                /* Shared. */

int                  interface_outGuiBufferSize;             /* Shared. */
int                  interface_outGuiBufferHead;             /* Shared. */
int                  interface_outGuiBufferTail;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int interface_guiSocket;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int interface_monitorInOut (int microseconds)
{
    int didSomething = 0;
    struct timeval timeOut;
    t_fdpoll *pollers = NULL;
    int i;
    
    timeOut.tv_sec  = 0;
    timeOut.tv_usec = microseconds;
    
    fd_set rSet;
    fd_set wSet;
    fd_set eSet;
    
    FD_ZERO (&rSet);
    FD_ZERO (&wSet);
    FD_ZERO (&eSet);
    
    for (pollers = interface_inPollers, i = interface_inPollersSize; i--; pollers++) {
        FD_SET (pollers->fdp_fd, &rSet);
    }

    select (interface_inMaximumFileDescriptor + 1, &rSet, &wSet, &eSet, &timeOut);
    
    for (i = 0; i < interface_inPollersSize; i++) {
        if (FD_ISSET (interface_inPollers[i].fdp_fd, &rSet)) {
            (*interface_inPollers[i].fdp_fn) (interface_inPollers[i].fdp_p, interface_inPollers[i].fdp_fd);
            didSomething = 1;
        }
    }
    
    return didSomething;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int interface_monitorBlocking (int microseconds)
{
    return interface_monitorInOut (microseconds);
}

int interface_monitorNonBlocking (void)
{
    return interface_monitorInOut (0);
}

void interface_monitorAddPoller (int fd, t_pollfn fn, void *ptr)
{
    int n = interface_inPollersSize;
    int oldSize = (int)(n * sizeof (t_fdpoll));
    int newSize = (int)(oldSize + sizeof (t_fdpoll));
    int i;
    t_fdpoll *p = NULL;
    
    for (i = n, p = interface_inPollers; i--; p++) { PD_ASSERT (p->fdp_fd != fd); }
    
    interface_inPollers = (t_fdpoll *)PD_MEMORY_RESIZE (interface_inPollers, oldSize, newSize);
        
    p = interface_inPollers + n;
    p->fdp_p = ptr;
    p->fdp_fd = fd;
    p->fdp_fn = fn;
        
    interface_inPollersSize = n + 1;
    if (fd > interface_inMaximumFileDescriptor) { interface_inMaximumFileDescriptor = fd; }
}

void interface_monitorRemovePoller (int fd)
{
    int n = interface_inPollersSize;
    int oldSize = (int)(n * sizeof (t_fdpoll));
    int newSize = (int)(oldSize - sizeof (t_fdpoll));
    int i;
    t_fdpoll *p;
    
    PD_ASSERT (oldSize != 0);
    
    for (i = n, p = interface_inPollers; i--; p++) {
    //
    if (p->fdp_fd == fd) {
    //
    while (i--) { *p = *(p + 1); p++; }
    interface_inPollers = (t_fdpoll *)PD_MEMORY_RESIZE (interface_inPollers, oldSize, newSize);
    interface_inPollersSize = n - 1;
    return;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_guiQueueAddIfNotAlreadyThere (void *owner, t_glist *glist, t_drawfn f)
{
    t_guiqueue **qNext = NULL;
    t_guiqueue *q = NULL;
    
    if (!interface_outGuiQueue) { qNext = &interface_outGuiQueue; }
    else {
        t_guiqueue *t = NULL;
        for (t = interface_outGuiQueue; t->gq_next; t = t->gq_next) { if (t->gq_p == owner) { return; } }
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
}

void interface_guiQueueRemove (void *owner)
{
    while (interface_outGuiQueue && interface_outGuiQueue->gq_p == owner) {
        t_guiqueue *first = interface_outGuiQueue;
        interface_outGuiQueue = interface_outGuiQueue->gq_next;
        PD_MEMORY_FREE (first);
    }
    
    if (interface_outGuiQueue) {
        t_guiqueue *q1 = NULL;
        t_guiqueue *q2 = NULL;
        for ((q1 = interface_outGuiQueue); (q2 = q1->gq_next); (q1 = q2)) {
            if (q2->gq_p == owner) { q1->gq_next = q2->gq_next; PD_MEMORY_FREE (q2); break; }
        }
    }
}

void interface_guiQueueRelease (void)
{
    while (interface_outGuiQueue) {
    //
    t_guiqueue *first = interface_outGuiQueue;
    interface_outGuiQueue = interface_outGuiQueue->gq_next;
    PD_MEMORY_FREE (first);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_closeSocket (int fd)
{
    #if PD_WINDOWS
        closesocket (fd);
    #else
        close (fd);
    #endif
}
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_initialize (void)
{
    #if PD_WINDOWS

    WSADATA d;
    short version = MAKEWORD (2, 0);
    
    if (WSAStartup (version, &d)) { PD_BUG; return PD_ERROR; }
    
    #endif
    
    interface_inPollers = (t_fdpoll *)PD_MEMORY_GET (0);
    
    #if ! ( PD_WITH_NOGUI )
    
    interface_outGuiBuffer     = (char *)PD_MEMORY_GET (INTERFACE_GUI_BUFFER_SIZE_START);
    interface_outGuiBufferSize = INTERFACE_GUI_BUFFER_SIZE_START;
    
    #endif
}

void gui_release (void)
{
    #if ! ( PD_WITH_NOGUI )
    
    PD_MEMORY_FREE (interface_outGuiBuffer);
    interface_guiQueueRelease();
    
    #endif
    
    PD_MEMORY_FREE (interface_inPollers);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ! ( PD_WITH_NOGUI )

int interface_flushBuffer (void)
{
    int need = interface_outGuiBufferHead - interface_outGuiBufferTail;
    
    if (need > 0) {
    //
    char *p = interface_outGuiBuffer + interface_outGuiBufferTail;
    int done = (int)send (interface_guiSocket, (void *)p, need, 0);

    if (done < 0) { PD_BUG; scheduler_needToExitWithError(); }
    else {
        if (done == 0) { return 0; }    
        else if (done == need) { interface_outGuiBufferHead = interface_outGuiBufferTail = 0; }
        else {
            PD_ASSERT (done < need); interface_outGuiBufferTail += done;
        }
        
        return 1;
    }
    //
    }
    
    return 0;
}

int interface_flushQueue (void)
{
    if (interface_outGuiQueue) {
    
        while (interface_outGuiQueue) {
        //
        t_guiqueue *first = interface_outGuiQueue;
        interface_outGuiQueue = interface_outGuiQueue->gq_next;
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
