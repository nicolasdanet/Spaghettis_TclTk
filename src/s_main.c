
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_MSVC
    #define snprintf sprintf_s
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pdinstance     *pd_this;
extern t_sample         *sys_soundin;
extern t_sample         *sys_soundout;
extern t_namelist       *sys_helppath;
extern t_namelist       *sys_externlist;
extern t_namelist       *sys_searchpath;

extern t_float          sys_dacsr;
extern int              sys_usestdpath;
extern int              sys_schedadvance;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

char        main_compileTime[] = __TIME__;                              /* Shared. */
char        main_compileDate[] = __DATE__;                              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int         main_noLoadbang;                                            /* Shared. */
int         main_noGUI;                                                 /* Shared. */
int         main_noSleep;                                               /* Shared. */
int         main_portNumber;                                            /* Shared. */
int         main_highPriority = -1;                                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

char        *main_commandToLaunchGUI;                                   /* Shared. */
t_symbol    *main_libDirectory;                                         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_namelist   *main_openList;                                     /* Shared. */
static t_namelist   *main_messageList;                                  /* Shared. */

static int          main_batch;                                         /* Shared. */
static int          main_version;                                       /* Shared. */
static int          main_devices;                                       /* Shared. */
static int          main_advance;                                       /* Shared. */
static int          main_callback;                                      /* Shared. */
static int          main_blockSize;                                     /* Shared. */
static int          main_sampleRate;                                    /* Shared. */

static int          main_numberOfMidiIn         = -1;                   /* Shared. */
static int          main_numberOfMidiOut        = -1;                   /* Shared. */
static int          main_numberOfAudioIn        = -1;                   /* Shared. */
static int          main_numberOfAudioOut       = -1;                   /* Shared. */
static int          main_numberOfChannelIn      = -1;                   /* Shared. */
static int          main_numberOfChannelOut     = -1;                   /* Shared. */

static int          main_midiIn[MIDI_MAXIMUM_IN]        = { 1 };        /* Shared. */
static int          main_midiOut[MIDI_MAXIMUM_OUT]      = { 1 };        /* Shared. */
static int          main_audioIn[AUDIO_MAXIMUM_IN]      = { 0 };        /* Shared. */
static int          main_audioOut[AUDIO_MAXIMUM_OUT]    = { 0 };        /* Shared. */
static int          main_channelIn[AUDIO_MAXIMUM_IN]    = { 0 };        /* Shared. */
static int          main_channelOut[AUDIO_MAXIMUM_OUT]  = { 0 };        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _fontinfo
{
    int fi_fontsize;
    int fi_maxwidth;
    int fi_maxheight;
    int fi_hostfontsize;
    int fi_width;
    int fi_height;
} t_fontinfo;

    /* these give the nominal point size and maximum height of the characters
    in the six fonts.  */

static t_fontinfo sys_fontlist[] = {
    {8, 6, 10, 1, 1, 1}, {10, 7, 13, 1, 1, 1}, {12, 9, 16, 1, 1, 1},
    {16, 10, 21, 1, 1, 1}, {24, 15, 25, 1, 1, 1}, {36, 25, 45, 1, 1, 1}};
#define NFONT (sizeof(sys_fontlist)/sizeof(*sys_fontlist))

/* here are the actual font size structs on msp's systems:
MSW:
font 8 5 9 8 5 11
font 10 7 13 10 6 13
font 12 9 16 14 8 16
font 16 10 20 16 10 18
font 24 15 25 16 10 18
font 36 25 42 36 22 41

linux:
font 8 5 9 8 5 9
font 10 7 13 12 7 13
font 12 9 16 14 9 15
font 16 10 20 16 10 19
font 24 15 25 24 15 24
font 36 25 42 36 22 41
*/

static t_fontinfo *sys_findfont(int fontsize)
{
    unsigned int i;
    t_fontinfo *fi;
    for (i = 0, fi = sys_fontlist; i < (NFONT-1); i++, fi++)
        if (fontsize < fi[1].fi_fontsize) return (fi);
    return (sys_fontlist + (NFONT-1));
}

int sys_nearestfontsize(int fontsize)
{
    return (sys_findfont(fontsize)->fi_fontsize);
}

int sys_hostfontsize(int fontsize)
{
    return (sys_findfont(fontsize)->fi_hostfontsize);
}

int sys_fontwidth(int fontsize)
{
    return (sys_findfont(fontsize)->fi_width);
}

int sys_fontheight(int fontsize)
{
    return (sys_findfont(fontsize)->fi_height);
}

int sys_defaultfont;    /* Shared. */
#define DEFAULTFONT 12

