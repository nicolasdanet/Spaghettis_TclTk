
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

#if PD_WINDOWS

static LARGE_INTEGER    interface_NTTime;
static double           interface_NTFrequency;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

static void interface_initializeClock (void)
{
    LARGE_INTEGER f1;
    LARGE_INTEGER now;
    
    QueryPerformanceCounter (&now);
    
    if (!QueryPerformanceFrequency (&f1)) { PD_BUG; f1.QuadPart = 1; }
    
    interface_NTTime = now;
    interface_NTFrequency = f1.QuadPart;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

double sys_getRealTime (void)    
{
    LARGE_INTEGER now;
    
    QueryPerformanceCounter (&now);
    
    if (interface_NTFrequency == 0) { interface_initializeClock(); }
    
    return (((double)(now.QuadPart - interface_NTTime.QuadPart)) / interface_NTFrequency);
}

#else 

double sys_getRealTime (void)    
{
    static struct timeval start;
    struct timeval now;
    
    gettimeofday (&now, NULL);
    if (start.tv_sec == 0 && start.tv_usec == 0) { start = now; }
    
    return ((now.tv_sec - start.tv_sec) + (1.0 / 1000000.0) * (now.tv_usec - start.tv_usec));
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
