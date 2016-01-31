
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

#if (PD_LINUX || PD_BSD || PD_HURD )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if defined ( _POSIX_MEMLOCK ) && ( _POSIX_MEMLOCK > 0 )
    #define PRIORITY_MEMLOCK
#endif

#if defined ( _POSIX_PRIORITY_SCHEDULING ) && ( _POSIX_PRIORITY_SCHEDULING > 0 )
    #define PRIORITY_SCHEDULING
#endif 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void priority_realTimeScheduling (int isWatchdog) 
{
    struct sched_param policy;
    int p1 = sched_get_priority_min (SCHED_FIFO);
    int p2 = sched_get_priority_max (SCHED_FIFO);
    int p3;
    
    #ifdef USEAPI_JACK    
        policy.sched_priority = (isWatchdog ? p1 + 7 : p1 + 5);
    #else
        policy.sched_priority = (isWatchdog ? p2 - 5 : p2 - 7);
    #endif

    if (sched_setscheduler (0, SCHED_FIFO, &policy) == -1) { PD_BUG; }
}

static void priority_memoryLocking (void) 
{       
    if (mlockall (MCL_CURRENT | MCL_FUTURE) == -1) { PD_BUG; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_setRealTimePolicy (int isWatchdog) 
{
    #ifdef PRIORITY_SCHEDULING
        priority_realTimeScheduling (isWatchdog);
    #endif

    #ifdef PRIORITY_MEMLOCK
        priority_memoryLocking();
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
