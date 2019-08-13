
/* Copyright (c) 1997-2019 Miller Puckette and others. */

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

#define AUDIO_DEFAULT_SAMPLERATE    44100

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

t_systime   scheduler_getLogicalTime                (void);
t_systime   scheduler_getLogicalTimeAfter           (double ms);
double      scheduler_getMillisecondsSince          (t_systime systime);    
double      scheduler_getUnitsSince                 (t_systime systime, double unit, int isSamples);
void        scheduler_needToExit                    (void);
void        scheduler_needToExitWithError           (void);
int         scheduler_isExiting                     (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     privilege_start                         (void);
t_error     privilege_drop                          (void);
t_error     privilege_restore                       (void);

t_error     privilege_relinquish                    (void);     /* Error MUST lead to stop execution. */
void        privilege_check                         (void);

t_error     priority_setPolicy                      (pthread_t thread);

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

int         monitor_blocking                        (double milliseconds);
int         monitor_nonBlocking                     (void);
void        monitor_addPoller                       (int fd, t_pollfn fn, void *ptr);
void        monitor_removePoller                    (int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        gui_jobAdd                              (void *owner, t_glist *glist, t_drawfn f);
void        gui_jobRemove                           (void *owner);
int         gui_jobFlush                            (void);
void        gui_jobClear                            (void);

void        gui_vAdd                                (const char *format, ...);
void        gui_add                                 (const char *s);
void        gui_flush                               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     interface_start                         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        midi_poll                               (void);
void        midi_receive                            (int port, int status, int a, int b);
void        midi_send                               (int port, int status, int a, int b);

void        midi_receiveSysex                       (int port, uint8_t byte);
void        midi_sendSysex                          (int port, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         audio_poll                              (void);
t_error     audio_stop                              (void);
t_error     audio_start                             (void);

t_float     audio_getSampleRate                     (void);
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

t_error     file_lock                               (int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist  *recentfiles_get                        (void);

void        recentfiles_add                         (t_symbol *name, t_symbol *directory, int check);
void        recentfiles_appendPath                  (const char *filepath);
void        recentfiles_clear                       (void);
void        recentfiles_update                      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist  *searchpath_getRoots                    (void);
t_pathlist  *searchpath_getExtended                 (void);

void        searchpath_extendedMatchedAtIndex       (int n);

void        searchpath_appendRoot                   (const char *filepath);
void        searchpath_setRootsEncoded              (int argc, t_atom *argv);
t_error     searchpath_scan                         (void);
void        searchpath_report                       (void);
int         searchpath_isExternalAvailable          (t_symbol *s);
int         searchpath_isAbstractionAvailable       (t_symbol *s);
int         searchpath_hasDuplicates                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int         path_isFileExist                        (const char *filepath);
int         path_isFileExistAsRegularFile           (const char *filepath);
int         path_isFileExistAsDirectory             (const char *filepath);
int         path_isValid                            (const char *filepath);
t_error     path_createDirectory                    (const char *filepath);
t_error     path_createDirectoryIfNeeded            (const char *filepath);

t_error     path_withDirectoryAndName               (char *dest, 
                                                        size_t size, 
                                                        const char *directory, 
                                                        const char *name);

t_symbol    *path_withDirectoryAndNameAsSymbol      (t_symbol *directory, t_symbol *name);

t_error     path_toDirectoryAndNameAsSymbol         (const char *filepath,
                                                        t_symbol **directory,
                                                        t_symbol **name);

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
void        preferences_save                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *memory_get                             (size_t n);
void        *memory_getResize                       (void *ptr, size_t oldSize, size_t newSize);

void        memory_free                             (void *ptr);

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

int         font_getDefaultSize                     (void);
void        font_setDefaultSize                     (int n);
int         font_getValidSize                       (int n);
double      font_getWidth                           (int fontSize);
double      font_getHeight                          (int fontSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     logger_initialize                       (void);
void        logger_release                          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        sys_setSignalHandlers                   (void);
int         sys_isMainThread                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void sys_closeSocket (int fd)
{
    close (fd);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "system/s_clock.h"
#include "system/s_time.h"
#include "system/s_atomic.h"
#include "system/s_ringbuffer.h"
#include "system/s_logger.h"
#include "system/s_MT.h"
#include "system/s_devices.h"
#include "system/s_deviceslist.h"
#include "system/s_API.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_system_h_
