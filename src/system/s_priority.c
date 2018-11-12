
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static uid_t priority_euid;     /* Static. */

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
    /* < https://developer.apple.com/library/ios/technotes/tn2169/_index.html > */

    // -- TODO: Use thread_policy_set instead?
    
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
    t_error err = PD_ERROR;
    
    #if defined ( _POSIX_PRIORITY_SCHEDULING ) && ( _POSIX_PRIORITY_SCHEDULING > 0 )

    struct sched_param param;
    int min = sched_get_priority_min (SCHED_FIFO);
    int max = sched_get_priority_max (SCHED_FIFO);
    
    param.sched_priority = PD_MIN (min + 4, max);       /* Arbitrary. */

    err = (sched_setscheduler (0, SCHED_FIFO, &param) != 0);
    
    #endif
    
    fprintf (stdout, err ? "RT Disabled\n" : "RT Enabled\n");

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
    
    PD_ASSERT (priority_euid != 0); PD_ABORT (priority_euid == 0);
        
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

t_error priority_setPolicy (void)
{
    return priority_setRTNative();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
