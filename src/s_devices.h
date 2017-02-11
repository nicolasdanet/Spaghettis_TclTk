
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
    int d_isMidi;
    } t_devicesproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        devices_initAsAudio                 (t_devicesproperties *p);
void        devices_initAsMidi                  (t_devicesproperties *p);
void        devices_setDefaults                 (t_devicesproperties *p);

void        devices_setBlockSize                (t_devicesproperties *p, int n);
void        devices_setSampleRate               (t_devicesproperties *p, int n);
void        devices_checkChannels               (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     devices_appendMidiIn                (t_devicesproperties *p, char *device);
t_error     devices_appendMidiOut               (t_devicesproperties *p, char *device);
t_error     devices_appendAudioIn               (t_devicesproperties *p, char *device, int channels);
t_error     devices_appendAudioOut              (t_devicesproperties *p, char *device, int channels);

t_error     devices_getInAtIndexAsString        (t_devicesproperties *p, int i, char *dest, size_t size);
t_error     devices_getOutAtIndexAsString       (t_devicesproperties *p, int i, char *dest, size_t size);

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
    PD_ASSERT (!p->d_isMidi);
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
    PD_ASSERT (!p->d_isMidi);
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_outChannels[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _deviceslist {
    int     d_inSize;
    int     d_outSize;
    char    d_inNames   [DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION];
    char    d_outNames  [DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION];
    } t_deviceslist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        deviceslist_init                    (t_deviceslist *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     deviceslist_appendIn                (t_deviceslist *p, const char *device);
t_error     deviceslist_appendOut               (t_deviceslist *p, const char *device);
t_error     deviceslist_appendMidiInAsNumber    (t_deviceslist *p, int n);
t_error     deviceslist_appendMidiOutAsNumber   (t_deviceslist *p, int n);
t_error     deviceslist_appendAudioInAsNumber   (t_deviceslist *p, int n);
t_error     deviceslist_appendAudioOutAsNumber  (t_deviceslist *p, int n);

char        *deviceslist_getInAtIndex           (t_deviceslist *p, int i);
char        *deviceslist_getOutAtIndex          (t_deviceslist *p, int i);

int         deviceslist_containsIn              (t_deviceslist *p, char *device);
int         deviceslist_containsOut             (t_deviceslist *p, char *device);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int deviceslist_getInSize (t_deviceslist *p)
{
    return p->d_inSize;
}

static inline int deviceslist_getOutSize (t_deviceslist *p)
{
    return p->d_outSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_devices_h_
