
/* 
    Copyright (c) 2010 Peter Brinkmann.
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

char *audio_nameNative (void)
{
    static char *name = "Dummy";
    return name;
}

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

t_error audio_openNative() 
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
    
    err |= string_copy (devicesIn,  MAXIMUM_DESCRIPTION, "Dummy");
    err |= string_copy (devicesOut, MAXIMUM_DESCRIPTION, "Dummy");
    
    *numberOfDevicesIn  = 1;
    *numberOfDevicesOut = 1;
    *canMultiple        = 0;
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

