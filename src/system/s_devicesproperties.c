
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

static void devices_init (t_devicesproperties *p)
{
    p->d_sampleRate = AUDIO_DEFAULT_SAMPLERATE;
    p->d_inSize     = 0;
    p->d_outSize    = 0;

    memset (p->d_in,            0, DEVICES_MAXIMUM_IO * sizeof (int));
    memset (p->d_out,           0, DEVICES_MAXIMUM_IO * sizeof (int));
    memset (p->d_inChannels,    0, DEVICES_MAXIMUM_IO * sizeof (int));
    memset (p->d_outChannels,   0, DEVICES_MAXIMUM_IO * sizeof (int));
}

void devices_initAsAudio (t_devicesproperties *p)
{
    devices_init (p);
    
    p->d_isMidi = 0;
}

void devices_initAsMidi (t_devicesproperties *p)
{
    devices_init (p);
    
    p->d_isMidi = 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void devices_setDefaultsAudio (t_devicesproperties *p)
{
    if (p->d_inSize == 0) { 
        p->d_in[0]              = 0;
        p->d_inChannels[0]      = 2;
        p->d_inSize             = 1;
    }
    
    if (p->d_outSize == 0) { 
        p->d_out[0]             = 0;
        p->d_outChannels[0]     = 2;
        p->d_outSize            = 1;
    }
}

static void devices_setDefaultsMidi (t_devicesproperties *p)
{
    if (p->d_inSize == 0) { 
        p->d_in[0]              = 0;
        p->d_inSize             = 1;
    }
    
    if (p->d_outSize == 0) { 
        p->d_out[0]             = 0;
        p->d_outSize            = 1;
    }
}

void devices_setDefaults (t_devicesproperties *p)
{
    if (p->d_isMidi) { devices_setDefaultsMidi (p); }
    else {
        devices_setDefaultsAudio (p);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void devices_setSampleRate (t_devicesproperties *p, int n)
{
    PD_ASSERT (!p->d_isMidi);
    
    if (n < 1) { n = AUDIO_DEFAULT_SAMPLERATE; }
    
    p->d_sampleRate = n;
}

int devices_getSampleRate (t_devicesproperties *p)
{
    return p->d_sampleRate;
}

int devices_getInSize (t_devicesproperties *p)
{
    return p->d_inSize;
}

int devices_getOutSize (t_devicesproperties *p)
{
    return p->d_outSize;
}

int devices_getInAtIndexAsNumber (t_devicesproperties *p, int i)
{
    return (i < p->d_inSize) ? p->d_in[i] : -1;
}

int devices_getOutAtIndexAsNumber (t_devicesproperties *p, int i)
{
    return (i < p->d_outSize) ? p->d_out[i] : -1;
}

int devices_getInChannelsAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (!p->d_isMidi);
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_inChannels[i];
}

int devices_getOutChannelsAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (!p->d_isMidi);
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_outChannels[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void devices_checkDisabled (t_devicesproperties *p)
{
    int i;
    
    PD_ASSERT (!p->d_isMidi);
    
    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {
        p->d_inChannels[i]  = PD_MAX (0, p->d_inChannels[i]);
        p->d_outChannels[i] = PD_MAX (0, p->d_outChannels[i]);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_appendMidiInWithNumber (t_devicesproperties *p, int n)
{
    PD_ASSERT (p->d_isMidi);
    
    if (n < 0 || p->d_inSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
        p->d_in[p->d_inSize] = n;
        p->d_inSize++;
    }
    
    return PD_ERROR_NONE;
}

t_error devices_appendMidiOutWithNumber (t_devicesproperties *p, int n)
{
    PD_ASSERT (p->d_isMidi);
    
    if (n < 0 || p->d_outSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
        p->d_out[p->d_outSize] = n;
        p->d_outSize++;
    }
    
    return PD_ERROR_NONE;
}

t_error devices_appendAudioInWithNumber (t_devicesproperties *p, int n, int channels)
{
    PD_ASSERT (!p->d_isMidi);
    
    if (n < 0 || p->d_inSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
    //
    p->d_in[p->d_inSize] = n;
    p->d_inChannels[p->d_inSize] = PD_CLAMP (channels, -DEVICES_MAXIMUM_CHANNELS, DEVICES_MAXIMUM_CHANNELS);
    p->d_inSize++;
    //
    }
    
    return PD_ERROR_NONE;
}

t_error devices_appendAudioOutWithNumber (t_devicesproperties *p, int n, int channels)
{
    PD_ASSERT (!p->d_isMidi);
    
    if (n < 0 || p->d_outSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
    //
    p->d_out[p->d_outSize] = n;
    p->d_outChannels[p->d_outSize] = PD_CLAMP (channels, -DEVICES_MAXIMUM_CHANNELS, DEVICES_MAXIMUM_CHANNELS);
    p->d_outSize++;
    //
    }
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_appendMidiInWithString (t_devicesproperties *p, char *device)
{
    return devices_appendMidiInWithNumber (p, midi_deviceAsNumberWithString (0, device));
}

t_error devices_appendMidiOutWithString (t_devicesproperties *p, char *device)
{
    return devices_appendMidiOutWithNumber (p, midi_deviceAsNumberWithString (1, device));
}

t_error devices_appendAudioInWithString (t_devicesproperties *p, char *device, int channels)
{
    return devices_appendAudioInWithNumber (p, audio_deviceAsNumberWithString (0, device), channels);
}

t_error devices_appendAudioOutWithString (t_devicesproperties *p, char *device, int channels)
{
    return devices_appendAudioOutWithNumber (p, audio_deviceAsNumberWithString (1, device), channels);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_getInAtIndexAsString (t_devicesproperties *p, int i, char *dest, size_t size)
{
    if (p->d_isMidi) {
        return midi_deviceAsStringWithNumber (0,  devices_getInAtIndexAsNumber (p, i), dest, size);
    } else {
        return audio_deviceAsStringWithNumber (0, devices_getInAtIndexAsNumber (p, i), dest, size);
    }
}

t_error devices_getOutAtIndexAsString (t_devicesproperties *p, int i, char *dest, size_t size)
{
    if (p->d_isMidi) { 
        return midi_deviceAsStringWithNumber (1,  devices_getOutAtIndexAsNumber (p, i), dest, size);
    } else {
        return audio_deviceAsStringWithNumber (1, devices_getOutAtIndexAsNumber (p, i), dest, size);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
