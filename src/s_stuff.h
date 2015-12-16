/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Audio and MIDI I/O, and other scheduling and system stuff. */

/* NOTE: this file describes Pd implementation details which may change
in future releases.  The public (stable) API is in m_pd.h. */  

/* in s_path.c */

typedef struct _namelist    /* element in a linked list of stored strings */
{
    struct _namelist *nl_next;  /* next in list */
    char *nl_string;            /* the string */
} t_namelist;

t_namelist *namelist_append(t_namelist *listwas, const char *s, int allowdup);
t_namelist *namelist_append_files(t_namelist *listwas, const char *s);
void namelist_free(t_namelist *listwas);
char *namelist_get(t_namelist *namelist, int n);
void sys_setextrapath(const char *p);
extern int sys_usestdpath;
extern t_namelist *sys_externlist;
extern t_namelist *sys_searchpath;
extern t_namelist *sys_helppath;
int sys_open_absolute(const char *name, const char* ext,
    char *dirresult, char **nameresult, unsigned int size, int bin, int *fdp);
int sys_trytoopenone(const char *dir, const char *name, const char* ext,
    char *dirresult, char **nameresult, unsigned int size, int bin);
t_symbol *sys_decodedialog(t_symbol *s);

/* s_file.c */

void sys_loadpreferences( void);
void sys_savepreferences( void);
extern int sys_defeatrt;
extern t_symbol *sys_flags;

/* s_main.c */
extern int sys_debuglevel;
extern int sys_verbose;
extern int sys_noloadbang;
extern int sys_nogui;
extern char *sys_guicmd;

EXPORT int sys_nearestfontsize(int fontsize);
EXPORT int sys_hostfontsize(int fontsize);

extern int sys_defaultfont;
EXPORT t_symbol *sys_libdir;    /* library directory for auxilliary files */
extern t_symbol *sys_guidir;    /* directory holding pd_gui, u_pdsend, etc */

/* s_loader.c */

typedef int (*loader_t)(t_canvas *canvas, char *classname); /* callback type */
EXPORT int sys_load_lib(t_canvas *canvas, char *filename);
EXPORT void sys_register_loader(loader_t loader);

/* s_audio.c */

#define SENDDACS_NO 0           /* return values for sys_send_dacs() */
#define SENDDACS_YES 1 
#define SENDDACS_SLEPT 2

#define DEFDACBLKSIZE 64
extern int sys_schedblocksize;  /* audio block size for scheduler */
extern int sys_hipriority;      /* real-time flag, true if priority boosted */
EXPORT t_sample *sys_soundout;
EXPORT t_sample *sys_soundin;
extern int sys_inchannels;
extern int sys_outchannels;
extern int sys_advance_samples; /* scheduler advance in samples */
extern int sys_blocksize;       /* audio I/O block size in sample frames */
extern t_float sys_dacsr;
extern int sys_schedadvance;
extern int sys_sleepgrain;
EXPORT void sys_set_audio_settings(int naudioindev, int *audioindev,
    int nchindev, int *chindev,
    int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev,
    int srate, int advance, int callback, int blocksize);
/* the same as above, but reopens the audio subsystem if needed */
EXPORT void sys_set_audio_settings_reopen(int naudioindev, int *audioindev,
    int nchindev, int *chindev,
    int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev,
    int srate, int advance, int callback, int blocksize);
EXPORT void sys_reopen_audio( void);
EXPORT void sys_close_audio(void);
    /* return true if the interface prefers always being open (ala jack) : */
EXPORT int audio_shouldkeepopen( void);
EXPORT int audio_isopen( void);     /* true if audio interface is open */
EXPORT int sys_audiodevnametonumber(int output, const char *name);
EXPORT void sys_audiodevnumbertoname(int output, int devno, char *name,
    int namesize);

int sys_send_dacs(void);
void sys_reportidle(void);
void sys_set_priority(int higher);
void sys_audiobuf(int nbufs);
void sys_getmeters(t_sample *inmax, t_sample *outmax);
void sys_listdevs(void);
void sys_setblocksize(int n);

EXPORT void sys_get_audio_devs(char *indevlist, int *nindevs,
                          char *outdevlist, int *noutdevs, int *canmulti, int *cancallback, 
                          int maxndev, int devdescsize);
EXPORT void sys_get_audio_apis(char *buf);

/* s_midi.c */
#define MAXMIDIINDEV 16         /* max. number of input ports */
#define MAXMIDIOUTDEV 16        /* max. number of output ports */
extern int sys_midiapi;
extern int sys_nmidiin;
extern int sys_nmidiout;
extern int sys_midiindevlist[];
extern int sys_midioutdevlist[];

EXPORT void sys_open_midi(int nmidiin, int *midiinvec,
    int nmidiout, int *midioutvec, int enable);

EXPORT void sys_get_midi_apis(char *buf);
EXPORT void sys_get_midi_devs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, 
   int maxndev, int devdescsize);
