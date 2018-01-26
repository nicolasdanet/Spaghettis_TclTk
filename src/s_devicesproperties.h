
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_devicesproperties_h_
#define __s_devicesproperties_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DEVICES_MAXIMUM_IO                  8
#define DEVICES_MAXIMUM_CHANNELS            32
#define DEVICES_MAXIMUM_BLOCKSIZE           2048

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _devicesproperties {
    int d_blockSize;
    int d_sampleRate;
    int d_inSize;
    int d_outSize;
    int d_in          [DEVICES_MAXIMUM_IO]; // --
    int d_out         [DEVICES_MAXIMUM_IO]; // --
    int d_inChannels  [DEVICES_MAXIMUM_IO]; // --
    int d_outChannels [DEVICES_MAXIMUM_IO]; // --
    int d_isMidi;
    } t_devicesproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    devices_initAsAudio                 (t_devicesproperties *p);
void    devices_initAsMidi                  (t_devicesproperties *p);
void    devices_setDefaults                 (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    devices_setBlockSize                (t_devicesproperties *p, int n);
void    devices_setSampleRate               (t_devicesproperties *p, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     devices_getBlockSize                (t_devicesproperties *p);
int     devices_getSampleRate               (t_devicesproperties *p);
int     devices_getInSize                   (t_devicesproperties *p);
int     devices_getOutSize                  (t_devicesproperties *p);
int     devices_getInAtIndexAsNumber        (t_devicesproperties *p, int i);
int     devices_getOutAtIndexAsNumber       (t_devicesproperties *p, int i);
int     devices_getInChannelsAtIndex        (t_devicesproperties *p, int i);
int     devices_getOutChannelsAtIndex       (t_devicesproperties *p, int i);
t_error devices_getInAtIndexAsString        (t_devicesproperties *p, int i, char *dest, size_t size);
t_error devices_getOutAtIndexAsString       (t_devicesproperties *p, int i, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    devices_checkDisabled               (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_appendMidiInWithString      (t_devicesproperties *p, char *device);
t_error devices_appendMidiOutWithString     (t_devicesproperties *p, char *device);
t_error devices_appendAudioInWithString     (t_devicesproperties *p, char *device, int channels);
t_error devices_appendAudioOutWithString    (t_devicesproperties *p, char *device, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error devices_appendMidiInWithNumber      (t_devicesproperties *p, int n);
t_error devices_appendMidiOutWithNumber     (t_devicesproperties *p, int n);
t_error devices_appendAudioInWithNumber     (t_devicesproperties *p, int n, int channels);
t_error devices_appendAudioOutWithNumber    (t_devicesproperties *p, int n, int channels);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_devicesproperties_h_
