
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_deviceslist_h_
#define __s_deviceslist_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DEVICES_DESCRIPTION     128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _deviceslist {
    int     d_blockSize;
    int     d_sampleRate;
    int     d_inSize;
    int     d_outSize;
    int     d_inChannels  [DEVICES_MAXIMUM_IO];                         // --
    int     d_outChannels [DEVICES_MAXIMUM_IO];                         // --
    char    d_inNames     [DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION];   // --
    char    d_outNames    [DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION];   // --
    } t_deviceslist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    deviceslist_init                    (t_deviceslist *p);
void    deviceslist_copy                    (t_deviceslist *dest, t_deviceslist *src);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    deviceslist_setDevices              (t_deviceslist *l, t_devicesproperties *p);
void    deviceslist_getDevices              (t_deviceslist *l, t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    deviceslist_setBlockSize            (t_deviceslist *p, int n);
void    deviceslist_setSampleRate           (t_deviceslist *p, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     deviceslist_getBlockSize            (t_deviceslist *p);
int     deviceslist_getSampleRate           (t_deviceslist *p);
int     deviceslist_getInSize               (t_deviceslist *p);
int     deviceslist_getOutSize              (t_deviceslist *p);
int     deviceslist_getInChannelsAtIndex    (t_deviceslist *p, int i);
int     deviceslist_getOutChannelsAtIndex   (t_deviceslist *p, int i);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error deviceslist_appendMidiIn            (t_deviceslist *p, const char *device);
t_error deviceslist_appendMidiOut           (t_deviceslist *p, const char *device);
t_error deviceslist_appendAudioIn           (t_deviceslist *p, const char *device, int channels);
t_error deviceslist_appendAudioOut          (t_deviceslist *p, const char *device, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char    *deviceslist_getInAtIndexAsString   (t_deviceslist *p, int i);
char    *deviceslist_getOutAtIndexAsString  (t_deviceslist *p, int i);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     deviceslist_containsIn              (t_deviceslist *p, char *device);
int     deviceslist_containsOut             (t_deviceslist *p, char *device);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     deviceslist_getTotalOfChannelsIn    (t_deviceslist *p);
int     deviceslist_getTotalOfChannelsOut   (t_deviceslist *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_deviceslist_h_
