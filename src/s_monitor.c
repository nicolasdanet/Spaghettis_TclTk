
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _fdpoll {
    void        *fdp_p;
    int         fdp_fd;
    t_pollfn    fdp_fn;
    } t_fdpoll;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_fdpoll *monitor_pollers;                   /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      monitor_pollersSize;                /* Static. */
static int      monitor_maximumFileDescriptor;      /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int monitor_proceed (int microseconds)
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
    
    for (pollers = monitor_pollers, i = monitor_pollersSize; i--; pollers++) {
        FD_SET (pollers->fdp_fd, &rSet);
    }

    select (monitor_maximumFileDescriptor + 1, &rSet, &wSet, &eSet, &timeOut);
    
    for (i = 0; i < monitor_pollersSize; i++) {
        if (FD_ISSET (monitor_pollers[i].fdp_fd, &rSet)) {
            (*monitor_pollers[i].fdp_fn) (monitor_pollers[i].fdp_p, monitor_pollers[i].fdp_fd);
            didSomething = 1;
        }
    }
    
    return didSomething;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int monitor_blocking (int microseconds)
{
    return monitor_proceed (microseconds);
}

int monitor_nonBlocking (void)
{
    return monitor_proceed (0);
}

void monitor_addPoller (int fd, t_pollfn fn, void *ptr)
{
    int n = monitor_pollersSize;
    int oldSize = (int)(n * sizeof (t_fdpoll));
    int newSize = (int)(oldSize + sizeof (t_fdpoll));
    int i;
    t_fdpoll *p = NULL;
    
    for (i = n, p = monitor_pollers; i--; p++) { PD_ASSERT (p->fdp_fd != fd); }
    
    monitor_pollers = (t_fdpoll *)PD_MEMORY_RESIZE (monitor_pollers, oldSize, newSize);
        
    p = monitor_pollers + n;
    p->fdp_p = ptr;
    p->fdp_fd = fd;
    p->fdp_fn = fn;
        
    monitor_pollersSize = n + 1;
    if (fd > monitor_maximumFileDescriptor) { monitor_maximumFileDescriptor = fd; }
}

void monitor_removePoller (int fd)
{
    int n = monitor_pollersSize;
    int oldSize = (int)(n * sizeof (t_fdpoll));
    int newSize = (int)(oldSize - sizeof (t_fdpoll));
    int i;
    t_fdpoll *p;
    
    PD_ASSERT (oldSize != 0);
    
    for (i = n, p = monitor_pollers; i--; p++) {
    //
    if (p->fdp_fd == fd) {
    //
    while (i--) { *p = *(p + 1); p++; }
    monitor_pollers = (t_fdpoll *)PD_MEMORY_RESIZE (monitor_pollers, oldSize, newSize);
    monitor_pollersSize = n - 1;
    return;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void monitor_initialize (void)
{
    monitor_pollers = (t_fdpoll *)PD_MEMORY_GET (0);
}

void monitor_release (void)
{
    PD_MEMORY_FREE (monitor_pollers);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