static void openit(const char *dirname, const char *filename)
{
    char dirbuf[PD_STRING], *nameptr;
    int fd = open_via_path(dirname, filename, "", dirbuf, &nameptr,
        PD_STRING, 0);
    if (fd >= 0)
    {
        close (fd);
        buffer_openFile(0, gensym(nameptr), gensym(dirbuf));
    }
    else
        post_error ("%s: can't open", filename);
}

/* this is called from the gui process.  The first argument is the cwd, and
succeeding args give the widths and heights of known fonts.  We wait until 
these are known to open files and send messages specified on the command line.
We ask the GUI to specify the "cwd" in case we don't have a local OS to get it
from; for instance we could be some kind of RT embedded system.  However, to
really make this make sense we would have to implement
open(), read(), etc, calls to be served somehow from the GUI too. */

void global_gui(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    char *cwd = atom_getSymbolAtIndex(0, argc, argv)->s_name;
    t_namelist *nl;
    unsigned int i;
    int j;
    int nhostfont = (argc-1)/3;
    /* */
    if (argc != 1 + 3 * nhostfont) { PD_BUG; }
    for (i = 0; i < NFONT; i++)
    {
        int best = 0;
        int wantheight = sys_fontlist[i].fi_maxheight;
        int wantwidth = sys_fontlist[i].fi_maxwidth;
        for (j = 0; j < nhostfont; j++)
        {
            if ((t_int)atom_getFloatAtIndex(3 * j + 3, argc, argv) <= wantheight &&
                (t_int)atom_getFloatAtIndex(3 * j + 2, argc, argv) <= wantwidth)
                    best = j;
        }
            /* best is now the host font index for the desired font index i. */
        sys_fontlist[i].fi_hostfontsize =
            (t_int)atom_getFloatAtIndex(3 * best + 1, argc, argv);
        sys_fontlist[i].fi_width = (t_int)atom_getFloatAtIndex(3 * best + 2, argc, argv);
        sys_fontlist[i].fi_height = (t_int)atom_getFloatAtIndex(3 * best + 3, argc, argv);
    }
#if 0
    for (i = 0; i < 6; i++)
        fprintf(stderr, "font (%d %d %d) -> (%d %d %d)\n",
            sys_fontlist[i].fi_fontsize,
            sys_fontlist[i].fi_maxwidth,
            sys_fontlist[i].fi_maxheight,
            sys_fontlist[i].fi_hostfontsize,
            sys_fontlist[i].fi_width,
            sys_fontlist[i].fi_height);
#endif
        /* load dynamic libraries specified with "-lib" args */
    for  (nl = sys_externlist; nl; nl = nl->nl_next)
        if (!sys_load_lib(0, nl->nl_string))
            post("%s: can't load library", nl->nl_string);
        /* open patches specifies with "-open" args */
    for  (nl = main_openList; nl; nl = nl->nl_next)
        openit(cwd, nl->nl_string);
    namelist_free(main_openList);
    main_openList = 0;
        /* send messages specified with "-send" args */
    for  (nl = main_messageList; nl; nl = nl->nl_next)
    {
        t_buffer *b = buffer_new();
        buffer_withStringUnzeroed(b, nl->nl_string, strlen(nl->nl_string));
        buffer_eval(b, 0, 0, 0);
        buffer_free(b);
    }
    namelist_free(main_messageList);
    main_messageList = 0;
}

static void sys_afterargparse(void);

/* this is called from main() in s_entry.c */
int sys_main(int argc, char **argv)
{
    int i, noprefs;
    //main_extra = 0;
    /* use Win32 "binary" mode by default since we don't want the
     * translation that Win32 does by default */
#ifdef _WIN32
# ifdef _MSC_VER /* MS Visual Studio */
    _set_fmode( _O_BINARY );
# else  /* MinGW */
    {
        extern int _fmode;
        _fmode = _O_BINARY;
    }
# endif /* _MSC_VER */
#endif  /* WIN32 */
    pd_initialize();                                  /* start the message system */
    sys_findprogdir(argv[0]);                   /* set sys_progname, guipath */
    for (i = noprefs = 0; i < argc; i++)        /* prescan args for noprefs */
        if (!strcmp(argv[i], "-noprefs"))
            noprefs = 1;
    if (!noprefs)
        sys_loadpreferences();                  /* load default settings */
#ifndef _WIN32
    if (!noprefs)
        sys_rcfile();                           /* parse the startup file */
#endif
    if (sys_argparse(argc-1, argv+1))           /* parse cmd line */
        return (1);
    sys_afterargparse();                    /* post-argparse settings */
    if (0 || main_version) fprintf(stderr, "%s %s compiled %s %s\n",
        PD_NAME, PD_VERSION, main_compileTime, main_compileDate);
    if (main_version)    /* if we were just asked our version, exit here. */
        return (0);
    sys_setsignalhandlers();
    if (sys_startgui(main_libDirectory->s_name))       /* start the gui */
        return (1);
    if (main_batch)
        return (scheduler_mainForBatchProcessing());
    else
    {
            /* open audio and MIDI */
        sys_reopen_midi();
        if (audio_shouldkeepopen())
            sys_reopen_audio();
            /* run scheduler until it quits */
        return (scheduler_main());
    }
}

