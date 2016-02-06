
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

static t_error priority_realTimeScheduling (int isWatchdog) 
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

    return (sched_setscheduler (0, SCHED_FIFO, &policy) == -1);
}

static t_error priority_memoryLocking (void) 
{       
    return (mlockall (MCL_CURRENT | MCL_FUTURE) == -1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error sys_setRealTimePolicy (int isWatchdog) 
{
    t_error err = PD_ERROR_NONE;
    
    #ifdef PRIORITY_SCHEDULING
        err |= priority_realTimeScheduling (isWatchdog);
    #endif

    #ifdef PRIORITY_MEMLOCK
        err |= priority_memoryLocking();
    #endif
    
    return err;
}

/*
#if defined(__linux__) || defined(__FreeBSD_kernel__)
        if (PD_WITH_REALTIME)
            sys_gui("::watchdog\n");
#endif
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
