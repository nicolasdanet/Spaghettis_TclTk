
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
#include "g_canvas.h"

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
    t_guifn             gq_fn;
    struct _guiqueue    *gq_next;
    } t_guiqueue;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_rootDirectory;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int  sys_audioapi;
extern int  main_portNumber;

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
static int                  interface_watchdogPipe;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int interface_pollSockets (int microseconds)
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
    PD_ABORT (newSize > INTERFACE_GUI_BUFFER_ABORT_SIZE);
    interface_outGuiBuffer = PD_MEMORY_RESIZE (interface_outGuiBuffer, oldSize, newSize);
    interface_outGuiBufferSize = newSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int interface_socketPollBlocking (int microseconds)
{
    return interface_pollSockets (microseconds);
}

int interface_socketPollNonBlocking (void)
{
    return interface_pollSockets (0);
}

void interface_socketAddCallback (int fd, t_pollfn fn, void *ptr)
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

void interface_socketRemoveCallback (int fd)
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

void interface_socketClose (int fd)
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

void interface_guiQueueAddIfNotAlreadyThere (void *owner, t_glist *glist, t_guifn f)
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

void interface_initialize (void)
{
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

#if PD_WITH_WATCHDOG

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

void sys_gui (char *s)
{
}

void sys_vGui (char *fmt, ...)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

void sys_gui (char *s)
{
    sys_vGui ("%s", s);
}

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

#endif // PD_WITH_NOGUI

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_NOGUI

int interface_pollSocketsOrFlushGui (void)
{
    return (interface_socketPollNonBlocking());
}

#else

int interface_pollSocketsOrFlushGui (void)
{
    return (interface_socketPollNonBlocking() || interface_flushBufferAndQueue());
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error interface_fetchGui (struct sockaddr_in *server)
{
    struct hostent *host = gethostbyname (INTERFACE_LOCALHOST);
    t_error err = ((interface_guiSocket = socket (AF_INET, SOCK_STREAM, 0)) < 0);
    
    PD_ASSERT (!err);
    
    if (host && !err) {
        server->sin_family = AF_INET;
        server->sin_port = htons ((unsigned short)main_portNumber);
        memcpy ((char *)&server->sin_addr, (char *)host->h_addr, host->h_length);
        err |= connect (interface_guiSocket, (struct sockaddr *)server, sizeof (struct sockaddr_in));
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
    err |= string_add (path, PD_STRING, main_rootDirectory->s_name);
    err |= string_add (path, PD_STRING, "/" PD_TCL_DIRECTORY "ui_main.tcl\"");
    
    err |= string_sprintf (port, PD_STRING, "%d", port);
    
    err |= string_copy (wish, PD_STRING, "\"");
    err |= string_add (wish, PD_STRING, main_rootDirectory->s_name);
    err |= string_add (wish, PD_STRING, "/" PD_BIN_DIRECTORY "wish85.exe\"");
    
    if (!err) {
        sys_bashfilename (path, path);
        sys_bashfilename (wish, wish);
        err |= ((spawned = _spawnl (P_NOWAIT, wish, "wish85.exe", path, port, 0)) < 0);
    }
    
    PD_ASSERT (!err);
    
    return err;
}

#else

static t_error interface_launchGuiSpawnProcess (void) 
{
    t_error err = PD_ERROR_NONE;
    
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
    
    err |= string_sprintf (command, PD_STRING, 
            "\"%s\" \"%s/%sui_main.tcl\" %d\n", 
            wish[i], 
            main_rootDirectory->s_name, 
            PD_TCL_DIRECTORY, 
            main_portNumber);

#else
    
    err |= string_sprintf (command, PD_STRING, 
            "TCL_LIBRARY=\"%s/lib/tcl/library\" TK_LIBRARY=\"%s/lib/tk/library\"%s \
            wish \"%s/" PD_TCL_DIRECTORY "/ui_main.tcl\" %d\n",
            main_rootDirectory->s_name,
            main_rootDirectory->s_name, 
            (getenv ("HOME") ? "" : " HOME=/tmp"),
            main_rootDirectory->s_name, 
            main_portNumber);
                    
#endif // PD_APPLE

    post_log ("%s", command);
    
    if (!err) {
    //
    pid_t pid = fork();
    
    if (pid < 0)   { err = PD_ERROR; PD_BUG; }
    else if (!pid) {
        setuid (getuid());                                              /* Lose setuid privileges. */
        execl ("/bin/sh", "sh", "-c", command, NULL); _exit (1);
    }
    //
    }
       
    return err;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error interface_launchGuiSocket (struct sockaddr_in *server)
{
    int f = -1;
    t_error err = ((f = socket (AF_INET, SOCK_STREAM, 0)) < 0);
        
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

    if ((n++ > 20) || (e != EADDRINUSE)) { err |= PD_ERROR; PD_BUG; break; } 
    else {
        server->sin_port = htons ((unsigned short)++main_portNumber);
    }
    //
    }
    //
    }
    
    PD_ASSERT (!err);
    
    return err;
}

static t_error interface_launchGui (struct sockaddr_in *server)
{
    t_error err = PD_ERROR_NONE;
    
    if (!(err |= interface_launchGuiSocket (server))) { err |= interface_launchGuiSpawnProcess(); }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error interface_start (void)
{
    t_error err = PD_ERROR_NONE;
    
    char command[PD_STRING];
    struct sockaddr_in server;
    
    socklen_t length = sizeof (struct sockaddr_in);
    
    int msgsock;
    char buf[15];

    int ntry = 0, portno = INTERFACE_PORT;
    int xsock = -1, dumbo = -1;
    
#ifdef _WIN32
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#else
    int stdinpipe[2];
    pid_t childpid;
#endif /* _WIN32 */

#ifdef _WIN32
    if (WSAStartup(version, &nobby)) PD_BUG;
#endif /* _WIN32 */

    #if !PD_WITH_NOGUI
    
    if (main_portNumber) { err = interface_fetchGui (&server); }            /* Wish first. */
    else {
        err = interface_launchGui (&server);                                /* Binary first. */
    }
    
    #endif 

    if (err) { return err; }
    
#if defined(__linux__) || defined(__FreeBSD_kernel__)
        /* now that we've spun off the child process we can promote
        our process's priority, if we can and want to.  If not specfied
        (-1), we assume real-time was wanted.  Afterward, just in case
        someone made Pd setuid in order to get permission to do this,
        unset setuid and lose root priveliges after doing this.  Starting
        in Linux 2.6 this is accomplished by putting lines like:
                @audio - rtprio 99
                @audio - memlock unlimited
        in the system limits file, perhaps /etc/limits.conf or
        /etc/security/limits.conf */

    sprintf(command, "%s/bin/pdwatchdog", root);
    if (PD_WITH_REALTIME)
    {
        struct stat statbuf;
        if (stat(command, &statbuf) < 0)
        {
            PD_BUG;
            PD_ABORT (1);
        }
    }
    else if (0)
        post("not setting real-time priority");
    
    if (PD_WITH_REALTIME)
    {
            /* To prevent lockup, we fork off a watchdog process with
            higher real-time priority than ours.  The GUI has to send
            a stream of ping messages to the watchdog THROUGH the Pd
            process which has to pick them up from the GUI and forward
            them.  If any of these things aren't happening the watchdog
            starts sending "stop" and "cont" signals to the Pd process
            to make it timeshare with the rest of the system.  (Version
            0.33P2 : if there's no GUI, the watchdog pinging is done
            from the scheduler idle routine in this process instead.) */
        int pipe9[2], watchpid;

        if (pipe(pipe9) < 0)
        {
            setuid(getuid());      /* lose setuid priveliges */
            PD_BUG;
            return (1);
        }
        watchpid = fork();
        if (watchpid < 0)
        {
            setuid(getuid());      /* lose setuid priveliges */
            if (errno)
                perror("interface_start");
            else fprintf(stderr, "interface_start failed\n");
            return (1);
        }
        else if (!watchpid)             /* we're the child */
        {
            sys_setRealTimePolicy(1);
            setuid(getuid());      /* lose setuid priveliges */
            if (pipe9[1] != 0)
            {
                dup2(pipe9[0], 0);
                close(pipe9[0]);
            }
            close(pipe9[1]);

            if (0) fprintf(stderr, "%s\n", command);
            execl("/bin/sh", "sh", "-c", command, (char*)0);
            perror("pd: exec");
            _exit(1);
        }
        else                            /* we're the parent */
        {
            sys_setRealTimePolicy(0);
            setuid(getuid());      /* lose setuid priveliges */
            close(pipe9[0]);
                /* set close-on-exec so that watchdog will see an EOF when we
                close our copy - otherwise it might hang waiting for some
                stupid child process (as seems to happen if jackd auto-starts
                for us.) */
            fcntl(pipe9[1], F_SETFD, FD_CLOEXEC);
            interface_watchdogPipe = pipe9[1];
                /* We also have to start the ping loop in the GUI;
                this is done later when the socket is open. */
        }
    }

    setuid(getuid());          /* lose setuid priveliges */
#endif /* __linux__ */

#ifdef _WIN32
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        fprintf(stderr, "pd: couldn't set high priority class\n");
#endif
#ifdef __APPLE__
    if (PD_WITH_REALTIME)
    {
        struct sched_param param;
        int policy = SCHED_RR;
        int err;
        param.sched_priority = 80; /* adjust 0 : 100 */

        err = pthread_setschedparam(pthread_self(), policy, &param);
        if (err)
            post("warning: high priority scheduling failed\n");
    }
#endif /* __APPLE__ */

    if (!PD_WITH_NOGUI && !main_portNumber)
    {
        if (0)
            fprintf(stderr, "Waiting for connection request... \n");
        if (listen(xsock, 5) < 0) PD_BUG;

        interface_guiSocket = accept (xsock, (struct sockaddr *) &server, (socklen_t *)&length);
#ifdef OOPS
        interface_socketClose(xsock);
#endif
        if (interface_guiSocket < 0) PD_BUG;
        if (0)
            fprintf(stderr, "... connected\n");
        interface_outGuiBufferHead = interface_outGuiBufferTail = 0;
    }
    if (!PD_WITH_NOGUI)
    {
        char buf[256], buf2[256];
        interface_inGuiReceiver = receiver_new (NULL, interface_guiSocket, NULL, NULL, 0);

            /* here is where we start the pinging. */
#if defined(__linux__) || defined(__FreeBSD_kernel__)
        if (PD_WITH_REALTIME)
            sys_gui("::watchdog\n");
#endif
        sys_get_audio_apis(buf);
        sys_get_midi_apis(buf2);
        sys_set_searchpath();     /* tell GUI about path and startup flags */
        sys_set_extrapath();
        sys_set_startup();
                           /* ... and about font, medio APIS, etc */
        sys_vGui("::initialize %s %s\n",
                 buf, buf2); 
                 /* */
                 /* */
        sys_vGui("set ::var(apiAudio) %d\n", sys_audioapi);
    }
    return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
