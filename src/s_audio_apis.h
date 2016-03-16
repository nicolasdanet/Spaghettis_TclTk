
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

extern t_sample *audio_soundIn;
extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

char    *audio_nameNative               (void);
int     audio_getPriorityNative         (int min, int max, int isWatchdog);

t_error audio_initializeNative          (void);
void    audio_releaseNative             (void);


t_error audio_openNative                (int sampleRate,
                                            int numberOfChannelsIn,
                                            int numberOfChannelsOut,
                                            int blockSize,
                                            int deviceIn,
                                            int deviceOut);
                                            
void    audio_closeNative               (void);
int     audio_pollDSPNative             (void);
t_error audio_getListsNative            (char *devicesIn,
                                            int  *numberOfDevicesIn,
                                            char *devicesOut,
                                            int  *numberOfDevicesOut,
                                            int  *canMultiple);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_audio_apis_h_