EXPORT void sys_get_midi_params(int *pnmidiindev, int *pmidiindev,
    int *pnmidioutdev, int *pmidioutdev);
EXPORT int sys_mididevnametonumber(int output, const char *name);
EXPORT void sys_mididevnumbertoname(int output, int devno, char *name,
    int namesize);

EXPORT void sys_reopen_midi( void);
EXPORT void sys_close_midi( void);
EXPORT void sys_putmidimess(int portno, int a, int b, int c);
EXPORT void sys_putmidibyte(int portno, int a);
EXPORT void sys_poll_midi(void);
EXPORT void sys_setmiditimediff(double inbuftime, double outbuftime);
EXPORT void sys_midibytein(int portno, int byte);

    /* implemented in the system dependent MIDI code (s_midi_pm.c, etc. ) */
void midi_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int maxndev, int devdescsize);
void sys_do_open_midi(int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev);

#ifdef USEAPI_ALSA
EXPORT void sys_alsa_putmidimess(int portno, int a, int b, int c);
EXPORT void sys_alsa_putmidibyte(int portno, int a);
EXPORT void sys_alsa_poll_midi(void);
EXPORT void sys_alsa_setmiditimediff(double inbuftime, double outbuftime);
EXPORT void sys_alsa_midibytein(int portno, int byte);
EXPORT void sys_alsa_close_midi( void);


    /* implemented in the system dependent MIDI code (s_midi_pm.c, etc. ) */
void midi_alsa_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int maxndev, int devdescsize);
void sys_alsa_do_open_midi(int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev);
#endif

/* m_sched.c */
EXPORT void sys_log_error(int type);
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

EXPORT void sys_microsleep(int microsec);
EXPORT void sys_init_fdpoll(void);

EXPORT void sys_bail(int exitcode);
EXPORT int sys_pollgui(void);

EXTERN_STRUCT _socketreceiver;
#define t_socketreceiver struct _socketreceiver

typedef void (*t_socketnotifier)(void *x, int n);
typedef void (*t_socketreceivefn)(void *x, t_binbuf *b);

EXPORT t_socketreceiver *socketreceiver_new(void *owner,
    t_socketnotifier notifier, t_socketreceivefn socketreceivefn, int udp);
EXPORT void socketreceiver_read(t_socketreceiver *x, int fd);
EXPORT void sys_sockerror(char *s);
EXPORT void sys_closesocket(int fd);

typedef void (*t_fdpollfn)(void *ptr, int fd);
EXPORT void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr);
EXPORT void sys_rmpollfn(int fd);
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
EXPORT void sys_set_midi_api(int whichapi);
EXPORT void sys_set_audio_api(int whichapi);
EXPORT int sys_audioapi;
EXPORT void sys_set_audio_state(int onoff);

/* API dependent audio flags and settings */
void oss_set32bit( void);
void linux_alsa_devname(char *devname);

EXPORT void sys_get_audio_params(
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

EXPORT double sys_time;
EXPORT double sys_time_per_dsp_tick;
EXPORT int sys_externalschedlib;

EXPORT t_sample* get_sys_soundout(void ) ;
EXPORT t_sample* get_sys_soundin(void ) ;
EXPORT int* get_sys_main_advance(void ) ;
EXPORT double* get_sys_time_per_dsp_tick(void ) ;
EXPORT int* get_sys_schedblocksize(void ) ;
EXPORT double* get_sys_time(void ) ;
EXPORT t_float* get_sys_dacsr(void ) ;
EXPORT int* get_sys_sleepgrain(void ) ;
EXPORT int* get_sys_schedadvance(void ) ;

EXPORT void sys_clearhist(void );
EXPORT void sys_initmidiqueue(void );
EXPORT int sys_addhist(int phase);
EXPORT void sys_setmiditimediff(double inbuftime, double outbuftime);
EXPORT void sched_tick( void);
EXPORT void sys_pollmidiqueue(void );
EXPORT int sys_pollgui(void );
EXPORT void sys_setchsr(int chin, int chout, int sr);

EXPORT void inmidi_realtimein(int portno, int cmd);
EXPORT void inmidi_byte(int portno, int byte);
EXPORT void inmidi_sysex(int portno, int byte);
EXPORT void inmidi_noteon(int portno, int channel, int pitch, int velo);
EXPORT void inmidi_controlchange(int portno,
                                 int channel,
                                 int ctlnumber,
                                 int value);
EXPORT void inmidi_programchange(int portno, int channel, int value);
EXPORT void inmidi_pitchbend(int portno, int channel, int value);
EXPORT void inmidi_aftertouch(int portno, int channel, int value);
EXPORT void inmidi_polyaftertouch(int portno,
                                  int channel,
                                  int pitch,
                                  int value);
/* } jsarlo */
extern t_widgetbehavior text_widgetbehavior;
