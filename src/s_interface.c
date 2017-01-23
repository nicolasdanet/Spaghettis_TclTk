
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

    typedef int socklen_t;
    
    #define EADDRINUSE WSAEADDRINUSE

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INTERFACE_PORT                      5400

#if ( PD_LINUX || PD_BSD || PD_HURD )
    #define INTERFACE_LOCALHOST             "127.0.0.1"
#else
    #define INTERFACE_LOCALHOST             "localhost"
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INTERFACE_GUI_BUFFER_START_SIZE     (1024 * 128)
#define INTERFACE_GUI_BUFFER_ABORT_SIZE     (1024 * 1024 * 1024)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _fdpoll {
    void        *fdp_p;
    int         fdp_fd;
    t_pollfn    fdp_fn;
    } t_fdpoll;

typedef struct _guiqueue {
    void                *gq_p;
    t_glist             *gq_glist;
    t_drawfn            gq_fn;
    struct _guiqueue    *gq_next;
    } t_guiqueue;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol     *main_directoryTcl;
extern t_symbol     *main_directoryBin;
extern t_pathlist   *path_search;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int  main_portNumber;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WATCHDOG

int interface_watchdogPipe;                                         /* Shared. */

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_receiver  *interface_inGuiReceiver;                               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_fdpoll             *interface_inPollers;                   /* Shared. */

static int                  interface_inPollersSize;                /* Shared. */
static int                  interface_inMaximumFileDescriptor;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_guiqueue           *interface_outGuiQueue;                 /* Shared. */
static char                 *interface_outGuiBuffer;                /* Shared. */

static int                  interface_outGuiBufferSize;             /* Shared. */
static int                  interface_outGuiBufferHead;             /* Shared. */
static int                  interface_outGuiBufferTail;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int                  interface_guiSocket;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int interface_monitorInOut (int microseconds)
{
    int didSomething = 0;
    struct timeval timeOut;
    t_fdpoll *pollers = NULL;
    int i;
    
    timeOut.tv_sec  = 0;
    timeOut.tv_usec = microseconds;
    
    fd_set rSet;
    fd_set wSet;
    fd_set eSet;
    
    FD_ZERO (&rSet);
    FD_ZERO (&wSet);
    FD_ZERO (&eSet);
    
    for (pollers = interface_inPollers, i = interface_inPollersSize; i--; pollers++) {
        FD_SET (pollers->fdp_fd, &rSet);
    }

    select (interface_inMaximumFileDescriptor + 1, &rSet, &wSet, &eSet, &timeOut);
    
    for (i = 0; i < interface_inPollersSize; i++) {
        if (FD_ISSET (interface_inPollers[i].fdp_fd, &rSet)) {
            (*interface_inPollers[i].fdp_fn) (interface_inPollers[i].fdp_p, interface_inPollers[i].fdp_fd);
            didSomething = 1;
        }
    }
    
    return didSomething;
}

