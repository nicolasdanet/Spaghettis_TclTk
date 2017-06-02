
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_devices_h_
#define __s_devices_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DEVICES_MAXIMUM_IO              8
#define DEVICES_MAXIMUM_CHANNELS        32
#define DEVICES_MAXIMUM_BLOCKSIZE       2048

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DEVICES_DESCRIPTION             128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _devicesproperties {
    int d_blockSize;
    int d_sampleRate;
    int d_inSize;
    int d_outSize;
    int d_in[DEVICES_MAXIMUM_IO];
    int d_out[DEVICES_MAXIMUM_IO];
    int d_inChannels[DEVICES_MAXIMUM_IO];
    int d_outChannels[DEVICES_MAXIMUM_IO];
    int d_isMidi;
    } t_devicesproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        devices_initAsAudio                 (t_devicesproperties *p);
void        devices_initAsMidi                  (t_devicesproperties *p);


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        devices_setDefaults                 (t_devicesproperties *p);
void        devices_setBlockSize                (t_devicesproperties *p, int n);
void        devices_setSampleRate               (t_devicesproperties *p, int n);
void        devices_checkDisabled               (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     devices_appendMidiIn                (t_devicesproperties *p, char *device);
t_error     devices_appendMidiOut               (t_devicesproperties *p, char *device);
t_error     devices_appendAudioIn               (t_devicesproperties *p, char *device, int channels);
t_error     devices_appendAudioOut              (t_devicesproperties *p, char *device, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     devices_appendMidiInAsNumber        (t_devicesproperties *p, int n);
t_error     devices_appendMidiOutAsNumber       (t_devicesproperties *p, int n);
t_error     devices_appendAudioInAsNumber       (t_devicesproperties *p, int n, int channels);
t_error     devices_appendAudioOutAsNumber      (t_devicesproperties *p, int n, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     devices_getInAtIndexAsString        (t_devicesproperties *p, int i, char *dest, size_t size);
t_error     devices_getOutAtIndexAsString       (t_devicesproperties *p, int i, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static inline int devices_getInSize (t_devicesproperties *p)
{
    return p->d_inSize;
}

static inline int devices_getOutSize (t_devicesproperties *p)
{
    return p->d_outSize;
}

static inline int devices_getInAtIndex (t_devicesproperties *p, int i)
{
    return (i < p->d_inSize) ? p->d_in[i] : -1;
}

static inline int devices_getOutAtIndex (t_devicesproperties *p, int i)
{
    return (i < p->d_outSize) ? p->d_out[i] : -1;
}

static inline int devices_getInChannelsAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (!p->d_isMidi);
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_inChannels[i];
}

static inline int devices_getOutChannelsAtIndex (t_devicesproperties *p, int i)
{
    PD_ASSERT (!p->d_isMidi);
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_outChannels[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _deviceslist {
    int     d_blockSize;
    int     d_sampleRate;
    int     d_inSize;
    int     d_outSize;
    int     d_inChannels[DEVICES_MAXIMUM_IO];
    int     d_outChannels[DEVICES_MAXIMUM_IO];
    char    d_inNames[DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION];
    char    d_outNames[DEVICES_MAXIMUM_IO * DEVICES_DESCRIPTION];
    } t_deviceslist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        deviceslist_setDevices                  (t_deviceslist *l, t_devicesproperties *p);
void        deviceslist_getDevices                  (t_deviceslist *l, t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        deviceslist_init                        (t_deviceslist *p);
void        deviceslist_copy                        (t_deviceslist *dest, t_deviceslist *src);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     deviceslist_appendMidiIn                (t_deviceslist *p, const char *device);
t_error     deviceslist_appendMidiOut               (t_deviceslist *p, const char *device);
t_error     deviceslist_appendAudioIn               (t_deviceslist *p, const char *device, int channels);
t_error     deviceslist_appendAudioOut              (t_deviceslist *p, const char *device, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char        *deviceslist_getInAtIndexAsString       (t_deviceslist *p, int i);
char        *deviceslist_getOutAtIndexAsString      (t_deviceslist *p, int i);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         deviceslist_containsIn                  (t_deviceslist *p, char *device);
int         deviceslist_containsOut                 (t_deviceslist *p, char *device);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         deviceslist_getTotalOfChannelsIn        (t_deviceslist *p);
int         deviceslist_getTotalOfChannelsOut       (t_deviceslist *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void deviceslist_setBlockSize (t_deviceslist *p, int n)
{
    p->d_blockSize = n;         /* Expect store to be thread-safe. */
}

static inline void deviceslist_setSampleRate (t_deviceslist *p, int n)
{
    p->d_sampleRate = n;
}

static inline int deviceslist_getBlockSize (t_deviceslist *p)
{
    return p->d_blockSize;
}

static inline int deviceslist_getSampleRate (t_deviceslist *p)
{
    return p->d_sampleRate;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int deviceslist_getInSize (t_deviceslist *p)
{
    return p->d_inSize;
}

static inline int deviceslist_getOutSize (t_deviceslist *p)
{
    return p->d_outSize;
}

static inline int deviceslist_getInChannelsAtIndex (t_deviceslist *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_inChannels[i];
}

static inline int deviceslist_getOutChannelsAtIndex (t_deviceslist *p, int i)
{
    PD_ASSERT (i < DEVICES_MAXIMUM_IO);
    
    return p->d_outChannels[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_devices_h_
