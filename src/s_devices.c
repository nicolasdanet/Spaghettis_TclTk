
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
#pragma mark-

#define DEVICES_MAXIMUM_BLOCKSIZE   2048

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

static void devices_init (t_devicesproperties *p)
{
    p->d_blockSize  = AUDIO_DEFAULT_BLOCKSIZE;
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
#pragma mark-

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
#pragma mark-

void devices_setBlockSize (t_devicesproperties *p, int n)
{
    PD_ASSERT (!p->d_isMidi);
    
    if (!PD_IS_POWER_2 (n)) { n = AUDIO_DEFAULT_BLOCKSIZE; }
    
    p->d_blockSize = PD_CLAMP (n, INTERNAL_BLOCKSIZE, DEVICES_MAXIMUM_BLOCKSIZE);
}

void devices_setSampleRate (t_devicesproperties *p, int n)
{
    PD_ASSERT (!p->d_isMidi);
    
    if (n < 1) { n = AUDIO_DEFAULT_SAMPLERATE; }
    
    p->d_sampleRate = n;
}

void devices_checkChannels (t_devicesproperties *p)
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
#pragma mark-

t_error devices_appendMidiIn (t_devicesproperties *p, char *device)
{
    int n = midi_deviceAsNumberWithString (0, device);
    
    PD_ASSERT (p->d_isMidi);
    
    if (n < 0 || p->d_inSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
        p->d_in[p->d_inSize] = n;
        p->d_inSize++;
    }
    
    return PD_ERROR_NONE;
}

t_error devices_appendMidiOut (t_devicesproperties *p, char *device)
{
    int n = midi_deviceAsNumberWithString (1, device);
    
    PD_ASSERT (p->d_isMidi);
    
    if (n < 0 || p->d_outSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
        p->d_out[p->d_outSize] = n;
        p->d_outSize++;
    }
    
    return PD_ERROR_NONE;
}

t_error devices_appendAudioIn (t_devicesproperties *p, char *device, int channels)
{
    int n = audio_deviceAsNumberWithString (0, device);
    
    PD_ASSERT (!p->d_isMidi);
    
    if (n < 0 || p->d_inSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
        p->d_in[p->d_inSize] = n;
        p->d_inChannels[p->d_inSize] = channels;
        p->d_inSize++;
    }
    
    return PD_ERROR_NONE;
}

t_error devices_appendAudioOut (t_devicesproperties *p, char *device, int channels)
{
    int n = audio_deviceAsNumberWithString (1, device); 
    
    PD_ASSERT (!p->d_isMidi);
    
    if (n < 0 || p->d_outSize >= DEVICES_MAXIMUM_IO) { return PD_ERROR; }
    else {
        p->d_out[p->d_outSize] = n;
        p->d_outChannels[p->d_outSize] = channels;
        p->d_outSize++;
    }
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error devices_getInAtIndexAsString (t_devicesproperties *p, int i, char *dest, size_t size)
{
    if (p->d_isMidi) {
        return midi_deviceAsStringWithNumber (0,  devices_getInAtIndex (p, i), dest, size);
    } else {
        return audio_deviceAsStringWithNumber (0, devices_getInAtIndex (p, i), dest, size);
    }
}

t_error devices_getOutAtIndexAsString (t_devicesproperties *p, int i, char *dest, size_t size)
{
    if (p->d_isMidi) { 
        return midi_deviceAsStringWithNumber (1,  devices_getOutAtIndex (p, i), dest, size);
    } else {
        return audio_deviceAsStringWithNumber (1, devices_getOutAtIndex (p, i), dest, size);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
