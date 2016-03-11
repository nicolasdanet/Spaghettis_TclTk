
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_audio_apis_h_
#define __s_audio_apis_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error pa_initialize                   (void);
void    pa_release                      (void);

t_error pa_open                         (int sampleRate,
                                            int numberOfChannelsIn,
                                            int numberOfChannelsOut,
                                            int blockSize,
                                            int advanceInNumberOfBlocks,
                                            int deviceIn,
                                            int deviceOut);
                                            
void    pa_close                        (void);
int     pa_pollDSP                      (void);
t_error pa_getLists                     (char *devicesIn,
                                            int  *numberOfDevicesIn,
                                            char *devicesOut,
                                            int  *numberOfDevicesOut,
                                            int  *canMultiple);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error jack_open                       (int numberOfChannelsIn, int numberOfChannelsOut);
void    jack_close                      (void);
int     jack_pollDSP                    (void);
t_error jack_getLists                   (char *devicesIn,
                                            int  *numberOfDevicesIn,
                                            char *devicesOut,
                                            int  *numberOfDevicesOut,
                                            int  *canMulitple);
                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void oss_initialize                     (void);

int  oss_send_dacs                      (void);
void oss_close_audio                    (void);
int  oss_open_audio                     (int naudioindev,
                                            int *audioindev,
                                            int nchindev,
                                            int *chindev,
                                            int naudiooutdev,
                                            int *audiooutdev,
                                            int nchoutdev,
                                            int *choutdev,
                                            int rate,
                                            int blocksize);
                                
void oss_getdevs                        (char *indevlist,
                                            int *nindevs,
                                            char *outdevlist,
                                            int *noutdevs,
                                            int *canmulti);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void alsa_putzeros                      (int iodev, int n);
void alsa_getzeros                      (int iodev, int n);
void alsa_printstate                    (void);
void alsa_adddev                        (char *name);

int  alsa_send_dacs                     (void);
void alsa_close_audio                   (void);
int  alsa_open_audio                    (int naudioindev,
                                            int *audioindev,
                                            int nchindev,
                                            int *chindev,
                                            int naudiooutdev,
                                            int *audiooutdev,
                                            int nchoutdev,
                                            int *choutdev,
                                            int rate,
                                            int blocksize);
                                
void alsa_getdevs                       (char *indevlist,
                                            int *nindevs,
                                            char *outdevlist,
                                            int *noutdevs,
                                            int *canmulti);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error dummy_open                      (void);
void    dummy_close                     (void);
int     dummy_pollDSP                   (void);

t_error dummy_getLists                  (char *devicesIn, 
                                            int  *numberOfDevicesIn,
                                            char *devicesOut,
                                            int  *numberOfDecivesOut,
                                            int  *canMultiple);
                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_audio_apis_h_
