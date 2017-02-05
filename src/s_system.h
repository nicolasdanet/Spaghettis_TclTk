
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_system_h_
#define __s_system_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DACS_NO                                 0
#define DACS_YES                                1 
#define DACS_SLEPT                              2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INTERNAL_BLOCKSIZE                      64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCHEDULER_AUDIO_NONE                    0
#define SCHEDULER_AUDIO_POLL                    1 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MAXIMUM_MIDI_IN                         8
#define MAXIMUM_MIDI_OUT                        8
#define MAXIMUM_AUDIO_IN                        4
#define MAXIMUM_AUDIO_OUT                       4
#define MAXIMUM_CHANNELS_IN                     32
#define MAXIMUM_CHANNELS_OUT                    32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MAXIMUM_DEVICES                         16
#define MAXIMUM_DESCRIPTION                     128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUDIO_DEFAULT_BLOCKSIZE                 64
#define AUDIO_DEFAULT_SAMPLERATE                44100

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* LCM (32000, 44100, 48000, 88200, 96000). */
    
#define SYSTIME_PER_MILLISECOND                 (32.0 * 441.0)
#define SYSTIME_PER_SECOND                      (SYSTIME_PER_MILLISECOND * 1000.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_pollfn)        (void *p, int fd);
typedef void (*t_notifyfn)      (void *owner, int fd);
typedef void (*t_receivefn)     (void *owner, t_buffer *b);
typedef void (*t_clockfn)       (void *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _clock {
    t_systime           c_systime;      /* Negative for unset clocks. */
    double              c_unit;         /* A positive value is in ticks, negative for number of samples. */
    t_clockfn           c_fn;
    void                *c_owner;
    struct _clock       *c_next;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _receiver {
    void                *r_owner;
    t_buffer            *r_message;
    char                *r_inRaw;
    int                 r_inHead;
    int                 r_inTail;
    int                 r_fd;
    int                 r_isUdp;
    int                 r_isBinary;
    int                 r_isClosed;
    t_notifyfn          r_fnNotify;
    t_receivefn         r_fnReceive;
    } t_receiver;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

int         main_entry                              (int argc, char **argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     scheduler_main                          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Usable in DSP perform. */

t_systime   scheduler_getLogicalTime                (void);
t_systime   scheduler_getLogicalTimeAfter           (double ms);
double      scheduler_getMillisecondsSince          (t_systime systime);    
double      scheduler_getUnitsSince                 (t_systime systime, double unit, int isSamples);
void        scheduler_setAudioMode                  (int flag);
void        scheduler_needToExit                    (void);
void        scheduler_needToExitWithError           (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     priority_privilegeStart                 (void);
t_error     priority_privilegeDrop                  (void);
t_error     priority_privilegeRestore               (void);
t_error     priority_privilegeRelinquish            (void);

t_error     priority_setPolicy                      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     clock_setUnitParsed                     (t_clock *x, t_float f, t_symbol *unitName);
void        clock_setUnitAsSamples                  (t_clock *x, double samples);
void        clock_setUnitAsMilliseconds             (t_clock *x, double ms);
t_error     clock_parseUnit                         (t_float f, 
                                                        t_symbol *unitName,
                                                        t_float *result,
                                                        int *isSamples);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_setSignalHandlers                   (void);
double      sys_getRealTimeInSeconds                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

int         monitor_blocking                        (int microseconds);
int         monitor_nonBlocking                     (void);
void        monitor_addPoller                       (int fd, t_pollfn fn, void *ptr);
void        monitor_removePoller                    (int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        defer_addJob                            (void *owner, t_glist *glist, t_drawfn f);
void        defer_removeJob                         (void *owner);
int         defer_flushJobs                         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         sys_guiPollOrFlush                      (void);
void        sys_guiFlush                            (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        interface_quit                          (void *dummy);
void        interface_watchdog                      (void *dummy);
t_error     interface_start                         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

FILE        *file_openWrite                         (const char *filepath);
int         file_openRaw                            (const char *filepath, int oflag);

int         file_openConsideringSearchPath          (const char *directory, 
                                                        const char *name,
                                                        const char *extension,
                                                        char *directoryResult,
                                                        char **nameResult,
                                                        size_t size);

void        file_openHelp                           (const char *directory, const char *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        path_slashToBackslashIfNecessary        (char *dest, char *src);
void        path_backslashToSlashIfNecessary        (char *dest, char *src);
int         path_isFileExist                        (const char *filepath);
int         path_isFileExistAsRegularFile           (const char *filepath);
t_error     path_withDirectoryAndName               (char *dest, 
                                                        size_t size, 
                                                        const char *directory, 
                                                        const char *name,
                                                        int expandEnvironment);
                                                        
t_error     path_expandEnvironment                  (char *dest, size_t size, const char *src);
void        path_setSearchPath                      (void *dummy, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         loader_load                             (t_glist *canvas, char *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        preferences_load                        (void);
void        preferences_save                        (void *dummy);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        leak_initialize                         (void);
void        leak_release                            (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        *sys_getMemoryChecked                   (size_t n, const char *f, int line);
void        *sys_getMemoryResizeChecked             (void *ptr,
                                                        size_t oldSize,
                                                        size_t newSize,
                                                        const char *f,
                                                        int line);

void        sys_freeMemoryChecked                   (void *ptr, const char *f, int line);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        font_withHostMeasured                   (void *dummy, t_symbol *s, int argc, t_atom *argv);
void        font_setDefaultFontSize                 (int size);
t_fontsize  font_getDefaultFontSize                 (void);
t_fontsize  font_getNearestValidFontSize            (int size);
int         font_getHostFontSize                    (t_fontsize fontSize);
double      font_getHostFontWidth                   (t_fontsize fontSize);
double      font_getHostFontHeight                  (t_fontsize fontSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     logger_initialize                       (void);
void        logger_release                          (void);
int         logger_isRunning                        (void);

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

void        midi_requireDialog                      (void *dummy);
void        midi_fromDialog                         (void *dummy, t_symbol *s, int argc, t_atom *argv);
int         midi_numberWithName                     (int isOutput, const char *name);
t_error     midi_numberToName                       (int isOutput, int k, char *dest, size_t size);
void        midi_open                               (void);
void        midi_close                              (void);

void        midi_getDevices                         (int *numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int *numberOfDevicesOut,
                                                        int *devicesOut);

void        midi_setDefaultDevices                  (int numberOfDevicesIn, 
                                                        int *devicesIn, 
                                                        int numberOfDevicesOut, 
                                                        int *devicesOut);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        inmidi_noteOn                           (int port, int channel, int pitch, int velocity);
void        inmidi_controlChange                    (int port, int channel, int control, int value);
void        inmidi_programChange                    (int port, int channel, int value);
void        inmidi_pitchBend                        (int port, int channel, int value);
void        inmidi_afterTouch                       (int port, int channel, int value);
void        inmidi_polyPressure                     (int port, int channel, int pitch, int value);
void        inmidi_byte                             (int port, int byte);
void        inmidi_sysex                            (int port, int byte);
void        inmidi_realTime                         (int port, int byte);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        outmidi_noteOn                          (int channel, int pitch, int velocity);
void        outmidi_controlChange                   (int channel, int control, int value);
void        outmidi_programChange                   (int channel, int value);
void        outmidi_pitchBend                       (int channel, int value);
void        outmidi_afterTouch                      (int channel, int value);
void        outmidi_polyPressure                    (int channel, int pitch, int value);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     audio_initialize                        (void);
void        audio_release                           (void);
int         audio_pollDSP                           (void);
t_error     audio_stopDSP                           (void);
t_error     audio_startDSP                          (void);

void        audio_initializeMemory                  (int usedChannelsIn, int usedChannelsOut);

void        audio_shrinkChannelsIn                  (int numberOfChannelsIn);
void        audio_shrinkChannelsOut                 (int numberOfChannelsOut);
void        audio_setSampleRate                     (t_float sampleRate);
void        audio_setBlockSize                      (int blockSize);
int         audio_getChannelsIn                     (void);
int         audio_getChannelsOut                    (void);
t_float     audio_getSampleRate                     (void);

void        audio_requireDialog                     (void *dummy);
void        audio_fromDialog                        (void *dummy, t_symbol *s, int argc, t_atom *argv);
int         audio_numberWithName                    (int isOutput, const char *name);
t_error     audio_numberToName                      (int isOutput, int k, char *dest, size_t size);
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

void        audio_setDevicesWithDefault             (int numberOfDevicesIn,
                                                        int *devicesIn,
                                                        int *channelsIn,
                                                        int numberOfDevicesOut,
                                                        int *devicesOut,
                                                        int *channelsOut,
                                                        int sampleRate,
                                                        int blockSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "s_midi_apis.h"
#include "s_audio_apis.h"
#include "s_logger_apis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_system_h_
