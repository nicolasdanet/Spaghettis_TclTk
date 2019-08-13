
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_API_h_
#define __s_API_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char  *midi_nameNative                    (void);
t_error     midi_getListsNative                 (t_deviceslist *);
void        midi_initializeNative               (void);
void        midi_releaseNative                  (void);
void        midi_openNative                     (t_devices *);
void        midi_closeNative                    (void);
void        midi_pushNative                     (int port, int status, int a, int b);
void        midi_pushSysexNative                (int port, int argc, t_atom *argv);
void        midi_pollNative                     (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char  *audio_nameNative                   (void);
int         audio_getVectorSizeNative           (void);
t_error     audio_getListsNative                (t_deviceslist *);
t_error     audio_initializeNative              (void);
void        audio_releaseNative                 (void);
void        audio_closeNative                   (void);
t_error     audio_openNative                    (t_devices *);
int         audio_pollNative                    (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Interface for various audio / MIDI backends. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        midi_open                           (void);
void        midi_close                          (void);

void        midi_getDevices                     (t_devices *p);
void        midi_setDevices                     (t_devices *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     audio_open                          (void);
void        audio_close                         (void);
int         audio_isOpened                      (void);

void        audio_getDevices                    (t_devices *p);
void        audio_setDevices                    (t_devices *p);

t_error     audio_check                         (t_devices *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         midi_deviceAsNumber                 (int isOutput, t_symbol *name);
t_error     midi_deviceAsString                 (int isOutput, int k, char *dest, size_t size);

t_symbol    *midi_deviceAsSymbol                (int isOutput, int k);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         audio_deviceAsNumber                (int isOutput, t_symbol *name);
t_error     audio_deviceAsString                (int isOutput, int k, char *dest, size_t size);

t_symbol    *audio_deviceAsSymbol               (int isOutput, int k);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int audio_getVectorSize (void)
{
    return audio_getVectorSizeNative();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_API_h_
