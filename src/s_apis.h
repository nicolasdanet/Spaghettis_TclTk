
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_apis_h_
#define __s_apis_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        midi_start                              (void);
void        midi_synchronise                        (void);
void        midi_poll                               (void);
void        midi_receive                            (int port, int byte);
void        midi_broadcast                          (int port, int hasOneByte, int a, int b, int c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        midi_requireDialog                      (void *dummy);
void        midi_fromDialog                         (void *dummy, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        midi_open                               (void);
void        midi_close                              (void);

void        midi_getDevices                         (int *numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int *numberOfDevicesOut,
                                                        int *devicesOut);

void        midi_setDevices                         (t_devicesproperties *p);

int         midi_deviceAsNumberWithString           (int isOutput, char *name);
t_error     midi_deviceAsStringWithNumber           (int isOutput, int k, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     audio_initialize                        (void);
void        audio_release                           (void);
int         audio_pollDSP                           (void);
t_error     audio_stopDSP                           (void);
t_error     audio_startDSP                          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        audio_initializeMemory                  (int usedChannelsIn, int usedChannelsOut);
void        audio_shrinkChannelsIn                  (int numberOfChannelsIn);
void        audio_shrinkChannelsOut                 (int numberOfChannelsOut);

void        audio_setSampleRate                     (t_float sampleRate);
void        audio_setBlockSize                      (int blockSize);

int         audio_getChannelsIn                     (void);
int         audio_getChannelsOut                    (void);
t_float     audio_getSampleRate                     (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        audio_requireDialog                     (void *dummy);
void        audio_fromDialog                        (void *dummy, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     audio_open                              (void);
void        audio_close                             (void);
int         audio_isOpened                          (void);

void        audio_getDevices                        (int *numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int *channelsIn,
                                                        int *numberOfDevicesOut,
                                                        int *devicesOut,
                                                        int *channelsOut,
                                                        int *sampleRate,
                                                        int *blockSize);

void        audio_setDevices                        (t_devicesproperties *p, int sampleRate, int blockSize);

int         audio_deviceAsNumberWithString          (int isOutput, char *name);
t_error     audio_deviceAsStringWithNumber          (int isOutput, int k, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_apis_h_
