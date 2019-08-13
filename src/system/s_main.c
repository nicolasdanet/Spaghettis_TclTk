
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *main_directoryTcl;                         /* Static. */
t_symbol    *main_directoryHelp;                        /* Static. */
t_symbol    *main_directorySupport;                     /* Static. */
t_symbol    *main_directoryTemplates;                   /* Static. */

int         main_portNumber;                            /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  main_version;                               /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error     audio_initialize    (void);
void        audio_release       (void);
void        message_initialize  (void);
void        message_release     (void);
void        midi_initialize     (void);
void        midi_release        (void);
void        setup_initialize    (void);
void        setup_release       (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error main_entryVersion (int console)
{
    char t[PD_STRING] = { 0 };
    t_error err = utils_version (t, PD_STRING);
    
    if (!err) {
        if (!console) { fprintf (stdout, "%s\n", t); }
        else {
            gui_vAdd ("::version {%s}\n", t); recentfiles_update();     // --
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
        fprintf (stderr, "Usage: pd [ --version ] [ -port port ]\n");   // --
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* 
    Simple layout is:
    
        .../bin/spaghettis
        .../tcl/ui_main.tcl
        .../help/
        
    Complexe layout is:
    
        .../bin/spaghettis
        .../lib/spaghettis/tcl/ui_main.tcl
        .../lib/spaghettis/help/

*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe > */
/* < https://stackoverflow.com/questions/933850/how-to-find-the-location-of-the-executable-in-c > */

#if PD_APPLE

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    t_error err = PD_ERROR_NONE;

    char path[PATH_MAX];
    uint32_t size = sizeof (path);

    err = (_NSGetExecutablePath (path, &size) != 0);
    
    if (!err) { 
        char *s = NULL;
        if ((s = realpath (path, NULL))) { err |= string_copy (dest, length, s); free (s); }
    }

    return err;
}

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_LINUX

static t_error main_getExecutablePathNative (char *dest, size_t length)
{
    t_error err = PD_ERROR_NONE;
    
    char path[PATH_MAX];
    
    ssize_t t = readlink ("/proc/self/exe", path, PATH_MAX);
    
    if (!(err = (t < 0 || t >= PATH_MAX))) {
        char *s = NULL;
        path[t] = 0;
        if ((s = realpath (path, NULL))) { err |= string_copy (dest, length, s); free (s); }
    }
    
    return err;
}

#endif // PD_LINUX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol *main_getRootDirectory (void)
{
    t_error err = PD_ERROR_NONE;
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    char *slash = NULL; 
    
    err |= main_getExecutablePathNative (t1, PD_STRING);

    /* Name of the executable's parent directory. */
    
    if (!err) { 
        if (!(err |= !(slash = strrchr (t1, '/')))) { *slash = 0; }
        if (!(err |= !(slash = strrchr (t1, '/')))) { *slash = 0; }
    }

    if (!err) {
    //
    err = string_copy (t2, PD_STRING, t1);
    err |= string_add (t2, PD_STRING, "/lib/" PD_NAME_LOWERCASE);
    
    if (!err) {
        if (path_isFileExist (t2)) { return gensym (t2); }      /* Complexe. */
        else {
            return gensym (t1);                                 /* Simple. */
        }
    }
    //
    }
    
    return NULL;
}

static t_error main_setPathsTemplates (t_symbol *support)
{
    t_error err = PD_ERROR_NONE;
    
    char t[PD_STRING] = { 0 };
    
    err |= string_sprintf (t, PD_STRING, "%s/templates", support->s_name);
    
    if (!err) { err |= path_createDirectoryIfNeeded (t); }
    if (!err) { main_directoryTemplates = gensym (t); }
    
    return err;
}

static t_error main_setPaths (t_symbol *root)
{
    if (root == NULL) { PD_BUG; return PD_ERROR; }
    else {
    //
    t_error err = PD_ERROR_NONE;
    
    char t[PD_STRING] = { 0 };
    
    const char *s = root->s_name;
    const char *home = getenv ("HOME");
    
    err |= (home == NULL);
    
    if (!err) {
    //
    #if PD_APPLE
    
    err |= string_sprintf (t, PD_STRING, "%s/Library/Application Support/" PD_NAME, home);
    
    #else
    
    err |= string_sprintf (t, PD_STRING, "%s/.config", home);
    
    if (!err) { err |= path_createDirectoryIfNeeded (t); }
    
    err |= string_sprintf (t, PD_STRING, "%s/.config/" PD_NAME_LOWERCASE, home);
    
    #endif
    
    if (!err) { err |= path_createDirectoryIfNeeded (t); }
    
    if (!err) { main_directorySupport = gensym (t); }
    if (!err) { err |= main_setPathsTemplates (main_directorySupport); }
    if (!err) {
    if (!(err |= string_sprintf (t, PD_STRING, "%s/tcl",  s))) { main_directoryTcl  = gensym (t); }
    if (!(err |= string_sprintf (t, PD_STRING, "%s/help", s))) { main_directoryHelp = gensym (t); }
    }
    //
    }
    
    return err;
    //
    }
}

/* Prevent to run multiple instances (it doesn't defeat an obstinate user). */
/* Assume that the lock is released by the system in all the ending situations. */

static int main_alreadyExists (void)
{
    #if PD_LINUX
    
    char path[PD_STRING] = { 0 };
    
    t_error err = string_sprintf (path, PD_STRING, "%s/spaghettis.lock", main_directorySupport->s_name);

    if (!err) {
    //
    int f = file_openWrite (path);
    
    err = (f < 0);

    if (!err) {
    //
    err = file_lock (f);
    //
    }
    //
    }
    
    PD_ASSERT (!err);
    
    return (err != PD_ERROR_NONE);
    
    #else
    
    return 0;   /* Don't care on macOS. */
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that order of calls below may be critical. */

int main_entry (int argc, char **argv)
{
    t_error err = privilege_start();
    
    if (!err && !(err = privilege_drop())) {
    //
    sys_setSignalHandlers();
    
    #if PD_WITH_DEBUG
        leak_initialize();
    #endif
    
    message_initialize();   /* Preallocate symbols and binding mechanism first. */
    
    err |= main_parseArguments (argc - 1, argv + 1);
    err |= main_setPaths (main_getRootDirectory());

    PD_ASSERT (main_directoryTcl        != NULL);
    PD_ASSERT (main_directoryHelp       != NULL);
    PD_ASSERT (main_directorySupport    != NULL);
    PD_ASSERT (main_directoryTemplates  != NULL);
    
    if (!err) {
    //
    main_version |= main_alreadyExists();
    
    if (main_version) { err |= main_entryVersion (0); }
    else {
    //
    if (!err) {
    //
    err |= logger_initialize();

    if (!err) {
    //
    err |= audio_initialize();
    
    if (!err) {
    //
    midi_initialize();
    setup_initialize();                     /* Instance initialized. */
    preferences_load();
    
    if (searchpath_scan() != PD_ERROR_NONE) { PD_BUG; }
    
    if (!(err |= interface_start())) {      /* DSP thread created. */
    
        if (!(err |= main_entryVersion (1))) { err |= scheduler_main(); }
    }
    
    setup_release();                        /* Instance released. */
    midi_release();
    audio_release(); 
    //
    }
    
    logger_release();
    //
    }
    //
    }
    //
    }
    //
    }
    
    message_release();
    
    #if PD_WITH_DEBUG
        leak_release(); post_log ("Shutdown");
    #endif
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
