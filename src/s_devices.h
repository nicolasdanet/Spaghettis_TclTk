
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_devices_h_
#define __s_devices_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DEVICES_MAXIMUM_IO          8
#define DEVICES_MAXIMUM_CHANNELS    32
#define DEVICES_MAXIMUM_DEVICES     16

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DEVICES_DESCRIPTION         128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _devicesproperties {
    int d_blockSize;
    int d_sampleRate;
    int d_inSize;
    int d_outSize;
    int d_in            [DEVICES_MAXIMUM_IO];
    int d_out           [DEVICES_MAXIMUM_IO];
    int d_inChannels    [DEVICES_MAXIMUM_IO];
    int d_outChannels   [DEVICES_MAXIMUM_IO];
    } t_devicesproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        devices_init                        (t_devicesproperties *p);
void        devices_setDefaultsMidi             (t_devicesproperties *p);
void        devices_setDefaultsAudio            (t_devicesproperties *p);
void        devices_setBlockSize                (t_devicesproperties *p, int n);
void        devices_setSampleRate               (t_devicesproperties *p, int n);
void        devices_checkForDisabledChannels    (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     devices_appendMidiIn                (t_devicesproperties *p, char *device);
t_error     devices_appendMidiOut               (t_devicesproperties *p, char *device);
t_error     devices_appendAudioIn               (t_devicesproperties *p, char *device, int channels);
t_error     devices_appendAudioOut              (t_devicesproperties *p, char *device, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

static inline int devices_getBlockSize (t_devicesproperties *p)
{
    return p->d_blockSize;
}

static inline int devices_getSampleRate (t_devicesproperties *p)
{
    return p->d_sampleRate;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

static inline int devices_getInSize (t_devicesproperties *p)
{
    return p->d_inSize;
}

static inline int devices_getInAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_in[i];
}

static inline int devices_getInChannelsAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_inChannels[i];
}

static inline int devices_getOutSize (t_devicesproperties *p)
{
    return p->d_outSize;
}

static inline int devices_getOutAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_out[i];
}

static inline int devices_getOutChannelsAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_outChannels[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_devices_h_
