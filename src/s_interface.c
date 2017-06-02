
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryTcl;
extern t_symbol *main_directoryBin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int  main_portNumber;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WATCHDOG

int         interface_watchdogPipe;             /* Static. */

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int         interface_guiSocket;                /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_receiver  *interface_guiReceiver;             /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define INTERFACE_PORT                          5400

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ( PD_LINUX || PD_BSD || PD_HURD )
    #define INTERFACE_LOCALHOST                 "127.0.0.1"
#else
    #define INTERFACE_LOCALHOST                 "localhost"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if ! ( PD_WITH_NOGUI )

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error interface_fetchGui (struct sockaddr_in *server)
{
    struct hostent *host = gethostbyname (INTERFACE_LOCALHOST);
    t_error err = ((interface_guiSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0);
    err |= (fcntl (interface_guiSocket, F_SETFD, FD_CLOEXEC) == -1);
    
    PD_ASSERT (!err);
    
    if (host && !err) {
        server->sin_family = AF_INET;
        server->sin_port = htons ((unsigned short)main_portNumber);
        memcpy ((char *)&server->sin_addr, (char *)host->h_addr, host->h_length);
        err |= (connect (interface_guiSocket, (struct sockaddr *)server, sizeof (struct sockaddr_in)) != 0);
        PD_ASSERT (!err);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

static t_error interface_launchGuiSpawnProcess (void) 
{
    t_error err = PD_ERROR_NONE;
    
    char path[PD_STRING] = { 0 };
    char port[PD_STRING] = { 0 };
    char wish[PD_STRING] = { 0 };

    int spawned;

    err |= string_copy (path, PD_STRING, "\"");
    err |= string_add (path, PD_STRING, main_directoryTcl->s_name);
    err |= string_add (path, PD_STRING, "/ui_main.tcl\"");
    
    err |= string_sprintf (port, PD_STRING, "%d", port);
    
    err |= string_copy (wish, PD_STRING, "\"");
    err |= string_add (wish, PD_STRING, main_directoryBin->s_name);
    err |= string_add (wish, PD_STRING, "/wish85.exe\"");
    
    if (!err) {
        path_slashToBackslashIfNecessary (path);
        path_slashToBackslashIfNecessary (wish);
        err |= ((spawned = _spawnl (P_NOWAIT, wish, "wish85.exe", path, port, 0)) < 0);
    }
    
    PD_ASSERT (!err);
    
    return err;
}

#else

static t_error interface_launchGuiSpawnProcess (void) 
{
    t_error err = PD_ERROR_NONE;
    
    char path[PD_STRING]    = { 0 };
    char command[PD_STRING] = { 0 };
    
#if PD_APPLE

    char *wish[10] = 
        {
            "/Applications/Utilities/Wish.app/Contents/MacOS/Wish",
            "/Applications/Utilities/Wish Shell.app/Contents/MacOS/Wish Shell",
            "/Applications/Wish.app/Contents/MacOS/Wish",
            "/Applications/Wish Shell.app/Contents/MacOS/Wish Shell",
            "/Library/Frameworks/Tk.framework/Resources/Wish.app/Contents/MacOS/Wish",
            "/Library/Frameworks/Tk.framework/Resources/Wish Shell.app/Contents/MacOS/Wish Shell",
            "/System/Library/Frameworks/Tk.framework/Resources/Wish.app/Contents/MacOS/Wish",
            "/System/Library/Frameworks/Tk.framework/Resources/Wish Shell.app/Contents/MacOS/Wish Shell",
            "/usr/bin/wish"
            "wish"
        };
    
    int i; for (i = 0; i < 9; i++) { if (path_isFileExist (wish[i])) { break; } }

#endif

    err |= string_sprintf (path, PD_STRING, "%s/ui_main.tcl", main_directoryTcl->s_name);
    
#if PD_APPLE

    err |= string_sprintf (command, PD_STRING, 
            "\"%s\" \"%s\" %d\n", 
            wish[i], 
            path, 
            main_portNumber);

#else

    err |= string_sprintf (command, PD_STRING, 
            "wish \"%s\" %d\n",
            path, 
            main_portNumber);
    
#endif // PD_APPLE

    if (!err) {
    //
    if ((err = (path_isFileExist (path) == 0))) { PD_BUG; }
    else {
    //
    pid_t pid = fork();
    
    if (pid < 0)   { err = PD_ERROR; PD_BUG; }
    else if (!pid) {
        if (!priority_privilegeRelinquish()) {                      /* Child lose setuid privileges. */
            execl ("/bin/sh", "sh", "-c", command, NULL);
        }
        _exit (1);
    }
    //
    }
    //
    }
       
    return err;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error interface_launchGuiSocket (struct sockaddr_in *server, int *fd)
{
    int f = -1;
    t_error err = ((f = socket (AF_INET, SOCK_STREAM, 0)) < 0);
    err |= (fcntl (f, F_SETFD, FD_CLOEXEC) == -1);
    
    #if PD_WINDOWS
        char arg = 1;
        err |= (setsockopt (f, IPPROTO_TCP, TCP_NODELAY, &arg, sizeof (char)) < 0);
    #else
        int arg = 1;
        err |= (setsockopt (f, IPPROTO_TCP, TCP_NODELAY, &arg, sizeof (int)) < 0);
    #endif

    if (err) { PD_BUG; }
    else {
    //
    main_portNumber = INTERFACE_PORT;
    int n = 0;
    
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons ((unsigned short)main_portNumber);

    while (bind (f, (struct sockaddr *)server, sizeof (struct sockaddr_in)) < 0) {
    //
    #if PD_WINDOWS
        int e = WSAGetLastError();
    #else
        int e = errno;
    #endif

    if ((n > 20) || (e != EADDRINUSE)) { err |= PD_ERROR; PD_BUG; break; } 
    else {
        server->sin_port = htons ((unsigned short)++main_portNumber);
    }
    
    n++;
    //
    }
    
    if (!err) { *fd = f; }
    //
    }
    
    PD_ASSERT (!err);
    
    return err;
}

static t_error interface_launchGui (struct sockaddr_in *server, int *fd)
{
    t_error err = PD_ERROR_NONE;
    
    if (!(err |= interface_launchGuiSocket (server, fd))) { err |= interface_launchGuiSpawnProcess(); }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error interface_startGui (void)
{
    t_error err = PD_ERROR_NONE;
    
    struct sockaddr_in server;
    int f = -1;
    int launch = (main_portNumber == 0);
    
    if (!launch) { err = interface_fetchGui (&server); }    /* Wish first. */
    else {
    //
    if (!(err = interface_launchGui (&server, &f))) {       /* Executable first. */
        if (!(err = (listen (f, 5) < 0))) {
            socklen_t s = sizeof (struct sockaddr_in);
            err = ((interface_guiSocket = accept (f, (struct sockaddr *)&server, (socklen_t *)&s)) < 0);
        }
        PD_ASSERT (!err);
    }
    //
    }
    
    /* Listen GUI messages. */
    
    if (!err) { interface_guiReceiver = receiver_new (NULL, interface_guiSocket, NULL, NULL, 0, 0); }
    
    /* Initialize GUI. */
    
    if (!err) {
        t_pathlist *l = instance_getSearchPath();
        sys_vGui ("::initialize\n");
        while (l) {
            sys_vGui ("lappend ::var(searchPath) {%s}\n", pathlist_getPath (l));    // --
            l = pathlist_getNext (l);
        }
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_WITH_NOGUI

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error interface_start (void)
{
    #if PD_WITH_NOGUI
        t_error err = PD_ERROR_NONE;
    #else
        t_error err = interface_startGui();
    #endif
    
    PD_ASSERT (!err);
    
    if (!err && !(err = priority_privilegeRestore())) { 
        err |= priority_setPolicy();
        err |= priority_privilegeRelinquish();
    }
    
    PD_ASSERT (!err);
        
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void interface_quit (void)
{
    scheduler_needToExit();
}

#if PD_WATCHDOG

void interface_watchdog (void *dummy)
{
    if (write (interface_watchdogPipe, "\n", 1) < 1) { PD_BUG; scheduler_needToExitWithError(); }
}

#endif

void interface_initialize (void)
{
    #if PD_WINDOWS

    WSADATA d;
    short version = MAKEWORD (2, 0);
    
    if (WSAStartup (version, &d)) { PD_BUG; }
    
    #endif
}

void interface_release (void)
{
    if (interface_guiReceiver) { receiver_free (interface_guiReceiver); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
