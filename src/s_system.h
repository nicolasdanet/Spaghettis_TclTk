
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

#define DACS_NO                             0
#define DACS_YES                            1 
#define DACS_SLEPT                          2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCHEDULER_AUDIO_NONE                0
#define SCHEDULER_AUDIO_POLL                1 
#define SCHEDULER_AUDIO_CALLBACK            2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MAXIMUM_MIDI_IN                     16
#define MAXIMUM_MIDI_OUT                    16
#define MAXIMUM_AUDIO_IN                    4
#define MAXIMUM_AUDIO_OUT                   4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUDIO_DEFAULT_DEVICE                0
#define AUDIO_DEFAULT_BLOCK                 64
#define AUDIO_DEFAULT_SAMPLING              44100

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS
    #define AUDIO_DEFAULT_ADVANCE           80
#elif PD_APPLE
    #define AUDIO_DEFAULT_ADVANCE           5
#else
    #define AUDIO_DEFAULT_ADVANCE           25
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define API_NONE                            0
#define API_ALSA                            1
#define API_OSS                             2
#define API_MMIO                            3
#define API_PORTAUDIO                       4
#define API_JACK                            5
#define API_DUMMY                           9

//#define API_SGI                           6
//#define API_AUDIOUNIT                     7
//#define API_ESD                           8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if defined ( USEAPI_MMIO )
    #define API_DEFAULT                     API_MMIO
    #define API_DEFAULT_STRING              "MMIO"
    
#elif defined ( USEAPI_ALSA )
    #define API_DEFAULT                     API_ALSA
    #define API_DEFAULT_STRING              "ALSA"
    
#elif defined ( USEAPI_OSS )
    #define API_DEFAULT                     API_OSS
    #define API_DEFAULT_STRING              "OSS"
    
#elif defined ( USEAPI_PORTAUDIO )
    #define API_DEFAULT                     API_PORTAUDIO
    #define API_DEFAULT_STRING              "PortAudio"
    
#elif defined ( USEAPI_JACK )
    #define API_DEFAULT                     API_JACK
    #define API_DEFAULT_STRING              "JACK"
    
#elif defined ( USEAPI_DUMMY )
    #define API_DEFAULT                     API_DUMMY
    #define API_DEFAULT_STRING              "Dummy"
#else
    #error "Unknown Audio API"
#endif 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS
    #define PATHLIST_SEPARATOR              ';'
