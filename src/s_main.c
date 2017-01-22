
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *main_directoryRoot;                        /* Shared. */
t_symbol    *main_directoryBin;                         /* Shared. */
t_symbol    *main_directoryTcl;                         /* Shared. */
t_symbol    *main_directoryHelp;                        /* Shared. */
t_symbol    *main_directoryExtras;                      /* Shared. */

int         main_portNumber;                            /* Shared. */
int         main_directoryWriteRequirePrivileges;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  main_version;                               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void main_entryNative (void)
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
    char t[PD_STRING] = { 0 };
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
        fprintf (stderr, "Usage: pd [ --version ] [ -port port ]\n");
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
        .../help/
        
    In "complexe" installations, the layout is
    
        .../bin/pd
        .../lib/pd/bin/pdwatchdog
        .../lib/pd/tcl/ui_main.tcl
        .../lib/pd/help/

*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* < https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe > */
/* < https://stackoverflow.com/questions/933850/how-to-find-the-location-of-the-executable-in-c > */

#if PD_WINDOWS

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    GetModuleFileName (NULL, dest, length); dest[length - 1] = 0;
        
    return PD_ERROR_NONE;
}

#elif PD_APPLE

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    t_error err = PD_ERROR_NONE;

    char path[PATH_MAX];
    int size = sizeof (path);

    err = (_NSGetExecutablePath (path, &size) != 0);
    
    if (!err) { 
        char *s = NULL;
        if (s = realpath (path, NULL)) { err |= string_copy (dest, length, s); free (s); }
    }

    return err;
}

#elif PD_LINUX

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    t_error err = PD_ERROR_NONE;
    
    char path[PATH_MAX];
    
    ssize_t t = readlink ("/proc/self/exe", path, PATH_MAX);
    
    if (!(err = (t < 0 || t >= PATH_MAX))) {
        char *s = NULL;
        path[t] = 0;
        if (s = realpath (path, NULL)) { err |= string_copy (dest, length, s); free (s); }
    }
    
    return err;
}

#else
    #error
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error main_getRootDirectory (void)
{
    t_error err = PD_ERROR_NONE;
    char buf1[PD_STRING] = { 0 };
    char buf2[PD_STRING] = { 0 };
    char *slash = NULL; 
    
    #if PD_WINDOWS
        err |= main_getExecutablePathNative (buf1, PD_STRING);
        path_backslashToSlashIfNecessary (buf1, buf1);
    #else
        err |= main_getExecutablePathNative (buf1, PD_STRING);
    #endif
    
    /* Dirname of the executable's parent directory. */
    
    if (!err) { 
        if (!(err |= !(slash = strrchr (buf1, '/')))) { *slash = 0; }
        if (!(err |= !(slash = strrchr (buf1, '/')))) { *slash = 0; }
    }

    if (!err) {
    //
    #if PD_WINDOWS
        main_directoryRoot = gensym (buf1);
    #else
        err = string_copy (buf2, PD_STRING, buf1);
        err |= string_add (buf2, PD_STRING, "/lib/pd");
        
        if (!err && path_isFileExist (buf2)) {                                              /* Complexe. */
            main_directoryRoot = gensym (buf2); main_directoryWriteRequirePrivileges = 1;
            
        } else {
            main_directoryRoot = gensym (buf1);                                             /* Simple. */
        }
    #endif
    //
    }
    
    return err;
}

t_error main_setPaths (t_symbol *root)
{
    if (root == NULL) { PD_BUG; return PD_ERROR; }
    else {
    //
    t_error err = PD_ERROR_NONE;
    
    char t[PD_STRING] =  { 0 };
    
    const char *s = root->s_name;
    
    if (!(err |= string_sprintf (t, PD_STRING, "%s/bin",    s))) { main_directoryBin    = gensym (t); }
    if (!(err |= string_sprintf (t, PD_STRING, "%s/tcl",    s))) { main_directoryTcl    = gensym (t); }
    if (!(err |= string_sprintf (t, PD_STRING, "%s/help",   s))) { main_directoryHelp   = gensym (t); }
    if (!(err |= string_sprintf (t, PD_STRING, "%s/extras", s))) { main_directoryExtras = gensym (t); }
    
    return err;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int main_entry (int argc, char **argv)
{
    t_error err = priority_privilegeStart();
    
    if (!err && !(err = priority_privilegeDrop())) {
    //
    main_entryNative();
    
    /* Note that the order of operations below is crucial. */
    
    message_initialize();
    
    err |= main_getRootDirectory();
    err |= main_parseArguments (argc - 1, argv + 1);
    err |= main_setPaths (main_directoryRoot);
    
    PD_ASSERT (main_directoryRoot   != NULL);
    PD_ASSERT (main_directoryBin    != NULL);
    PD_ASSERT (main_directoryTcl    != NULL);
    PD_ASSERT (main_directoryHelp   != NULL);
    PD_ASSERT (main_directoryExtras != NULL);
    
    err |= logger_initialize();

    if (!err) {
    //
    if (main_version) { 
        return main_entryVersion (0); 
    }
    
    err |= audio_initialize();
    
    if (!err) {
    //
    midi_initialize();
    instance_initialize();
    sys_setSignalHandlers();
    
    setup_initialize();
    preferences_load();
    
    if (!(err |= interface_start())) {
        if (!(err |= main_entryVersion (1))) { err |= scheduler_main(); }
    }
    
    setup_release();
    instance_release();
    midi_release();
    audio_release(); 
    //
    }
    
    logger_release();
    //
    }
    
    message_release();
    //
    }
    
    #if PD_WITH_DEBUG
        post_log ("Shutdown");
    #endif
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
