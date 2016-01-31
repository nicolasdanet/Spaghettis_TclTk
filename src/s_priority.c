
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__)

#if defined(_POSIX_PRIORITY_SCHEDULING) || defined(_POSIX_MEMLOCK)
#include <sched.h>
#endif

void sys_set_priority(int higher) 
{
#ifdef _POSIX_PRIORITY_SCHEDULING
    struct sched_param par;
    int p1 ,p2, p3;
    p1 = sched_get_priority_min(SCHED_FIFO);
    p2 = sched_get_priority_max(SCHED_FIFO);
#ifdef USEAPI_JACK    
    p3 = (higher ? p1 + 7 : p1 + 5);
#else
    p3 = (higher ? p2 - 5 : p2 - 7);
#endif
    par.sched_priority = p3;
    if (sched_setscheduler(0,SCHED_FIFO,&par) < 0)
    {
        if (!higher)
            post("priority %d scheduling failed; running at normal priority",
                p3);
        else fprintf(stderr, "priority %d scheduling failed.\n", p3);
    }
    else if (!higher && 0)
        post("priority %d scheduling enabled.\n", p3);
#endif

#ifdef REALLY_POSIX_MEMLOCK /* this doesn't work on Fedora 4, for example. */
#ifdef _POSIX_MEMLOCK
    /* tb: force memlock to physical memory { */
    {
        struct rlimit mlock_limit;
        mlock_limit.rlim_cur=0;
        mlock_limit.rlim_max=0;
        setrlimit(RLIMIT_MEMLOCK,&mlock_limit);
    }
    /* } tb */
    if (mlockall(MCL_FUTURE) != -1) 
        fprintf(stderr, "memory locking enabled.\n");
#endif
#endif
}

#endif /* __linux__ */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
