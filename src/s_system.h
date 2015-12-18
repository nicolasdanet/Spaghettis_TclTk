
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

#define MAXIMUM_MIDI_IN         16
#define MAXIMUM_MIDI_OUT        16

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SEND_DACS_NO            0
#define SEND_DACS_YES           1 
#define SEND_DACS_SLEPT         2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DEFAULT_BLOCKSIZE       64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _namelist {
    struct _namelist    *nl_next;
    char                *nl_string;
    } t_namelist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_namelist  *namelist_append                (t_namelist *namelist, const char *s, int allowdup);
t_namelist  *namelist_append_files          (t_namelist *namelist, const char *s);
char        *namelist_get                   (t_namelist *namelist, int n);
void        namelist_free                   (t_namelist *namelist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef int (*loader_t)(t_canvas *canvas, char *classname);

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
void        sys_loadpreferences             (void);
void        sys_savepreferences             (void);
int         sys_nearestfontsize             (int fontsize);
int         sys_hostfontsize                (int fontsize);
int         sys_load_lib                    (t_canvas *canvas, char *filename);
void        sys_register_loader             (loader_t loader);

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
void        sys_reportidle                  (void);
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

void sys_alsa_putmidimess(int portno, int a, int b, int c);
void sys_alsa_putmidibyte(int portno, int a);
void sys_alsa_poll_midi(void);
void sys_alsa_setmiditimediff(double inbuftime, double outbuftime);
void sys_alsa_midibytein(int portno, int byte);
void sys_alsa_close_midi(void);


    /* implemented in the system dependent MIDI code (s_midi_pm.c, etc. ) */
void midi_alsa_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int maxndev, int devdescsize);
void sys_alsa_do_open_midi(int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev);
    
#endif // USEAPI_ALSA

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* m_sched.c */
void sys_log_error(int type);
#define ERR_NOTHING 0
#define ERR_ADCSLEPT 1
#define ERR_DACSLEPT 2
#define ERR_RESYNC 3
#define ERR_DATALATE 4

#define SCHED_AUDIO_NONE 0
#define SCHED_AUDIO_POLL 1 
#define SCHED_AUDIO_CALLBACK 2
void sched_set_using_audio(int flag);

/* s_inter.c */

void sys_microsleep(int microsec);
void sys_init_fdpoll(void);

void sys_bail(int exitcode);
int sys_pollgui(void);

PD_STRUCT _socketreceiver;
#define t_socketreceiver struct _socketreceiver

typedef void (*t_socketnotifier)(void *x, int n);
typedef void (*t_socketreceivefn)(void *x, t_binbuf *b);

t_socketreceiver *socketreceiver_new(void *owner,
    t_socketnotifier notifier, t_socketreceivefn socketreceivefn, int udp);
void socketreceiver_read(t_socketreceiver *x, int fd);
void sys_sockerror(char *s);
void sys_closesocket(int fd);

typedef void (*t_fdpollfn)(void *ptr, int fd);
void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr);
void sys_rmpollfn(int fd);
#if defined(USEAPI_OSS) || defined(USEAPI_ALSA)
void sys_setalarm(int microsec);
#endif

#define API_NONE 0
#define API_ALSA 1
#define API_OSS 2
#define API_MMIO 3
#define API_PORTAUDIO 4
#define API_JACK 5
#define API_SGI 6           /* gone */
#define API_AUDIOUNIT 7
#define API_ESD 8           /* no idea what this was, probably gone now */
#define API_DUMMY 9

    /* figure out which API should be the default.  The one we judge most
    likely to offer a working device takes precedence so that if you
    start up Pd for the first time there's a reasonable chance you'll have
    sound.  (You'd think portaudio would be best but it seems to default
    to jack on linux, and and on Windows we only use it for ASIO). 
    If nobody shows up, define DUMMY and make it the default.*/
#if defined(USEAPI_MMIO)
# define API_DEFAULT API_MMIO
# define API_DEFSTRING "MMIO"
#elif defined(USEAPI_ALSA)
# define API_DEFAULT API_ALSA
# define API_DEFSTRING "ALSA"
#elif defined(USEAPI_OSS)
# define API_DEFAULT API_OSS
# define API_DEFSTRING "OSS"
#elif defined(USEAPI_AUDIOUNIT)
# define API_DEFAULT API_AUDIOUNIT
# define API_DEFSTRING "AudioUnit"
#elif defined(USEAPI_ESD)
# define API_DEFAULT API_ESD
# define API_DEFSTRING "ESD (?)"
#elif defined(USEAPI_PORTAUDIO)
# define API_DEFAULT API_PORTAUDIO
# define API_DEFSTRING "portaudio"
#elif defined(USEAPI_JACK)
# define API_DEFAULT API_JACK
# define API_DEFSTRING "Jack audio connection kit"
#else 
# ifndef USEAPI_DUMMY   /* we need at least one so bring in the dummy */
# define USEAPI_DUMMY
# endif /* USEAPI_DUMMY */
# define API_DEFAULT API_DUMMY
# define API_DEFSTRING "dummy audio"
#endif 

