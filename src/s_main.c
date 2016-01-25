
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
    
    SetConsoleOutputCP (CP_UTF8);
    
    #endif
}

static t_error main_entryVersion (void)
{
    char t[PD_STRING];
    t_error err = utils_version (t, PD_STRING);
    if (!err) { fprintf (stdout, "%s\n", t); }
    return err;
}

static t_error main_parseArguments (int argc, char **argv)
{
    t_error err = PD_ERROR_NONE;
    
    while (!err && (argc > 0) && (**argv == '-')) {
    //
    if (!strcmp (*argv, "--version")) { 
        main_version = 1; 
        argc--; argv++; 

    } else if (!strcmp (*argv, "-guiport") && (argc > 1)) {
        if (sscanf (argv[1], "%d", &main_portNumber) >= 1) { argc -= 2; argv += 2; }
        else {
            err = PD_ERROR;
        }
        
    } else {
        err = PD_ERROR;
    }
    //
    }

    if (err) {
        fprintf (stderr, "Usage: pd [ --version ] [ -guiport port ]\n"); 
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int main_entry (int argc, char **argv)
{
    main_entryPlatformSpecific();
    
    pd_initialize();
    sys_findprogdir (argv[0]);
    sys_loadpreferences();

    if (main_parseArguments (argc - 1, argv + 1)) { return 1; }
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

int sys_getblksize (void)
{
    return (AUDIO_DEFAULT_BLOCK);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