static char *(usagemessage[]) = {
"usage: pd [-flags] [file]...\n",
"\naudio configuration flags:\n",
"-r <n>           -- specify sample rate\n",
"-audioindev ...  -- audio in devices; e.g., \"1,3\" for first and third\n",
"-audiooutdev ... -- audio out devices (same)\n",
"-audiodev ...    -- specify input and output together\n",
"-audioaddindev   -- add an audio input device by name\n",
"-audioaddoutdev  -- add an audio output device by name\n",
"-audioadddev     -- add an audio input and output device by name\n",
"-inchannels ...  -- audio input channels (by device, like \"2\" or \"16,8\")\n",
"-outchannels ... -- number of audio out channels (same)\n",
"-channels ...    -- specify both input and output channels\n",
"-audiobuf <n>    -- specify size of audio buffer in msec\n",
"-blocksize <n>   -- specify audio I/O block size in sample frames\n",
"-sleepgrain <n>  -- specify number of milliseconds to sleep when idle\n",
"-nodac           -- suppress audio output\n",
"-noadc           -- suppress audio input\n",
"-noaudio         -- suppress audio input and output (-nosound is synonym) \n",
"-callback        -- use callbacks if possible\n",
"-nocallback      -- use polling-mode (true by default)\n",
"-listdev         -- list audio and MIDI devices\n",

#ifdef USEAPI_OSS
"-oss             -- use OSS audio API\n",
#endif

#ifdef USEAPI_ALSA
"-alsa            -- use ALSA audio API\n",
"-alsaadd <name>  -- add an ALSA device name to list\n",
#endif

#ifdef USEAPI_JACK
"-jack            -- use JACK audio API\n",
#endif

#ifdef USEAPI_PORTAUDIO
#ifdef _WIN32
"-asio            -- use ASIO audio driver (via Portaudio)\n",
"-pa              -- synonym for -asio\n",
#else
"-pa              -- use Portaudio API\n",
#endif
#endif

#ifdef USEAPI_MMIO
"-mmio            -- use MMIO audio API (default for Windows)\n",
#endif

"      (default audio API for this platform:  ", API_DEFAULT_STRING, ")\n\n",

"\nMIDI configuration flags:\n",
"-midiindev ...   -- midi in device list; e.g., \"1,3\" for first and third\n",
"-midioutdev ...  -- midi out device list, same format\n",
"-mididev ...     -- specify -midioutdev and -midiindev together\n",
"-midiaddindev    -- add a MIDI input device by name\n",
"-midiaddoutdev   -- add a MIDI output device by name\n",
"-midiadddev      -- add a MIDI input and output device by name\n",
"-nomidiin        -- suppress MIDI input\n",
"-nomidiout       -- suppress MIDI output\n",
"-nomidi          -- suppress MIDI input and output\n",
#ifdef USEAPI_OSS
"-ossmidi         -- use OSS midi API\n",
#endif
#ifdef USEAPI_ALSA
"-alsamidi        -- use ALSA midi API\n",
#endif


"\nother flags:\n",
"-path <path>     -- add to file search path\n",
"-nostdpath       -- don't search standard (\"extra\") directory\n",
"-stdpath         -- search standard directory (true by default)\n",
"-helppath <path> -- add to help file search path\n",
"-open <file>     -- open file(s) on startup\n",
"-lib <file>      -- load object library(s)\n",
"-font-size <n>     -- specify default font size in points\n",
"-font-face <name>  -- specify default font\n",
"-font-weight <name>-- specify default font weight (normal or bold)\n",
"-verbose         -- extra printout on startup and when searching for files\n",
"-noverbose       -- no extra printout\n",
"-version         -- don't run Pd; just print out which version it is \n",
"-d <n>           -- specify debug level\n",
"-loadbang        -- do not suppress all loadbangs (true by default)\n",
"-noloadbang      -- suppress all loadbangs\n",
"-stderr          -- send printout to standard error instead of GUI\n",
"-nostderr        -- send printout to GUI instead of standard error (true by default)\n",
"-gui             -- start GUI (true by default)\n",
"-nogui           -- suppress starting the GUI\n",
"-guiport <n>     -- connect to pre-existing GUI over port <n>\n",
"-guicmd \"cmd...\" -- start alternatve GUI program (e.g., remote via ssh)\n",
"-send \"msg...\"   -- send a message at startup, after patches are loaded\n",
"-prefs           -- load preferences on startup (true by default)\n",
"-noprefs         -- suppress loading preferences on startup\n",
"-rt or -realtime -- use real-time priority\n",
"-nrt             -- don't use real-time priority\n",
"-sleep           -- sleep when idle, don't spin (true by default)\n",
"-nosleep         -- spin, don't sleep (may lower latency on multi-CPUs)\n",
"-schedlib <file> -- plug in external scheduler\n",
"-extraflags <s>  -- string argument to send schedlib\n",
"-batch           -- run off-line as a batch process\n",
"-nobatch         -- run interactively (true by default)\n",
"-autopatch       -- enable auto-connecting new from selected objects (true by default)\n",
"-noautopatch     -- defeat auto-patching new from selected objects\n",
"-compatibility <f> -- set back-compatibility to version <f>\n",
};

