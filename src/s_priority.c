
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

#if ( PD_LINUX || PD_BSD || PD_HURD )

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

#ifdef PRIORITY_SCHEDULING

static t_error priority_setRealTimeScheduling (int isWatchdog) 
{
    struct sched_param param;
    int p1 = sched_get_priority_min (SCHED_FIFO);
    int p2 = sched_get_priority_max (SCHED_FIFO);
    int p3;
    
    #ifdef USEAPI_JACK    
        param.sched_priority = (isWatchdog ? p1 + 7 : p1 + 5);
    #else
        param.sched_priority = (isWatchdog ? p2 - 5 : p2 - 7);
    #endif

    return (sched_setscheduler (0, SCHED_FIFO, &param) == -1);
}

#endif // PRIORITY_SCHEDULING

#ifdef PRIORITY_MEMLOCK

static t_error priority_setRealTimeMemoryLocking (void) 
{       
    return (mlockall (MCL_CURRENT | MCL_FUTURE) == -1);
}

#endif // PRIORITY_MEMLOCK

static t_error priority_setRealTime (int isWatchdog) 
{
    t_error err = PD_ERROR_NONE;
    
    #ifdef PRIORITY_SCHEDULING
        err |= priority_setRealTimeScheduling (isWatchdog);
    #endif

    #ifdef PRIORITY_MEMLOCK
        err |= priority_setRealTimeMemoryLocking();
    #endif
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // ( PD_LINUX || PD_BSD || PD_HURD )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_APPLE

static t_error priority_setRTPlatformSpecific (void)
{
    t_error err = PD_ERROR_NONE;
    
    struct sched_param param;
    
    memset (&param, 0, sizeof (struct sched_param));
    
    /* The POSIX thread API is able to set thread priority only within the lowest priority band (0–63). */
    /* Use thread_policy_set instead? */
    /* < https://developer.apple.com/library/ios/technotes/tn2169/_index.html > */
    
    param.sched_priority = 63; 

    err = (pthread_setschedparam (pthread_self(), SCHED_RR, &param) != 0);
    
    return err;
}

#elif PD_WINDOWS

static t_error priority_setRTPlatformSpecific (void)
{
}

#elif ( PD_LINUX || PD_BSD || PD_HURD )

static t_error priority_setRTPlatformSpecific (void)
{
}

#else

static t_error priority_setRTPlatformSpecific (void)
{
    return PD_ERROR_NONE;
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_REALTIME

t_error priority_setPolicy (void)
{
    t_error err = priority_setRTPlatformSpecific();
    
    #if PD_WITH_WATCHDOG
    #if ! ( PD_WITH_NOGUI )
    
    if (!err) { sys_gui ("::watchdog\n"); }
    
    #endif
    #endif

    return err;
}

#else

t_error priority_setPolicy (void)
{
    return PD_ERROR_NONE;
}

#endif // PD_WITH_REALTIME
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
