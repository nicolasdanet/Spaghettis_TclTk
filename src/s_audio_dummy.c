
/* 
    Copyright (c) 2010-2016 Peter Brinkmann.
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

char *audio_nameNative (void)
{
    static char *name = "Dummy"; return name;
}

int audio_getPriorityNative (int min, int max, int isWatchdog)
{
    return (isWatchdog ? max - 5 : max - 7);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_initializeNative (void)
{
    return PD_ERROR_NONE;
}

void audio_releaseNative (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_openNative (int sampleRate,
    int numberOfChannelsIn,
    int numberOfChannelsOut,
    int blockSize,
    int deviceIn,
    int deviceOut) 
{
    return PD_ERROR_NONE;
}

void audio_closeNative() 
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_pollDSPNative() 
{
    return DACS_NO;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_getListsNative (char *devicesIn, 
    int *numberOfDevicesIn,
    char *devicesOut,
    int *numberOfDevicesOut,
    int *canMultiple) 
{
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (devicesIn,  DEVICES_DESCRIPTION, "Dummy");
    err |= string_copy (devicesOut, DEVICES_DESCRIPTION, "Dummy");
    
    *numberOfDevicesIn  = 1;
    *numberOfDevicesOut = 1;
    *canMultiple        = 0;
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