static void sys_parsedevlist(int *np, int *vecp, int max, char *str)
{
    int n = 0;
    while (n < max)
    {
        if (!*str) break;
        else
        {
            char *endp;
            vecp[n] = strtol(str, &endp, 10);
            if (endp == str)
                break;
            n++;
            if (!endp)
                break;
            str = endp + 1;
        }
    }
    *np = n;
}

static int sys_getmultidevchannels(int n, int *devlist)
{
    int sum = 0;
    if (n<0)return(-1);
    if (n==0)return 0;
    while(n--)sum+=*devlist++;
    return sum;
}


    /* this routine tries to figure out where to find the auxilliary files
    Pd will need to run.  This is either done by looking at the command line
    invokation for Pd, or if that fails, by consulting the variable
    INSTALL_PREFIX.  In MSW, we don't try to use INSTALL_PREFIX. */
void sys_findprogdir(char *progname)
{
    char sbuf[PD_STRING], sbuf2[PD_STRING], *sp;
    char *lastslash; 
#ifndef _WIN32
    struct stat statbuf;
#endif /* NOT _WIN32 */

    /* find out by what string Pd was invoked; put answer in "sbuf". */
#ifdef _WIN32
    GetModuleFileName(NULL, sbuf2, sizeof(sbuf2));
    sbuf2[PD_STRING-1] = 0;
    sys_unbashfilename(sbuf2, sbuf);
#else
    strncpy(sbuf, progname, PD_STRING);
    sbuf[PD_STRING-1] = 0;
#endif /* _WIN32 */
    lastslash = strrchr(sbuf, '/');
    if (lastslash)
    {
            /* bash last slash to zero so that sbuf is directory pd was in,
                e.g., ~/pd/bin */
        *lastslash = 0; 
            /* go back to the parent from there, e.g., ~/pd */
        lastslash = strrchr(sbuf, '/');
        if (lastslash)
        {
            strncpy(sbuf2, sbuf, lastslash-sbuf);
            sbuf2[lastslash-sbuf] = 0;
        }
        else strcpy(sbuf2, "..");
    }
    else
    {
            /* no slashes found.  Try INSTALL_PREFIX. */
#ifdef INSTALL_PREFIX
        strcpy(sbuf2, INSTALL_PREFIX);
#else
        strcpy(sbuf2, ".");
#endif
    }
        /* now we believe sbuf2 holds the parent directory of the directory
        pd was found in.  We now want to infer the "lib" directory and the
        "gui" directory.  In "simple" unix installations, the layout is
            .../bin/pd
            .../bin/pdwatchdog (etc)
            .../bin/ui_main.tcl
            .../doc
        and in "complicated" unix installations, it's:
            .../bin/pd
            .../lib/pd/bin/pdwatchdog
            .../lib/pd/bin/ui_main.tcl
            .../lib/pd/doc
        To decide which, we stat .../lib/pd; if that exists, we assume it's
        the complicated layout.  In MSW, it's the "simple" layout, but
        "wish" is found in bin:
            .../bin/pd
            .../bin/wish85.exe
            .../doc
        */
#ifdef _WIN32
    main_libDirectory = gensym(sbuf2);
#else
    strncpy(sbuf, sbuf2, PD_STRING-30);
    sbuf[PD_STRING-30] = 0;
    strcat(sbuf, "/lib/pd");
    if (stat(sbuf, &statbuf) >= 0)
    {
            /* complicated layout: lib dir is the one we just stat-ed above */
        main_libDirectory = gensym(sbuf);
    }
    else
    {
            /* simple layout: lib dir is the parent */
        main_libDirectory = gensym(sbuf2);
    }
#endif
}

#ifdef _WIN32
static int sys_mmio = 1;
#else
static int sys_mmio = 0;
#endif