#else
    #define PATHLIST_SEPARATOR              ':'
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_pollfn)        (void *p, int fd);
typedef void (*t_notifyfn)      (void *x, int n);
typedef void (*t_receivefn)     (void *x, t_buffer *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef int  (*t_loader)        (t_canvas *canvas, char *classname);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _socketreceiver {
    void        *sr_owner;
    char        *sr_inBuffer;
    int         sr_inHead;
    int         sr_inTail;
    int         sr_isUdp;
    t_notifyfn  sr_fnNotify;
    t_receivefn sr_fnReceive;
    } t_socketreceiver;

typedef struct _pathlist {
    struct _pathlist    *pl_next;
    char                *pl_string;
    } t_pathlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef int t_fontsize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pathlist  *pathlist_newAppend                 (t_pathlist *x, const char *s);
t_pathlist  *pathlist_newAppendFiles            (t_pathlist *x, const char *s, char delimiter);
char        *pathlist_getFileAtIndex            (t_pathlist *x, int n);
char        *pathlist_getFile                   (t_pathlist *x);
t_pathlist  *pathlist_getNext                   (t_pathlist *x);

void        pathlist_free                       (t_pathlist *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        font_withHostMeasured               (void *dummy, t_symbol *s, int argc, t_atom *argv);
t_fontsize  font_getNearestValidFontSize        (int size);
int         font_getHostFontSize                (t_fontsize fontSize);
int         font_getHostFontWidth               (t_fontsize fontSize);
int         font_getHostFontHeight              (t_fontsize fontSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         main_entry                          (int argc, char **argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        clock_setUnitAsSamples              (t_clock *x, double samples);
void        clock_setUnitAsMilliseconds         (t_clock *x, double ms);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

double      scheduler_getSystime                (void);
double      scheduler_getSystimeAfter           (double ms);
double      scheduler_getMillisecondsSince      (double systime);
double      scheduler_getUnitsSince             (double systime, double unit, int isSamples);
void        scheduler_setAudioMode              (int flag);
void        scheduler_needToRestart             (void);
void        scheduler_needToExit                (void);
void        scheduler_lock                      (void);
void        scheduler_unlock                    (void);
void        scheduler_audioCallback             (void);
int         scheduler_main                      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

double      sys_getRealTime                     (void);
void        sys_pollSocketsBlocking             (int microseconds);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        preferences_load                    (void);
void        preferences_save                    (void *dummy);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_setsignalhandlers                  (void);
int  sys_startgui                           (const char *guipath);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void post_atoms                             (int argc, t_atom *argv);
void open_via_helppath                      (const char *name, const char *dir);

void sys_bashfilename                       (char *from, char *to);
void sys_unbashfilename                     (char *from, char *to);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_set_searchpath                     (void);
void sys_set_extrapath                      (void);
void sys_set_startup                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_setextrapath                (const char *p);
int         sys_open_absolute               (const char *name,
                                                const char* ext,
                                                char *dirresult,
                                                char **nameresult,
                                                unsigned int size,
                                                int bin,
                                                int *fdp);
                            
int         sys_trytoopenone                (const char *dir,
                                                const char *name,
                                                const char* ext,
                                                char *dirresult,
                                                char **nameresult,
                                                unsigned int size,
                                                int bin);
                            
t_symbol    *sys_decodedialog               (t_symbol *s);
int         sys_load_lib                    (t_canvas *canvas, char *name);
void        sys_register_loader             (t_loader loader);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_set_audio_settings          (int naudioindev,
                                                int *audioindev,
                                                int nchindev,
                                                int *chindev,
                                                int naudiooutdev,
                                                int *audiooutdev,
                                                int nchoutdev,
                                                int *choutdev,
                                                int srate,
                                                int advance,
                                                int callback,
                                                int blocksize);
                                            
void        sys_set_audio_settings_reopen   (int naudioindev,
                                                int *audioindev,
                                                int nchindev,
                                                int *chindev,
                                                int naudiooutdev,
                                                int *audiooutdev,
                                                int nchoutdev,
                                                int *choutdev,
                                                int srate,
                                                int advance,
                                                int callback,
                                                int blocksize);
                                                
void        sys_reopen_audio                (void);
void        sys_close_audio                 (void);

int         audio_shouldkeepopen            (void);
int         audio_isopen                    (void);
int         sys_audiodevnametonumber        (int output, const char *name);
void        sys_audiodevnumbertoname        (int output, int devno, char *name, int namesize);

int         sys_send_dacs                   (void);
void        sys_set_priority                (int higher);
void        sys_audiobuf                    (int nbufs);
void        sys_getmeters                   (t_sample *inmax, t_sample *outmax);
void        sys_listdevs                    (void);
void        sys_setblocksize                (int n);

void        sys_get_audio_devs              (char *indevlist,
                                                int *nindevs,
                                                char *outdevlist,
                                                int *noutdevs,
                                                int *canmulti,
                                                int *cancallback, 
                                                int maxndev,
                                                int devdescsize);
                                                
void        sys_get_audio_apis              (char *buf);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_open_midi                   (int nmidiin,
                                                int *midiinvec,
                                                int nmidiout,
                                                int *midioutvec,
                                                int enable);

void        sys_get_midi_apis               (char *buf);
void        sys_get_midi_devs               (char *indevlist,
                                                int *nindevs,
                                                char *outdevlist,
                                                int *noutdevs, 
                                                int maxndev,
                                                int devdescsize);
                                                
void        sys_get_midi_params             (int *pnmidiindev,
                                                int *pmidiindev,
                                                int *pnmidioutdev,
                                                int *pmidioutdev);
                                                
int         sys_mididevnametonumber         (int output, const char *name);
void        sys_mididevnumbertoname         (int output, int devno, char *name, int namesize);

void        sys_reopen_midi                 (void);
void        sys_close_midi                  (void);
void        sys_putmidimess                 (int portno, int a, int b, int c);
void        sys_putmidibyte                 (int portno, int a);
void        sys_poll_midi                   (void);
void        sys_setmiditimediff             (double inbuftime, double outbuftime);
void        sys_midibytein                  (int portno, int byte);

void        midi_getdevs                    (char *indevlist,
                                                int *nindevs,
                                                char *outdevlist,
                                                int *noutdevs,
                                                int maxndev,
                                                int devdescsize);
                                                
void        sys_do_open_midi                (int nmidiindev,
                                                int *midiindev,
                                                int nmidioutdev,
                                                int *midioutdev);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef USEAPI_ALSA

void sys_alsa_putmidimess       (int portno, int a, int b, int c);
void sys_alsa_putmidibyte       (int portno, int a);
void sys_alsa_poll_midi         (void);
void sys_alsa_setmiditimediff   (double inbuftime, double outbuftime);
void sys_alsa_midibytein        (int portno, int byte);
void sys_alsa_close_midi        (void);

void midi_alsa_getdevs          (char *indevlist,
                                    int *nindevs,
                                    char *outdevlist,
                                    int *noutdevs,
                                    int maxndev,
                                    int devdescsize);
                                    
void sys_alsa_do_open_midi      (int nmidiindev,
                                    int *midiindev,
                                    int nmidioutdev,
                                    int *midioutdev);
    
#endif // USEAPI_ALSA

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_tick                   (void);
void sys_pollmidiqueue          (void);
void sys_initmidiqueue          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_init_fdpoll            (void);
void sys_bail                   (int exitcode);
int  sys_pollgui                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_setmiditimediff        (double inbuftime, double outbuftime);
int  sys_pollgui                (void);
void sys_setchsr                (int chin, int chout, int sr);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_socketreceiver *socketreceiver_new (void *owner, t_notifyfn notifier, t_receivefn fn, int udp);
void             socketreceiver_read (t_socketreceiver *x, int fd);
void             sys_sockerror       (char *s);
void             sys_closesocket     (int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void sys_addpollfn      (int fd, t_pollfn fn, void *ptr);
void sys_rmpollfn       (int fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if defined ( USEAPI_OSS ) || defined ( USEAPI_ALSA )

void sys_setalarm       (int microsec);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_audiocallback)(void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int  pa_open_audio           (int inchans,
                                int outchans,
                                int rate,
                                t_sample *soundin,
                                t_sample *soundout,
                                int framesperbuf,
                                int nbuffers,
                                int indeviceno,
                                int outdeviceno,
                                t_audiocallback callback);
                                
void pa_close_audio         (void);
int  pa_send_dacs           (void);
void pa_listdevs            (void);
void pa_getdevs             (char *indevlist,
                                int *nindevs,
                                char *outdevlist,
                                int *noutdevs,
                                int *canmulti, 
                                int maxndev,
                                int devdescsize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int oss_open_audio          (int naudioindev,
                                int *audioindev,
                                int nchindev,
                                int *chindev,
                                int naudiooutdev,
                                int *audiooutdev,
                                int nchoutdev,
                                int *choutdev,
                                int rate,
                                int blocksize);
                                
void oss_close_audio        (void);
int  oss_send_dacs          (void);
void oss_getdevs            (char *indevlist,
                                int *nindevs,
                                char *outdevlist,
                                int *noutdevs,
                                int *canmulti, 
                                int maxndev,
                                int devdescsize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int  alsa_open_audio        (int naudioindev,
                                int *audioindev,
                                int nchindev,
                                int *chindev,
                                int naudiooutdev,
                                int *audiooutdev,
                                int nchoutdev,
                                int *choutdev,
                                int rate,
                                int blocksize);
                                
void alsa_close_audio       (void);
int  alsa_send_dacs         (void);
void alsa_getdevs           (char *indevlist,
                                int *nindevs,
                                char *outdevlist,
                                int *noutdevs,
                                int *canmulti, 
                                int maxndev,
                                int devdescsize);
void alsa_adddev            (char *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int  jack_open_audio        (int wantinchans,
                                int wantoutchans,
                                int srate,
                                t_audiocallback callback);
                                
void jack_close_audio       (void);
int  jack_send_dacs         (void);
void jack_getdevs           (char *indevlist,
                                int *nindevs,
                                char *outdevlist,
                                int *noutdevs,
                                int *canmulti,
                                int maxndev,
                                int devdescsize);
                                
void jack_listdevs          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int  mmio_open_audio        (int naudioindev,
                                int *audioindev,
                                int nchindev,
                                int *chindev,
                                int naudiooutdev,
                                int *audiooutdev,
                                int nchoutdev,
                                int *choutdev,
                                int rate,
                                int blocksize);
                                
void mmio_close_audio       (void);
int  mmio_send_dacs         (void);
void mmio_getdevs           (char *indevlist,
                                int *nindevs,
                                char *outdevlist,
                                int *noutdevs,
                                int *canmulti,
                                int maxndev,
                                int devdescsize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int  dummy_open_audio       (int nin, int nout, int sr);
void dummy_close_audio      (void);
int  dummy_send_dacs        (void);
void dummy_getdevs          (char *indevlist, 
                                int *nindevs,
                                char *outdevlist,
                                int *noutdevs,
                                int *canmulti,
                                int maxndev,
                                int devdescsize);
void dummy_listdevs         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_listmididevs       (void);
void sys_set_midi_api       (int whichapi);
void sys_set_audio_api      (int whichapi);
void sys_set_audio_state    (int onoff);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_get_audio_params   (int *pnaudioindev,
                                int *paudioindev,
                                int *chindev,
                                int *pnaudiooutdev,
                                int *paudiooutdev,
                                int *choutdev,
                                int *prate,
                                int *padvance,
                                int *callback,
                                int *blocksize);
                                
void sys_save_audio_params  (int naudioindev,
                                int *audioindev,
                                int *chindev,
                                int naudiooutdev,
                                int *audiooutdev,
                                int *choutdev,
                                int rate,
                                int advance,
                                int callback,
                                int blocksize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void inmidi_realtimein          (int portno, int cmd);
void inmidi_byte                (int portno, int byte);
void inmidi_sysex               (int portno, int byte);
void inmidi_noteon              (int portno, int channel, int pitch, int velo);
void inmidi_controlchange       (int portno, int channel, int ctlnumber, int value);
void inmidi_programchange       (int portno, int channel, int value);
void inmidi_pitchbend           (int portno, int channel, int value);
void inmidi_aftertouch          (int portno, int channel, int value);
void inmidi_polyaftertouch      (int portno, int channel, int pitch, int value);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_system_h_
