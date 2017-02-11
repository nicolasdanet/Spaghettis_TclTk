
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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

void deviceslist_init (t_deviceslist *p)
{
    p->d_inSize  = 0;
    p->d_outSize = 0;

    memset (p->d_inNames,   0, DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION * sizeof (char));
    memset (p->d_outNames,  0, DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION * sizeof (char));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error deviceslist_appendIn (t_deviceslist *p, const char *device)
{
    if (p->d_inSize < DEVICES_MAXIMUM_DEVICES) {
        char *s = p->d_inNames + (p->d_inSize * DEVICES_DESCRIPTION);
        t_error err = string_copy (s, DEVICES_DESCRIPTION, device);
        if (!err) { p->d_inSize++; }
        else {
            string_clear (s, DEVICES_DESCRIPTION);
        }
        return err;
    }
    
    return PD_ERROR;
}

t_error deviceslist_appendOut (t_deviceslist *p, const char *device)
{
    if (p->d_outSize < DEVICES_MAXIMUM_DEVICES) {
        char *s = p->d_outNames + (p->d_outSize * DEVICES_DESCRIPTION);
        t_error err = string_copy (s, DEVICES_DESCRIPTION, device);
        if (!err) { p->d_outSize++; }
        else {
            string_clear (s, DEVICES_DESCRIPTION);
        }
        return err;
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error deviceslist_appendMidiInAsNumber (t_deviceslist *p, int n)
{
    if (p->d_inSize < DEVICES_MAXIMUM_DEVICES) {
    //
    char *s = p->d_inNames + (p->d_inSize * DEVICES_DESCRIPTION);
    
    if (!midi_deviceAsStringWithNumber (0, n, s, DEVICES_DESCRIPTION)) {
        p->d_inSize++;
        return PD_ERROR_NONE;
    }
    //
    }
    
    return PD_ERROR;
}

t_error deviceslist_appendMidiOutAsNumber (t_deviceslist *p, int n)
{
    if (p->d_outSize < DEVICES_MAXIMUM_DEVICES) {
    //
    char *s = p->d_outNames + (p->d_outSize * DEVICES_DESCRIPTION);

    if (!midi_deviceAsStringWithNumber (1, n, s, DEVICES_DESCRIPTION)) {
        p->d_outSize++;
        return PD_ERROR_NONE;
    }
    //
    }
    
    return PD_ERROR;
}

t_error deviceslist_appendAudioInAsNumber (t_deviceslist *p, int n)
{
    if (p->d_inSize < DEVICES_MAXIMUM_DEVICES) {
    //
    char *s = p->d_inNames + (p->d_inSize * DEVICES_DESCRIPTION);
    
    if (!audio_deviceAsStringWithNumber (0, n, s, DEVICES_DESCRIPTION)) {
        p->d_inSize++;
        return PD_ERROR_NONE;
    }
    //
    }
    
    return PD_ERROR;
}

t_error deviceslist_appendAudioOutAsNumber (t_deviceslist *p, int n)
{
    if (p->d_outSize < DEVICES_MAXIMUM_DEVICES) {
    //
    char *s = p->d_outNames + (p->d_outSize * DEVICES_DESCRIPTION);
    
    if (!audio_deviceAsStringWithNumber (1, n, s, DEVICES_DESCRIPTION)) {
        p->d_outSize++;
        return PD_ERROR_NONE;
    }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *deviceslist_getInAtIndex (t_deviceslist *p, int i)
{
    PD_ASSERT (i >= 0);
    PD_ASSERT (i < DEVICES_MAXIMUM_DEVICES);
    
    if (i < p->d_inSize) { return (p->d_inNames + (i * DEVICES_DESCRIPTION)); }
    else {
        return NULL;
    }
}

char *deviceslist_getOutAtIndex (t_deviceslist *p, int i)
{
    PD_ASSERT (i >= 0);
    PD_ASSERT (i < DEVICES_MAXIMUM_DEVICES);
    
    if (i < p->d_outSize) { return (p->d_outNames + (i * DEVICES_DESCRIPTION)); }
    else {
        return NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int deviceslist_containsIn (t_deviceslist *p, char *device)
{
    int i;
    
    for (i = 0; i < p->d_inSize; i++) { 
        if (!strcmp (device, p->d_inNames + (i * DEVICES_DESCRIPTION))) { return i; }
    }
    
    return -1;
}

int deviceslist_containsOut (t_deviceslist *p, char *device)
{
    int i;
    
    for (i = 0; i < p->d_outSize; i++) { 
        if (!strcmp (device, p->d_outNames + (i * DEVICES_DESCRIPTION))) { return i; }
    }
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