int sys_argparse(int argc, char **argv)
{
    char sbuf[PD_STRING];
    int i;
    while ((argc > 0) && **argv == '-')
    {
        if (!strcmp(*argv, "-r") && argc > 1 &&
            sscanf(argv[1], "%d", &main_sampleRate) >= 1)
        {
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(*argv, "-inchannels") && (argc > 1))
        {
            sys_parsedevlist(&main_numberOfChannelIn,
                main_channelIn, AUDIO_MAXIMUM_IN, argv[1]);

          if (!main_numberOfChannelIn)
              goto usage;

          argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-outchannels") && (argc > 1))
        {
            sys_parsedevlist(&main_numberOfChannelOut, main_channelOut,
                AUDIO_MAXIMUM_OUT, argv[1]);

          if (!main_numberOfChannelOut)
            goto usage;

          argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-channels") && (argc > 1))
        {
            sys_parsedevlist(&main_numberOfChannelIn, main_channelIn,AUDIO_MAXIMUM_IN,
                argv[1]);
            sys_parsedevlist(&main_numberOfChannelOut, main_channelOut,AUDIO_MAXIMUM_OUT,
                argv[1]);

            if (!main_numberOfChannelOut)
              goto usage;

            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-soundbuf") || !strcmp(*argv, "-audiobuf") && (argc > 1))
        {
            main_advance = atoi(argv[1]);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-callback"))
        {
            main_callback = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nocallback"))
        {
            main_callback = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-blocksize"))
        {
            main_blockSize = atoi(argv[1]);
            argc -= 2; argv += 2;
        }
        /*
        else if (!strcmp(*argv, "-sleepgrain") && (argc > 1))
        {
            scheduler_sleepGrain = 1000 * atof(argv[1]);
            argc -= 2; argv += 2;
        }*/
        else if (!strcmp(*argv, "-nodac"))
        {
            main_numberOfAudioOut=0;
            main_numberOfChannelOut = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-noadc"))
        {
            main_numberOfAudioIn=0;
            main_numberOfChannelIn = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nosound") || !strcmp(*argv, "-noaudio"))
        {
            main_numberOfAudioIn=main_numberOfAudioOut = 0;
            main_numberOfChannelIn = main_numberOfChannelOut = 0;
            argc--; argv++;
        }
#ifdef USEAPI_OSS
        else if (!strcmp(*argv, "-oss"))
        {
            sys_set_audio_api(API_OSS);
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-ossmidi"))
        {
          sys_set_midi_api(API_OSS);
            argc--; argv++;
        }
#endif
#ifdef USEAPI_ALSA
        else if (!strcmp(*argv, "-alsa"))
        {
            sys_set_audio_api(API_ALSA);
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-alsaadd") && (argc > 1))
        {
            if (argc > 1)
                alsa_adddev(argv[1]);
            else goto usage;
            argc -= 2; argv +=2;
        }
        else if (!strcmp(*argv, "-alsamidi"))
        {
          sys_set_midi_api(API_ALSA);
            argc--; argv++;
        }
#endif
#ifdef USEAPI_JACK
        else if (!strcmp(*argv, "-jack"))
        {
            sys_set_audio_api(API_JACK);
            argc--; argv++;
        }
#endif
#ifdef USEAPI_PORTAUDIO
        else if (!strcmp(*argv, "-pa") || !strcmp(*argv, "-portaudio")
#ifdef _WIN32
            || !strcmp(*argv, "-asio")
#endif
            )
        {
            sys_set_audio_api(API_PORTAUDIO);
            sys_mmio = 0;
            argc--; argv++;
        }
#endif
#ifdef USEAPI_MMIO
        else if (!strcmp(*argv, "-mmio"))
        {
            sys_set_audio_api(API_MMIO);
            sys_mmio = 1;
            argc--; argv++;
        }
#endif
        else if (!strcmp(*argv, "-nomidiin"))
        {
            main_numberOfMidiIn = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nomidiout"))
        {
            main_numberOfMidiOut = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nomidi"))
        {
            main_numberOfMidiIn = main_numberOfMidiOut = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-midiindev") && (argc > 1))
        {
            sys_parsedevlist(&main_numberOfMidiIn, main_midiIn, MIDI_MAXIMUM_IN,
                argv[1]);
            if (!main_numberOfMidiIn)
                goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-midioutdev") && (argc > 1))
        {
            sys_parsedevlist(&main_numberOfMidiOut, main_midiOut, MIDI_MAXIMUM_OUT,
                argv[1]);
            if (!main_numberOfMidiOut)
                goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-mididev") && (argc > 1))
        {
            sys_parsedevlist(&main_numberOfMidiIn, main_midiIn, MIDI_MAXIMUM_IN,
                argv[1]);
            sys_parsedevlist(&main_numberOfMidiOut, main_midiOut, MIDI_MAXIMUM_OUT,
                argv[1]);
            if (!main_numberOfMidiOut)
                goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-midiaddindev") && (argc > 1))
        {
            if (main_numberOfMidiIn < 0)
                main_numberOfMidiIn = 0;
            if (main_numberOfMidiIn < MIDI_MAXIMUM_IN)
            {
                int devn = sys_mididevnametonumber(0, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find MIDI input device: %s\n",
                        argv[1]);
                else main_midiIn[main_numberOfMidiIn++] = devn + 1;
            }
            else fprintf(stderr, "number of MIDI devices limited to %d\n",
                MIDI_MAXIMUM_IN);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-midiaddoutdev") && (argc > 1))
        {
            if (main_numberOfMidiOut < 0)
                main_numberOfMidiOut = 0;
            if (main_numberOfMidiOut < MIDI_MAXIMUM_IN)
            {
                int devn = sys_mididevnametonumber(1, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find MIDI output device: %s\n",
                        argv[1]);
                else main_midiOut[main_numberOfMidiOut++] = devn + 1;
            }
            else fprintf(stderr, "number of MIDI devices limited to %d\n",
                MIDI_MAXIMUM_IN);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-midiadddev") && (argc > 1))
        {
            if (main_numberOfMidiIn < 0)
                main_numberOfMidiIn = 0;
            if (main_numberOfMidiOut < 0)
                main_numberOfMidiOut = 0;
            if (main_numberOfMidiIn < MIDI_MAXIMUM_IN && main_numberOfMidiOut < MIDI_MAXIMUM_IN)
            {
                int devn = sys_mididevnametonumber(1, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find MIDI output device: %s\n",
                        argv[1]);
                else main_midiOut[main_numberOfMidiIn++] = devn + 1;
                devn = sys_mididevnametonumber(1, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find MIDI output device: %s\n",
                        argv[1]);
                else main_midiOut[main_numberOfMidiOut++] = devn + 1;
            }
            else fprintf(stderr, "number of MIDI devices limited to %d",
                MIDI_MAXIMUM_IN);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-path") && (argc > 1))
        {
            sys_searchpath = namelist_append_files(sys_searchpath, argv[1]);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-nostdpath"))
        {
            sys_usestdpath = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-stdpath"))
        {
            sys_usestdpath = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-helppath"))
        {
            sys_helppath = namelist_append_files(sys_helppath, argv[1]);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-open") && argc > 1)
        {
            main_openList = namelist_append_files(main_openList, argv[1]);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-lib") && argc > 1)
        {
            sys_externlist = namelist_append_files(sys_externlist, argv[1]);
            argc -= 2; argv += 2;
        }
        else if ((!strcmp(*argv, "-font-size") || !strcmp(*argv, "-font"))
            && argc > 1)
        {
            sys_defaultfont = sys_nearestfontsize(atoi(argv[1]));
            argc -= 2;
            argv += 2;
        }
        /*else if ((!strcmp(*argv, "-font-face") || !strcmp(*argv, "-typeface"))
            && argc > 1)
        {
            strncpy(sys_font,*(argv+1),sizeof(sys_font)-1);
            sys_font[sizeof(sys_font)-1] = 0;
            argc -= 2;
            argv += 2;
        }*/
        /*else if (!strcmp(*argv, "-font-weight") && argc > 1)
        {
            strncpy(sys_fontweight,*(argv+1),sizeof(sys_fontweight)-1);
            sys_fontweight[sizeof(sys_fontweight)-1] = 0;
            argc -= 2;
            argv += 2;
        }
        else if (!strcmp(*argv, "-verbose"))
        {
            sys_verbose++;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-noverbose"))
        {
            sys_verbose=0;
            argc--; argv++;
        }*/
        else if (!strcmp(*argv, "-version"))
        {
            main_version = 1;
            argc--; argv++;
        }
        /* else if (!strcmp(*argv, "-d") && argc > 1 &&
            sscanf(argv[1], "%d", &sys_debuglevel) >= 1)
        {
            argc -= 2;
            argv += 2;
        }*/
        else if (!strcmp(*argv, "-loadbang"))
        {
            main_noLoadbang = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-noloadbang"))
        {
            main_noLoadbang = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-gui"))
        {
            main_noGUI = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nogui"))
        {
            main_noGUI = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-guiport") && argc > 1 &&
            sscanf(argv[1], "%d", &main_portNumber) >= 1)
        {
            argc -= 2;
            argv += 2;
        }
        /*else if (!strcmp(*argv, "-guicmd") && argc > 1)
        {
            main_commandToLaunchGUI = argv[1];
            argc -= 2; argv += 2;
        }*/
        else if (!strcmp(*argv, "-send") && argc > 1)
        {
            main_messageList = namelist_append(main_messageList, argv[1], 1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-listdev"))
        {
            main_devices = 1;
            argc--; argv++;
        }
        /*else if (!strcmp(*argv, "-extraflags") && argc > 1)
        {
            main_extra = 1;
            strncpy(sys_extraflagsstring, argv[1],
                sizeof(sys_extraflagsstring) - 1);
            argv += 2;
            argc -= 2;
        }*/
        else if (!strcmp(*argv, "-batch"))
        {
            main_batch = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nobatch"))
        {
            main_batch = 0;
            argc--; argv++;
        }
        /*else if (!strcmp(*argv, "-autopatch"))
        {
            sys_noautopatch = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-noautopatch"))
        {
            sys_noautopatch = 1;
            argc--; argv++;
        }*/
        /*else if (!strcmp(*argv, "-compatibility") && argc > 1)
        {
            float f;
            if (sscanf(argv[1], "%f", &f) < 1)
                goto usage;
            pd_compatibilitylevel = 0.5 + 100. * f;
            argv += 2;
            argc -= 2;
        }*/
        else if (!strcmp(*argv, "-rt") || !strcmp(*argv, "-realtime"))
        {
            main_highPriority = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nrt") || !strcmp(*argv, "-nort") || !strcmp(*argv, "-norealtime"))
        {
            main_highPriority = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-sleep"))
        {
            main_noSleep = 0;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-nosleep"))
        {
            main_noSleep = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-soundindev") ||
            !strcmp(*argv, "-audioindev"))
        {
            sys_parsedevlist(&main_numberOfAudioIn, main_audioIn,
                AUDIO_MAXIMUM_IN, argv[1]);
            if (!main_numberOfAudioIn)
                goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-soundoutdev") ||
            !strcmp(*argv, "-audiooutdev"))
        {
            sys_parsedevlist(&main_numberOfAudioOut, main_audioOut,
                AUDIO_MAXIMUM_OUT, argv[1]);
            if (!main_numberOfAudioOut)
                goto usage;
            argc -= 2; argv += 2;
        }
        else if ((!strcmp(*argv, "-sounddev") || !strcmp(*argv, "-audiodev"))
                 && (argc > 1))
        {
            sys_parsedevlist(&main_numberOfAudioIn, main_audioIn,
                AUDIO_MAXIMUM_IN, argv[1]);
            sys_parsedevlist(&main_numberOfAudioOut, main_audioOut,
                AUDIO_MAXIMUM_OUT, argv[1]);
            if (!main_numberOfAudioOut)
                goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-audioaddindev") && (argc > 1))
        {
            if (main_numberOfAudioIn < 0)
                main_numberOfAudioIn = 0;
            if (main_numberOfAudioIn < AUDIO_MAXIMUM_IN)
            {
                int devn = sys_audiodevnametonumber(0, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find audio input device: %s\n",
                        argv[1]);
                else main_audioIn[main_numberOfAudioIn++] = devn + 1;
            }
            else fprintf(stderr, "number of audio devices limited to %d\n",
                AUDIO_MAXIMUM_IN);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-audioaddoutdev") && (argc > 1))
        {
            if (main_numberOfAudioOut < 0)
                main_numberOfAudioOut = 0;
            if (main_numberOfAudioOut < AUDIO_MAXIMUM_IN)
            {
                int devn = sys_audiodevnametonumber(1, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find audio output device: %s\n",
                        argv[1]);
                else main_audioOut[main_numberOfAudioOut++] = devn + 1;
            }
            else fprintf(stderr, "number of audio devices limited to %d\n",
                AUDIO_MAXIMUM_IN);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-audioadddev") && (argc > 1))
        {
            if (main_numberOfAudioIn < 0)
                main_numberOfAudioIn = 0;
            if (main_numberOfAudioOut < 0)
                main_numberOfAudioOut = 0;
            if (main_numberOfAudioIn < AUDIO_MAXIMUM_IN && main_numberOfAudioOut < AUDIO_MAXIMUM_IN)
            {
                int devn = sys_audiodevnametonumber(0, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find audio input device: %s\n",
                        argv[1]);
                else main_audioIn[main_numberOfAudioIn++] = devn + 1;
                devn = sys_audiodevnametonumber(1, argv[1]);
                if (devn < 0)
                    fprintf(stderr, "Couldn't find audio output device: %s\n",
                        argv[1]);
                else main_audioOut[main_numberOfAudioOut++] = devn + 1;
            }
            else fprintf(stderr, "number of audio devices limited to %d",
                AUDIO_MAXIMUM_IN);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(*argv, "-noprefs")) /* did this earlier */
            argc--, argv++;
        else
        {
            unsigned int i;
        usage:
            for (i = 0; i < sizeof(usagemessage)/sizeof(*usagemessage); i++)
                fprintf(stderr, "%s", usagemessage[i]);
            return (1);
        }
    }
    if (main_batch)
        main_noGUI = 1;
#ifdef _WIN32
        /* we need to tell Windows to output UTF-8 */
        SetConsoleOutputCP(CP_UTF8);
#endif
    if (!sys_defaultfont)
        sys_defaultfont = DEFAULTFONT;
    for (; argc > 0; argc--, argv++) 
        main_openList = namelist_append_files(main_openList, *argv);


    return (0);
}

int sys_getblksize(void)
{
    return (AUDIO_DEFAULT_BLOCK);
}

    /* stuff to do, once, after calling sys_argparse() -- which may itself
    be called more than once (first from "settings, second from .pdrc, then
    from command-line arguments */
static void sys_afterargparse(void)
{
    char sbuf[PD_STRING];
    int i;
    int naudioindev, audioindev[AUDIO_MAXIMUM_IN], chindev[AUDIO_MAXIMUM_IN];
    int naudiooutdev, audiooutdev[AUDIO_MAXIMUM_OUT], choutdev[AUDIO_MAXIMUM_OUT];
    int nchindev, nchoutdev, rate, advance, callback, blocksize;
    int nmidiindev = 0, midiindev[MIDI_MAXIMUM_IN];
    int nmidioutdev = 0, midioutdev[MIDI_MAXIMUM_OUT];
        /* correct to make audio and MIDI device lists zero based.  On
        MMIO, however, "1" really means the second device (the first one
        is "mapper" which is was not included when the command args were
        set up, so we leave it that way for compatibility. */
    if (!sys_mmio)
    {
        for (i = 0; i < main_numberOfAudioIn; i++)
            main_audioIn[i]--;
        for (i = 0; i < main_numberOfAudioOut; i++)
            main_audioOut[i]--;
    }
    for (i = 0; i < main_numberOfMidiIn; i++)
        main_midiIn[i]--;
    for (i = 0; i < main_numberOfMidiOut; i++)
        main_midiOut[i]--;
    if (main_devices)
        sys_listdevs();
        
            /* get the current audio parameters.  These are set
            by the preferences mechanism (sys_loadpreferences()) or
            else are the default.  Overwrite them with any results
            of argument parsing, and store them again. */
    sys_get_audio_params(&naudioindev, audioindev, chindev,
        &naudiooutdev, audiooutdev, choutdev, &rate, &advance,
            &callback, &blocksize);
    if (main_numberOfChannelIn >= 0)
    {
        nchindev = main_numberOfChannelIn;
        for (i = 0; i < nchindev; i++)
            chindev[i] = main_channelIn[i];
    }
    else nchindev = naudioindev;
    if (main_numberOfAudioIn >= 0)
    {
        naudioindev = main_numberOfAudioIn;
        for (i = 0; i < naudioindev; i++)
            audioindev[i] = main_audioIn[i];
    }
    
    if (main_numberOfChannelOut >= 0)
    {
        nchoutdev = main_numberOfChannelOut;
        for (i = 0; i < nchoutdev; i++)
            choutdev[i] = main_channelOut[i];
    }
    else nchoutdev = naudiooutdev;
    if (main_numberOfAudioOut >= 0)
    {
        naudiooutdev = main_numberOfAudioOut;
        for (i = 0; i < naudiooutdev; i++)
            audiooutdev[i] = main_audioOut[i];
    }
    sys_get_midi_params(&nmidiindev, midiindev, &nmidioutdev, midioutdev);
    if (main_numberOfMidiIn >= 0)
    {
        nmidiindev = main_numberOfMidiIn;
        for (i = 0; i < nmidiindev; i++)
            midiindev[i] = main_midiIn[i];
    }
    if (main_numberOfMidiOut >= 0)
    {
        nmidioutdev = main_numberOfMidiOut;
        for (i = 0; i < nmidioutdev; i++)
            midioutdev[i] = main_midiOut[i];
    }
    if (main_advance)
        advance = main_advance;
    if (main_sampleRate)
        rate = main_sampleRate;
    if (main_callback)
        callback = main_callback;
    if (main_blockSize)
        blocksize = main_blockSize;
    sys_set_audio_settings(naudioindev, audioindev, nchindev, chindev,
        naudiooutdev, audiooutdev, nchoutdev, choutdev, rate, advance, 
        callback, blocksize);
    sys_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev, 0);
}

static void sys_addreferencepath(void)
{
    char sbuf[PD_STRING];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
