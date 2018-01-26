
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_apis_h_
#define __s_apis_h_

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
#endif // __s_apis_h_
