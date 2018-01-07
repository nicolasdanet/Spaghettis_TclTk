
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static uid_t priority_euid;         /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_LINUX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if defined ( _POSIX_MEMLOCK ) && ( _POSIX_MEMLOCK > 0 )
    #define PRIORITY_MEMLOCK        1
#else
    #define PRIORITY_MEMLOCK        0
#endif

#if defined ( _POSIX_PRIORITY_SCHEDULING ) && ( _POSIX_PRIORITY_SCHEDULING > 0 )
    #define PRIORITY_SCHEDULING     1
#else 
    #define PRIORITY_SCHEDULING     0
#endif 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIORITY_SCHEDULING

static t_error priority_setRTLinuxScheduling (void)
{
    struct sched_param param;
    int min = sched_get_priority_min (SCHED_FIFO);
    int max = sched_get_priority_max (SCHED_FIFO);
    int required = min + 5;                             /* Arbitrary. Should be set according to Jack. */
    
    param.sched_priority = PD_CLAMP (required, min, max);

    return (sched_setscheduler (0, SCHED_FIFO, &param) == -1);
}

#endif // PRIORITY_SCHEDULING

#if PRIORITY_MEMLOCK

static t_error priority_setRTLinuxMemoryLocking (void)
{       
    return (mlockall (MCL_CURRENT | MCL_FUTURE) == -1);
}

#endif // PRIORITY_MEMLOCK

static t_error priority_setRTLinux (void)
{
    t_error err = PD_ERROR;
    
    #if PRIORITY_SCHEDULING
        err = PD_ERROR_NONE; err |= priority_setRTLinuxScheduling();
    #endif

    #if PRIORITY_MEMLOCK
        priority_setRTLinuxMemoryLocking();
    #endif
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_LINUX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_APPLE

static t_error priority_setRTNative (void)
{
    t_error err = PD_ERROR_NONE;
    
    struct sched_param param;
    
    memset (&param, 0, sizeof (struct sched_param));
    
    /* The POSIX thread API is able to set thread priority only within the lowest priority band (0–63). */
    /* Use thread_policy_set instead? */
    /* < https://developer.apple.com/library/ios/technotes/tn2169/_index.html > */
    
    param.sched_priority = 63; 

    err = (pthread_setschedparam (pthread_self(), SCHED_RR, &param) != 0);
    
    PD_ASSERT (!err);
    
    return err;
}

#elif PD_WINDOWS

static t_error priority_setRTNative (void)
{
    if (!SetPriorityClass (GetCurrentProcess(), HIGH_PRIORITY_CLASS)) { PD_BUG; }
    
    return PD_ERROR_NONE;
}

#elif PD_LINUX

static t_error priority_setRTNative (void)
{
    if (!priority_setRTLinux()) { fprintf (stdout, "RT Enabled\n"); }
    else {
        fprintf (stdout, "RT Disabled\n");
    }

    return PD_ERROR_NONE;
}

#else

static t_error priority_setRTNative (void)
{
    PD_BUG; return PD_ERROR_NONE;
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://www.securecoding.cert.org/confluence/x/WIAAAQ > */

t_error priority_privilegeStart (void)
{
    priority_euid = geteuid();
    
    PD_ASSERT (priority_euid != 0); PD_ABORT (priority_euid == 0);      /* Need a security audit first. */
        
    return (getuid() == 0);
}

t_error priority_privilegeDrop (void)
{
    return (seteuid (getuid()) != 0);
}

t_error priority_privilegeRestore (void)
{
    t_error err = PD_ERROR_NONE;
    
    if (geteuid() != priority_euid) { err = (seteuid (priority_euid) != 0); }
    
    return err;
}

t_error priority_privilegeRelinquish (void)
{
    t_error err = priority_privilegeRestore();
    
    if (!err) {
        err = (setuid (getuid()) != 0);
        if (!err) { 
            err = (setuid (0) != -1); 
        }
    }
    
    PD_ASSERT (!err);
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WITH_REALTIME

t_error priority_setPolicy (void)
{
    return priority_setRTNative();
}

#else

t_error priority_setPolicy (void)
{
    return PD_ERROR_NONE;
}

#endif // PD_WITH_REALTIME
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