static void interface_increaseGuiBuffer()
{
    int oldSize = interface_outGuiBufferSize;
    int newSize = oldSize * 2;
    
    PD_ASSERT (newSize <= INTERFACE_GUI_BUFFER_ABORT_SIZE); 
    PD_ABORT (newSize > INTERFACE_GUI_BUFFER_ABORT_SIZE);           /* GUI buffer no more consumed? */
    
    interface_outGuiBuffer = PD_MEMORY_RESIZE (interface_outGuiBuffer, oldSize, newSize);
    interface_outGuiBufferSize = newSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int interface_monitorBlocking (int microseconds)
{
    return interface_monitorInOut (microseconds);
}

int interface_monitorNonBlocking (void)
{
    return interface_monitorInOut (0);
}

void interface_monitorAddPoller (int fd, t_pollfn fn, void *ptr)
{
    int n = interface_inPollersSize;
    int oldSize = n * sizeof (t_fdpoll);
    int newSize = oldSize + sizeof (t_fdpoll);
    int i;
    t_fdpoll *p = NULL;
    
    for (i = n, p = interface_inPollers; i--; p++) { PD_ASSERT (p->fdp_fd != fd); }
    
    interface_inPollers = (t_fdpoll *)PD_MEMORY_RESIZE (interface_inPollers, oldSize, newSize);
        
    p = interface_inPollers + n;
    p->fdp_p = ptr;
    p->fdp_fd = fd;
    p->fdp_fn = fn;
        
    interface_inPollersSize = n + 1;
    if (fd > interface_inMaximumFileDescriptor) { interface_inMaximumFileDescriptor = fd; }
}

void interface_monitorRemovePoller (int fd)
{
    int n = interface_inPollersSize;
    int oldSize = n * sizeof (t_fdpoll);
    int newSize = oldSize - sizeof (t_fdpoll);
    int i;
    t_fdpoll *p;
    
    PD_ASSERT (oldSize != 0);
    
    for (i = n, p = interface_inPollers; i--; p++) {
    //
    if (p->fdp_fd == fd) {
    //
    while (i--) { *p = *(p + 1); p++; }
    interface_inPollers = (t_fdpoll *)PD_MEMORY_RESIZE (interface_inPollers, oldSize, newSize);
    interface_inPollersSize = n - 1;
    return;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_guiQueueAddIfNotAlreadyThere (void *owner, t_glist *glist, t_drawfn f)
{
    t_guiqueue **qNext = NULL;
    t_guiqueue *q = NULL;
    
    if (!interface_outGuiQueue) { qNext = &interface_outGuiQueue; }
    else {
        t_guiqueue *t = NULL;
        for (t = interface_outGuiQueue; t->gq_next; t = t->gq_next) { if (t->gq_p == owner) { return; } }
        if (t->gq_p == owner) { return; }
        else {
            qNext = &t->gq_next;
        }
    }
    
    q = (t_guiqueue *)PD_MEMORY_GET (sizeof (t_guiqueue));
    
    q->gq_p     = owner;
    q->gq_glist = glist;
    q->gq_fn    = f;
    q->gq_next  = NULL;
    
    *qNext = q;
}

void interface_guiQueueRemove (void *owner)
{
    while (interface_outGuiQueue && interface_outGuiQueue->gq_p == owner) {
        t_guiqueue *first = interface_outGuiQueue;
        interface_outGuiQueue = interface_outGuiQueue->gq_next;
        PD_MEMORY_FREE (first);
    }
    
    if (interface_outGuiQueue) {
        t_guiqueue *q1 = NULL;
        t_guiqueue *q2 = NULL;
        for (q1 = interface_outGuiQueue; q2 = q1->gq_next; q1 = q2) {
            if (q2->gq_p == owner) { q1->gq_next = q2->gq_next; PD_MEMORY_FREE (q2); break; }
        }
    }
}

void interface_guiQueueRelease (void)
{
    while (interface_outGuiQueue) {
    //
    t_guiqueue *first = interface_outGuiQueue;
    interface_outGuiQueue = interface_outGuiQueue->gq_next;
    PD_MEMORY_FREE (first);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_closeSocket (int fd)
{
    #if PD_WINDOWS
        closesocket (fd);
    #else
        close (fd);
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_initialize (void)
{
    #if PD_WINDOWS

    WSADATA d;
    short version = MAKEWORD (2, 0);
    
    if (WSAStartup (version, &d)) { PD_BUG; return PD_ERROR; }
    
    #endif
    
    interface_inPollers = (t_fdpoll *)PD_MEMORY_GET (0);
    
    #if ! ( PD_WITH_NOGUI )
    
    interface_outGuiBuffer     = (char *)PD_MEMORY_GET (INTERFACE_GUI_BUFFER_START_SIZE);
    interface_outGuiBufferSize = INTERFACE_GUI_BUFFER_START_SIZE;
    
    #endif
}

void interface_release (void)
{
    #if ! ( PD_WITH_NOGUI )
    
    PD_MEMORY_FREE (interface_outGuiBuffer);
    interface_guiQueueRelease();
    
    #endif
    
    if (interface_inGuiReceiver) { receiver_free (interface_inGuiReceiver); }
    PD_MEMORY_FREE (interface_inPollers);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_quit (void *dummy)
{
    scheduler_needToExit();
}

#if PD_WATCHDOG

void interface_watchdog (void *dummy)
{
    if (write (interface_watchdogPipe, "\n", 1) < 1) { PD_BUG; scheduler_needToExitWithError(); }
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ! ( PD_WITH_NOGUI )

static int interface_flushBuffer (void)
{
    size_t need = interface_outGuiBufferHead - interface_outGuiBufferTail;
    
    if (need > 0) {
    //
    char *p = interface_outGuiBuffer + interface_outGuiBufferTail;
    ssize_t done = send (interface_guiSocket, (void *)p, need, 0);

    if (done < 0) { PD_BUG; scheduler_needToExitWithError(); }
    else {
        if (done == 0) { return 0; }    
        else if (done == need) { interface_outGuiBufferHead = interface_outGuiBufferTail = 0; }
        else {
            PD_ASSERT (done < need); interface_outGuiBufferTail += done;
        }
        
        return 1;
    }
    //
    }
    
    return 0;
}

static int interface_flushQueue (void)
{
    if (interface_outGuiQueue) {
    
        while (interface_outGuiQueue) {
        //
        t_guiqueue *first = interface_outGuiQueue;
        interface_outGuiQueue = interface_outGuiQueue->gq_next;
        (*first->gq_fn) (first->gq_p, first->gq_glist);
        PD_MEMORY_FREE (first);
        //
        }

        return 1;
    }
    
    return 0;
}

static int interface_flushBufferAndQueue (void)
{
    int didSomething = 0;
    
    didSomething |= interface_flushQueue();
    didSomething |= interface_flushBuffer();

    return didSomething;
}

#endif // !PD_WITH_NOGUI

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_NOGUI

void sys_vGui (char *fmt, ...)
{
}

void sys_gui (char *s)
{
}

void sys_guiFlush (void)
{

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#else

void sys_vGui (char *format, ...)
{
    int bufferWasTooSmall = 1;
    
    do {
    //
    int t;
    size_t size;
    char *dest = NULL;
    va_list ap;
    
    va_start (ap, format);
    dest = interface_outGuiBuffer + interface_outGuiBufferHead;
    size = interface_outGuiBufferSize - interface_outGuiBufferHead;
    t = vsnprintf (dest, size, format, ap);
    va_end (ap);
    
    if (t < 0) { PD_BUG; return; }
    
    if ((size_t)t >= size) { interface_increaseGuiBuffer(); }
    else {
        bufferWasTooSmall = 0;
        interface_outGuiBufferHead += t;
    }
    //
    } while (bufferWasTooSmall);
}

void sys_gui (char *s)
{
    sys_vGui ("%s", s);
}

void sys_guiFlush (void)
{
    interface_flushBufferAndQueue();
}

#endif // PD_WITH_NOGUI

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_NOGUI

int interface_pollOrFlushGui (void)
{
    return interface_monitorNonBlocking();
}

#else

int interface_pollOrFlushGui (void)
{
    return (interface_monitorNonBlocking() || interface_flushBufferAndQueue());
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error interface_fetchGuiServer (struct sockaddr_in *server)
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
#pragma mark -

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
        path_slashToBackslashIfNecessary (path, path);
        path_slashToBackslashIfNecessary (wish, wish);
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
    if (err = (path_isFileExist (path) == 0)) { PD_BUG; }
    else {
    //
    pid_t pid = fork();
    
    if (pid < 0)   { err = PD_ERROR; PD_BUG; }
    else if (!pid) {
        if (!priority_privilegeRelinquish()) {               /* Child lose setuid privileges. */
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
#pragma mark -

#if ! ( PD_WITH_NOGUI )

static t_error interface_startGui (void)
{
    t_error err = PD_ERROR_NONE;
    
    struct sockaddr_in server;
    int f = -1;
    int launch = (main_portNumber == 0);
    
    if (!launch) { err = interface_fetchGuiServer (&server); }        /* Wish first. */
    else {
    //
    if (!(err = interface_launchGui (&server, &f))) {           /* Executable first. */
        if (!(err = (listen (f, 5) < 0))) {
            socklen_t s = sizeof (struct sockaddr_in);
            err = ((interface_guiSocket = accept (f, (struct sockaddr *)&server, (socklen_t *)&s)) < 0);
        }
        PD_ASSERT (!err);
    }
    //
    }
    
    if (!err) {
        interface_inGuiReceiver = receiver_new (NULL, interface_guiSocket, NULL, NULL, 0, 0);
    }
    
    /* Initialize GUI. */
    
    if (!err) {
        t_pathlist *l = NULL;
        sys_vGui ("::initialize\n");
        for (l = path_search; l; l = pathlist_getNext (l)) {
            sys_vGui ("lappend ::var(searchPath) {%s}\n", pathlist_getFile (l));    // --
        }
    }
    
    return err;
}

#endif // PD_WITH_NOGUI

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