#define DEFAULTAUDIODEV 0

#define MAXAUDIOINDEV 4
#define MAXAUDIOOUTDEV 4

#define DEFMIDIDEV 0

#define DEFAULTSRATE 44100
#ifdef _WIN32
#define DEFAULTADVANCE 80
#else
#ifdef __APPLE__
#define DEFAULTADVANCE 5    /* this is in addition to their own delay */
#else
#define DEFAULTADVANCE 25
#endif
#endif

typedef void (*t_audiocallback)(void);

int pa_open_audio(int inchans, int outchans, int rate, t_sample *soundin,
    t_sample *soundout, int framesperbuf, int nbuffers,
    int indeviceno, int outdeviceno, t_audiocallback callback);
void pa_close_audio(void);
int pa_send_dacs(void);
void sys_reportidle(void);
void pa_listdevs(void);
void pa_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, 
        int maxndev, int devdescsize);

int oss_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate, int blocksize);
void oss_close_audio(void);
int oss_send_dacs(void);
void oss_reportidle(void);
void oss_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, 
        int maxndev, int devdescsize);

int alsa_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate, int blocksize);
void alsa_close_audio(void);
int alsa_send_dacs(void);
void alsa_reportidle(void);
void alsa_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, 
        int maxndev, int devdescsize);

int jack_open_audio(int wantinchans, int wantoutchans, int srate,
    t_audiocallback callback);
void jack_close_audio(void);
int jack_send_dacs(void);
void jack_reportidle(void);
void jack_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, 
        int maxndev, int devdescsize);
void jack_listdevs(void);

int mmio_open_audio(int naudioindev, int *audioindev,
    int nchindev, int *chindev, int naudiooutdev, int *audiooutdev,
    int nchoutdev, int *choutdev, int rate, int blocksize);
void mmio_close_audio( void);
void mmio_reportidle(void);
int mmio_send_dacs(void);
void mmio_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, 
        int maxndev, int devdescsize);

int audiounit_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate);
void audiounit_close_audio(void);
int audiounit_send_dacs(void);
void audiounit_listdevs(void);
void audiounit_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, 
        int maxndev, int devdescsize);

int esd_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate);
void esd_close_audio(void);
int esd_send_dacs(void);
void esd_listdevs(void);
void esd_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti, 
        int maxndev, int devdescsize);

int dummy_open_audio(int nin, int nout, int sr);
int dummy_close_audio( void);
int dummy_send_dacs( void);
void dummy_getdevs(char *indevlist, int *nindevs, char *outdevlist,
    int *noutdevs, int *canmulti, int maxndev, int devdescsize);
void dummy_listdevs( void);

void sys_listmididevs(void);
void sys_set_midi_api(int whichapi);
void sys_set_audio_api(int whichapi);
int sys_audioapi;
void sys_set_audio_state(int onoff);

/* API dependent audio flags and settings */
void oss_set32bit( void);
void linux_alsa_devname(char *devname);

void sys_get_audio_params(
    int *pnaudioindev, int *paudioindev, int *chindev,
    int *pnaudiooutdev, int *paudiooutdev, int *choutdev,
    int *prate, int *padvance, int *callback, int *blocksize);
void sys_save_audio_params(
    int naudioindev, int *audioindev, int *chindev,
    int naudiooutdev, int *audiooutdev, int *choutdev,
    int rate, int advance, int callback, int blocksize);

/* s_file.c */

typedef void (*t_printhook)(const char *s);
extern t_printhook sys_printhook;  /* set this to override printing */
extern int sys_printtostderr;

/* jsarlo { */

double sys_time;
double sys_time_per_dsp_tick;
int sys_externalschedlib;

t_sample* get_sys_soundout(void ) ;
t_sample* get_sys_soundin(void ) ;
int* get_sys_main_advance(void ) ;
double* get_sys_time_per_dsp_tick(void ) ;
int* get_sys_schedblocksize(void ) ;
double* get_sys_time(void ) ;
t_float* get_sys_dacsr(void ) ;
int* get_sys_sleepgrain(void ) ;
int* get_sys_schedadvance(void ) ;

void sys_clearhist(void );
void sys_initmidiqueue(void );
int sys_addhist(int phase);
void sys_setmiditimediff(double inbuftime, double outbuftime);
void sched_tick( void);
void sys_pollmidiqueue(void );
int sys_pollgui(void );
void sys_setchsr(int chin, int chout, int sr);

void inmidi_realtimein(int portno, int cmd);
void inmidi_byte(int portno, int byte);
void inmidi_sysex(int portno, int byte);
void inmidi_noteon(int portno, int channel, int pitch, int velo);
void inmidi_controlchange(int portno,
                                 int channel,
                                 int ctlnumber,
                                 int value);
void inmidi_programchange(int portno, int channel, int value);
void inmidi_pitchbend(int portno, int channel, int value);
void inmidi_aftertouch(int portno, int channel, int value);
void inmidi_polyaftertouch(int portno,
                                  int channel,
                                  int pitch,
                                  int value);
/* } jsarlo */
extern t_widgetbehavior text_widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_system_h_
