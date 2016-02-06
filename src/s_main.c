
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
#pragma mark -

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

/* < https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe > */

static t_error main_getExecutablePath (char *dest, size_t length)
{
    t_error err = PD_ERROR_NONE;
    
#if PD_WINDOWS

    GetModuleFileName (NULL, dest, length);
    dest[length - 1] = 0;
        
#elif PD_APPLE
    
    char *real = NULL;
    char path[PATH_MAX + 1];
	int size = sizeof (path);

	err = (_NSGetExecutablePath (path, &size) != 0);
    
    if (!err) { 
        if (real = realpath (path, NULL)) { err |= string_copy (dest, length, real); free (real); }
    }
    
#else
    
    err = PD_ERROR; PD_BUG;
    
#endif
    
    return err;
}

static t_error main_getRootDirectory (void)
{
    t_error err = PD_ERROR_NONE;
    char buf1[PD_STRING] = { 0 };
    char buf2[PD_STRING] = { 0 };
    char *slash = NULL; 
    
    #if PD_WINDOWS
        err |= main_getExecutablePath (buf2, PD_STRING);
        sys_unbashfilename (buf2, buf1);
        *buf2 = 0;
    #else
        err |= main_getExecutablePath (buf1, PD_STRING);
    #endif
    
    /* Dirname of the executable's parent directory. */
    
    if (!err) { 
        if (!(err |= !(slash = strrchr (buf1, '/')))) { *slash = 0; }
        if (!(err |= !(slash = strrchr (buf1, '/')))) { *slash = 0; }
    }

    if (!err) {
    //
    #if PD_WINDOWS
        main_rootDirectory = gensym (buf1);
    #else
        err = string_copy (buf2, PD_STRING, buf1);
        err |= string_add (buf2, PD_STRING, "/lib/pd");
        
        if (!err && path_isFileExist (buf2)) { main_rootDirectory = gensym (buf2); }    /* Complexe. */
        else {
            main_rootDirectory = gensym (buf1);                                         /* Simple. */
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
    t_error err = PD_ERROR_NONE;
    
    main_entryPlatformSpecific();
    
    err |= main_getRootDirectory(); post_log ("! %s", main_rootDirectory->s_name);
    err |= main_parseArguments (argc - 1, argv + 1);
    
    if (!err) {
    //
    if (main_version) { 
        return main_entryVersion (0); 
    }
        
    pd_initialize();
    interface_initialize();
        
    preferences_load();
    sys_setSignalHandlers();
    
    PD_ASSERT (main_rootDirectory != NULL);
    
    if (!(err |= interface_start())) {
        sys_reopen_midi();
        if (audio_shouldkeepopen()) { sys_reopen_audio(); }
        if (!(err |= main_entryVersion (1))) { err |= scheduler_main(); }
    }
    
    interface_release();
    pd_release();
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
