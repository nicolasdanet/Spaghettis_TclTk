
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

t_symbol    *main_rootDirectory;        /* Shared. */

int         main_portNumber;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  main_version;               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void main_entryPlatformSpecific (void)
{
    #if PD_WINDOWS
    
    #if PD_MSVC
        _set_fmode (_O_BINARY);
    #else
        { extern int _fmode; _fmode = _O_BINARY; }
    #endif
    
    SetConsoleOutputCP (CP_UTF8);
    
    #endif
}

static t_error main_entryVersion (int console)
{
    char t[PD_STRING];
    t_error err = utils_version (t, PD_STRING);
    
    if (!err) {
        if (!console) { fprintf (stdout, "%s\n", t); }
        else {
            post ("%s", t);
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error main_parseArguments (int argc, char **argv)
{
    t_error err = PD_ERROR_NONE;
    
    while (!err && (argc > 0) && (**argv == '-')) {
    //
    if (!strcmp (*argv, "--version")) { 
        main_version = 1; argc--; argv++; 

    } else if (!strcmp (*argv, "-port") && (argc > 1)) {
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
        fprintf (stderr, "Usage: pd [ --version ] [ -port port ]\n");    // --
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* 
    In "simple" installations, the layout is
    
        .../bin/pd
        .../bin/pdwatchdog
        .../tcl/ui_main.tcl
        
    In "complexe" installations, the layout is
    
        .../bin/pd
        .../lib/pd/bin/pdwatchdog
        .../lib/pd/tcl/ui_main.tcl

*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error main_findRootDirectory (char *progname)
{
    t_error err = PD_ERROR_NONE;
    char buf1[PD_STRING] = { 0 };
    char buf2[PD_STRING] = { 0 };
    char *slash = NULL; 
    
    #if PD_WINDOWS
        GetModuleFileName (NULL, buf2, PD_STRING);
        buf2[PD_STRING - 1] = 0;
        sys_unbashfilename (buf2, buf1);
    #else
        err |= string_copy (buf1, PD_STRING, progname);
    #endif
    
    *buf2 = 0;
    
    slash = strrchr (buf1, '/');
    
    if (!slash) { err |= string_add (buf2, PD_STRING, ".."); }
    else {
        *slash = 0;
        if ((slash = strrchr (buf1, '/'))) { err |= string_append (buf2, PD_STRING, buf1, slash - buf1); }
        else {
            if (*buf1 == '.') { err |= string_add (buf2, PD_STRING, ".."); }
            else {
                err |= string_add (buf2, PD_STRING, ".");
            }
        }
    }
    
    if (!err) {
    //
    #if PD_WINDOWS
        main_rootDirectory = gensym (buf2);      /* Dirname of the executable's parent directory. */
    #else
        err |= string_copy (buf1, PD_STRING, buf2);
        err |= string_add (buf1, PD_STRING, "/lib/pd");
        
        if (path_isFileExist (buf1)) { main_rootDirectory = gensym (buf1); }        /* Complexe. */
        else {
            main_rootDirectory = gensym (buf2);                                     /* Simple. */
        }
    #endif
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int main_entry (int argc, char **argv)
{
    main_entryPlatformSpecific();
    
    if (main_findRootDirectory (argv[0]))           { return 1; }
    if (main_parseArguments (argc - 1, argv + 1))   { return 1; }
    if (main_version) { 
        return main_entryVersion (0); 
    }
        
    pd_initialize();
    preferences_load();
    sys_setSignalHandlers();
    
    if (sys_startgui (main_rootDirectory->s_name))  { return 1; }
    sys_reopen_midi();
    if (audio_shouldkeepopen()) { sys_reopen_audio(); }

    main_entryVersion (1);
    
    scheduler_main();
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
