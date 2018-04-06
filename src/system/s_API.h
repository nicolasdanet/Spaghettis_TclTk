
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_API_h_
#define __s_API_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char *midi_nameNative         (void);
t_error midi_getListsNative         (t_deviceslist *);
void    midi_initializeNative       (void);
void    midi_releaseNative          (void);
void    midi_openNative             (t_devicesproperties *);
void    midi_closeNative            (void);
void    midi_pushNextMessageNative  (int, int, int, int);
void    midi_pushNextByteNative     (int, int );
void    midi_pollNative             (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

const char  *audio_nameNative       (void);
t_error audio_getListsNative        (t_deviceslist *);
t_error audio_initializeNative      (void);
void    audio_releaseNative         (void);
void    audio_closeNative           (void);
t_error audio_openNative            (t_devicesproperties *);
int     audio_pollNative            (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Interface for various audio / MIDI backends. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        midi_open                           (void);
void        midi_close                          (void);

void        midi_getDevices                     (t_devicesproperties *p);
void        midi_setDevices                     (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     audio_open                          (void);
void        audio_close                         (void);
int         audio_isOpened                      (void);

void        audio_getDevices                    (t_devicesproperties *p);
void        audio_setDevices                    (t_devicesproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Properties are used temporarily to pass informations. */
/* Whereas lists truly manage devices (used and available). */
/* A device can be referred by its position in the list or by its name. */
/* It is stored literally to avoid any ambiguous situation. */
/* Functions below achieves the look-up. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         midi_deviceAsNumberWithString       (int isOutput, char *name);
t_error     midi_deviceAsStringWithNumber       (int isOutput, int k, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         audio_deviceAsNumberWithString      (int isOutput, char *name);
t_error     audio_deviceAsStringWithNumber      (int isOutput, int k, char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
#endif // __s_API_h_
