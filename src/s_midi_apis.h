
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_midi_apis_h_
#define __s_midi_apis_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char    *midi_nameNative                (void);

void    midi_initializeNative           (void);
void    midi_releaseNative              (void);

void    midi_openNative                 (int numberOfDevicesIn, 
                                            int *devicesIn, 
                                            int numberOfDevicesOut, 
                                            int *devicesOut);
                                        
void    midi_closeNative                (void);
void    midi_pushNextMessageNative      (int port, int a, int b, int c);
void    midi_pushNextByteNative         (int port, int a);
void    midi_pollNative                 (void);
t_error midi_getListsNative             (char *devicesIn, 
                                            int  *numberOfDevicesIn, 
                                            char *devicesOut, 
                                            int  *numberOfDevicesOut);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void midi_initialize (void) 
{ 
    return midi_initializeNative();
}

static inline void midi_release (void)
{ 
    return midi_releaseNative();    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_midi_apis_h_
