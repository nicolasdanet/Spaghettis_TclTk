
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

t_symbol    *main_libDirectory;                                         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int         main_portNumber;                                            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  main_version;                                               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void main_entryPlatformSpecific (void)
{
    #if PD_WINDOWS
    #if PD_MSVC
        _set_fmode( _O_BINARY );
    #else
        { extern int _fmode; _fmode = _O_BINARY; }
    #endif
    #endif
}

static t_error main_entryVersion (void)
{
    char t[PD_STRING];
    t_error err = utils_version (t, PD_STRING);
    if (!err) { fprintf (stdout, "%s\n", t); }
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int main_entry (int argc, char **argv)
{
    main_entryPlatformSpecific();
    
    pd_initialize();
    sys_findprogdir (argv[0]);
    sys_loadpreferences();

    if (sys_argparse (argc - 1, argv + 1)) { return 1; }
    if (main_version) { return main_entryVersion(); }
    
    sys_setsignalhandlers();
    
    if (sys_startgui (main_libDirectory->s_name)) { return 1; }
    sys_reopen_midi();
    if (audio_shouldkeepopen()) { sys_reopen_audio(); }

    return (scheduler_main());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

static void sys_addreferencepath(void)
{
    char sbuf[PD_STRING];
}

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

int sys_argparse(int argc, char **argv)
{
    char sbuf[PD_STRING];
    int i;
    while ((argc > 0) && **argv == '-')
    {
        if (!strcmp(*argv, "-version"))
        {
            main_version = 1;
            argc--; argv++;
        }
        else if (!strcmp(*argv, "-guiport") && argc > 1 &&
            sscanf(argv[1], "%d", &main_portNumber) >= 1)
        {
            argc -= 2;
            argv += 2;
        }
        else
        {
            unsigned int i;
            for (i = 0; i < sizeof(usagemessage)/sizeof(*usagemessage); i++)
                fprintf(stderr, "%s", usagemessage[i]);
            return (1);
        }
    }

#ifdef _WIN32
        /* we need to tell Windows to output UTF-8 */
        SetConsoleOutputCP(CP_UTF8);
#endif

    return (0);
}

int sys_getblksize (void)
{
    return (AUDIO_DEFAULT_BLOCK);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
