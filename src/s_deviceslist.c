
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void deviceslist_init (t_deviceslist *p)
{
    p->d_blockSize  = AUDIO_DEFAULT_BLOCKSIZE;
    p->d_sampleRate = AUDIO_DEFAULT_SAMPLERATE;
    p->d_inSize     = 0;
    p->d_outSize    = 0;

    memset (p->d_inChannels,  0, DEVICES_MAXIMUM_IO * sizeof (int));
    memset (p->d_outChannels, 0, DEVICES_MAXIMUM_IO * sizeof (int));
    memset (p->d_inNames,     0, DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION * sizeof (char));
    memset (p->d_outNames,    0, DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION * sizeof (char));
}

void deviceslist_copy (t_deviceslist *dest, t_deviceslist *src)
{
    dest->d_blockSize  = src->d_blockSize;
    dest->d_sampleRate = src->d_sampleRate;
    dest->d_inSize     = src->d_inSize;
    dest->d_outSize    = src->d_outSize;
    
    memcpy (dest->d_inChannels,  src->d_inChannels,  DEVICES_MAXIMUM_IO * sizeof (int));
    memcpy (dest->d_outChannels, src->d_outChannels, DEVICES_MAXIMUM_IO * sizeof (int));
    memcpy (dest->d_inNames,     src->d_inNames,  DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION * sizeof (char));
    memcpy (dest->d_outNames,    src->d_outNames, DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION * sizeof (char));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void deviceslist_setBlockSize (t_deviceslist *p, int n)
{
    p->d_blockSize = n;         /* Expect store to be thread-safe. */
}

void deviceslist_setSampleRate (t_deviceslist *p, int n)
{
    p->d_sampleRate = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int deviceslist_getBlockSize (t_deviceslist *p)
{
    return p->d_blockSize;
}

int deviceslist_getSampleRate (t_deviceslist *p)
{
    return p->d_sampleRate;
}

int deviceslist_getInSize (t_deviceslist *p)
{
    return p->d_inSize;
}

int deviceslist_getOutSize (t_deviceslist *p)
{
    return p->d_outSize;
}

int deviceslist_getInChannelsAtIndex (t_deviceslist *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_inChannels[i];
}

int deviceslist_getOutChannelsAtIndex (t_deviceslist *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_outChannels[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error deviceslist_appendMidiIn (t_deviceslist *p, const char *device)
{
    if (p->d_inSize < DEVICES_MAXIMUM_IO) {
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

t_error deviceslist_appendMidiOut (t_deviceslist *p, const char *device)
{
    if (p->d_outSize < DEVICES_MAXIMUM_IO) {
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

t_error deviceslist_appendAudioIn (t_deviceslist *p, const char *device, int channels)
{
    if (p->d_inSize < DEVICES_MAXIMUM_IO) {
        char *s = p->d_inNames + (p->d_inSize * DEVICES_DESCRIPTION);
        t_error err = string_copy (s, DEVICES_DESCRIPTION, device);
        int t = PD_CLAMP (channels, -DEVICES_MAXIMUM_CHANNELS, DEVICES_MAXIMUM_CHANNELS);
        if (!err) { p->d_inChannels[p->d_inSize] = t; p->d_inSize++; }
        else {
            string_clear (s, DEVICES_DESCRIPTION);
        }
        return err;
    }
    
    return PD_ERROR;
}

t_error deviceslist_appendAudioOut (t_deviceslist *p, const char *device, int channels)
{
    if (p->d_outSize < DEVICES_MAXIMUM_IO) {
        char *s = p->d_outNames + (p->d_outSize * DEVICES_DESCRIPTION);
        t_error err = string_copy (s, DEVICES_DESCRIPTION, device);
        int t = PD_CLAMP (channels, -DEVICES_MAXIMUM_CHANNELS, DEVICES_MAXIMUM_CHANNELS);
        if (!err) { p->d_outChannels[p->d_outSize] = t; p->d_outSize++; }
        else {
            string_clear (s, DEVICES_DESCRIPTION);
        }
        return err;
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error deviceslist_appendMidiInAsNumber (t_deviceslist *p, int n)
{
    if (p->d_inSize < DEVICES_MAXIMUM_IO) {
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

static t_error deviceslist_appendMidiOutAsNumber (t_deviceslist *p, int n)
{
    if (p->d_outSize < DEVICES_MAXIMUM_IO) {
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

static t_error deviceslist_appendAudioInAsNumber (t_deviceslist *p, int n, int channels)
{
    if (p->d_inSize < DEVICES_MAXIMUM_IO) {
    //
    char *s = p->d_inNames + (p->d_inSize * DEVICES_DESCRIPTION);
    
    if (!audio_deviceAsStringWithNumber (0, n, s, DEVICES_DESCRIPTION)) {
    //
    p->d_inChannels[p->d_inSize] = PD_CLAMP (channels, -DEVICES_MAXIMUM_CHANNELS, DEVICES_MAXIMUM_CHANNELS);
    p->d_inSize++;
    return PD_ERROR_NONE;
    //
    }
    //
    }
    
    return PD_ERROR;
}

static t_error deviceslist_appendAudioOutAsNumber (t_deviceslist *p, int n, int channels)
{
    if (p->d_outSize < DEVICES_MAXIMUM_IO) {
    //
    char *s = p->d_outNames + (p->d_outSize * DEVICES_DESCRIPTION);
    
    if (!audio_deviceAsStringWithNumber (1, n, s, DEVICES_DESCRIPTION)) {
    //
    p->d_outChannels[p->d_outSize] = PD_CLAMP (channels, -DEVICES_MAXIMUM_CHANNELS, DEVICES_MAXIMUM_CHANNELS);
    p->d_outSize++;
    return PD_ERROR_NONE;
    //
    }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char *deviceslist_getInAtIndexAsString (t_deviceslist *p, int i)
{
    PD_ASSERT (i >= 0);
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    if (i < p->d_inSize) { return (p->d_inNames + (i * DEVICES_DESCRIPTION)); }
    else {
        return NULL;
    }
}

char *deviceslist_getOutAtIndexAsString (t_deviceslist *p, int i)
{
    PD_ASSERT (i >= 0);
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    if (i < p->d_outSize) { return (p->d_outNames + (i * DEVICES_DESCRIPTION)); }
    else {
        return NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void deviceslist_setDevices (t_deviceslist *l, t_devicesproperties *p)
{
    int i;
    
    deviceslist_init (l);
        
    if (p->d_isMidi) {
    
        for (i = 0; i < devices_getInSize (p); i++) {
            deviceslist_appendMidiInAsNumber (l, devices_getInAtIndex (p, i));
        }
        
        for (i = 0; i < devices_getOutSize (p); i++) {
            deviceslist_appendMidiOutAsNumber (l, devices_getOutAtIndex (p, i));
        }
    
    } else {
        
        deviceslist_setBlockSize (l, devices_getBlockSize (p));
        deviceslist_setSampleRate (l, devices_getSampleRate (p));
        
        for (i = 0; i < devices_getInSize (p); i++) {
            deviceslist_appendAudioInAsNumber (l,
                devices_getInAtIndex (p, i),
                devices_getInChannelsAtIndex (p, i));
        }

        for (i = 0; i < devices_getOutSize (p); i++) {
            deviceslist_appendAudioOutAsNumber (l,
                devices_getOutAtIndex (p, i),
                devices_getOutChannelsAtIndex (p, i));
        }
    }
}

void deviceslist_getDevices (t_deviceslist *l, t_devicesproperties *p)
{
    int i;
    
    if (p->d_isMidi) {
        
        for (i = 0; i < deviceslist_getInSize (l); i++) {
            devices_appendMidiIn (p, deviceslist_getInAtIndexAsString (l, i));
        }
            
        for (i = 0; i < deviceslist_getOutSize (l); i++) {
            devices_appendMidiOut (p, deviceslist_getOutAtIndexAsString (l, i));
        }
    
    } else {
    
        devices_setBlockSize (p, deviceslist_getBlockSize (l));
        devices_setSampleRate (p, deviceslist_getSampleRate (l));
        
        for (i = 0; i < deviceslist_getInSize (l); i++) {
            devices_appendAudioIn (p,
                deviceslist_getInAtIndexAsString (l, i),
                deviceslist_getInChannelsAtIndex (l, i));
        }
        
        for (i = 0; i < deviceslist_getOutSize (l); i++) {
            devices_appendAudioOut (p,
                deviceslist_getOutAtIndexAsString (l, i),
                deviceslist_getOutChannelsAtIndex (l, i));
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

int deviceslist_getTotalOfChannelsIn (t_deviceslist *p)
{
    int i, k = 0;
    
    for (i = 0; i < p->d_inSize; i++) { 
        int n = p->d_inChannels[i]; if (n > 0) { k += n; }
    }
    
    return k;
}

int deviceslist_getTotalOfChannelsOut (t_deviceslist *p)
{
    int i, k = 0;
    
    for (i = 0; i < p->d_outSize; i++) { 
        int n = p->d_outChannels[i]; if (n > 0) { k += n; }
    }
    
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
