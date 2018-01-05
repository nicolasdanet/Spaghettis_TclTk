
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_system_h_
#define __s_system_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define INTERNAL_BLOCKSIZE          64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SCHEDULER_AUDIO_STOP        0
#define SCHEDULER_AUDIO_POLL        1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define AUDIO_DEFAULT_BLOCKSIZE     64
#define AUDIO_DEFAULT_SAMPLERATE    44100

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that LCM of (32000, 44100, 48000, 88200, 96000) is 14112000. */

#define SYSTIME_PER_MILLISECOND     (32.0 * 441.0)
#define SYSTIME_PER_SECOND          (SYSTIME_PER_MILLISECOND * 1000.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef void (*t_pollfn)            (void *p, int fd);
typedef void (*t_notifyfn)          (void *owner, int fd);
typedef void (*t_receivefn)         (void *owner, t_buffer *b);
typedef void (*t_clockfn)           (void *owner);
typedef void (*t_drawfn)            (t_gobj *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    DACS_NO     = 0,
    DACS_YES    = 1,
    DACS_SLEPT  = 2
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         main_entry                              (int argc, char **argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     scheduler_main                          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Usable in DSP perform. */

t_systime   scheduler_getLogicalTime                (void);
t_systime   scheduler_getLogicalTimeAfter           (double ms);
double      scheduler_getMillisecondsSince          (t_systime systime);    
double      scheduler_getUnitsSince                 (t_systime systime, double unit, int isSamples);
void        scheduler_setAudioState                 (int state);
void        scheduler_needToExit                    (void);
void        scheduler_needToExitWithError           (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     priority_privilegeStart                 (void);
t_error     priority_privilegeDrop                  (void);
t_error     priority_privilegeRestore               (void);
t_error     priority_privilegeRelinquish            (void);

t_error     priority_setPolicy                      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_receiver  *receiver_new                           (void *owner,
                                                            int fd,
                                                            t_notifyfn notify,          /* Socket closed. */
                                                            t_receivefn receive,        /* Data received. */
                                                            int isUdp, 
                                                            int isBinary);

void        receiver_free                           (t_receiver *x);
int         receiver_isClosed                       (t_receiver *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         monitor_blocking                        (int microseconds);
int         monitor_nonBlocking                     (void);
void        monitor_addPoller                       (int fd, t_pollfn fn, void *ptr);
void        monitor_removePoller                    (int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        gui_jobAdd                              (void *owner, t_glist *glist, t_drawfn f);
void        gui_jobRemove                           (void *owner);

void        gui_vAdd                                (const char *format, ...);
void        gui_add                                 (const char *s);
int         gui_flush                               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     interface_start                         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        midi_start                              (void);
void        midi_poll                               (void);
void        midi_receive                            (int port, int byte);
void        midi_broadcast                          (int port, int hasOneByte, int a, int b, int c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         audio_poll                              (void);
t_error     audio_stop                              (void);
t_error     audio_start                             (void);

void        audio_setSampleRate                     (t_float f);
void        audio_setBlockSize                      (int n);

t_float     audio_getSampleRate                     (void);
int         audio_getBlockSize                      (void);
int         audio_getTotalOfChannelsIn              (void);
int         audio_getTotalOfChannelsOut             (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         file_openWrite                          (const char *filepath);
int         file_openRead                           (const char *filepath);

int         file_openReadWithDirectoryAndName       (const char *directory,
                                                        const char *name,
                                                        const char *extension,
                                                        t_fileproperties *p);

int         file_openReadConsideringSearchPath      (const char *directory,
                                                        const char *name,
                                                        const char *extension,
                                                        t_fileproperties *p);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist  *searchpath_get                         (void);

void        searchpath_appendPath                   (char *filepath);
void        searchpath_setEncoded                   (int argc, t_atom *argv);


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        path_slashToBackslashIfNecessary        (char *s);
void        path_backslashToSlashIfNecessary        (char *s);
int         path_isFileExist                        (const char *filepath);
int         path_isFileExistAsRegularFile           (const char *filepath);
int         path_isFileExistAsDirectory             (const char *filepath);
t_error     path_createDirectory                    (const char *filepath);

t_error     path_withDirectoryAndName               (char *dest, 
                                                        size_t size, 
                                                        const char *directory, 
                                                        const char *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         loader_load                             (t_glist *glist, t_symbol *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     properties_loadBegin                    (void);
void        properties_loadClose                    (void);
t_error     properties_saveBegin                    (void);
void        properties_saveClose                    (void);
int         properties_getKey                       (const char *key, char *value, int size);
void        properties_setKey                       (const char *key, const char *value);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        preferences_load                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *leak_getMemoryChecked                  (size_t n, const char *f, int line);
void        *leak_getMemoryResizeChecked            (void *ptr,
                                                        size_t oldSize,
                                                        size_t newSize,
                                                        const char *f,
                                                        int line);

void        leak_freeMemoryChecked                  (void *ptr, const char *f, int line);


void        leak_initialize                         (void);
void        leak_release                            (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        font_setDefaultFontSize                 (int size);
t_fontsize  font_getDefaultFontSize                 (void);
t_fontsize  font_getNearestValidFontSize            (int size);
int         font_getHostFontSize                    (t_fontsize fontSize);
double      font_getHostFontWidth                   (t_fontsize fontSize);
double      font_getHostFontHeight                  (t_fontsize fontSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     logger_initialize                       (void);
void        logger_release                          (void);
int         logger_isRunning                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        sys_setSignalHandlers                   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void sys_closeSocket (int fd)
{
    #if PD_WINDOWS
        closesocket (fd);
    #else
        close (fd);
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "s_clock.h"
#include "s_time.h"
#include "s_atomic.h"
#include "s_ringbuffer.h"
#include "s_logger.h"
#include "s_MT.h"
#include "s_devices.h"
#include "s_apis.h"
#include "s_midi_apis.h"
#include "s_audio_apis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_system_h_
