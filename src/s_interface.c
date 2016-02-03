
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

#define INTERFACE_GUI_BUFFER_SIZE           8192

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _fdpoll {
    void        *fdp_p;
    int         fdp_fd;
    t_pollfn    fdp_fn;
    } t_fdpoll;

typedef struct _guiqueue {
    void                *gq_owner;
    t_glist             *gq_glist;
    t_guifn             gq_fn;
    struct _guiqueue    *gq_next;
    } t_guiqueue;

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

static int                  interface_inGuiSocket;                  /* Shared. */
static int                  interface_inPollersSize;                /* Shared. */
static int                  interface_inMaximumFileDescriptor;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char                 *interface_outGuiBuffer;                /* Shared. */
static t_guiqueue           *interface_outGuiQueueHead;             /* Shared. */

static int                  interface_outGuiBufferSize;             /* Shared. */
static int                  interface_outGuiBufferHead;             /* Shared. */
static int                  interface_outGuiBufferTail;             /* Shared. */
static int                  interface_outWatchdogPipe;              /* Shared. */
static int                  interface_outIsWaitingForPing;          /* Shared. */
static int                  interface_outBytesSinceLastPing;        /* Shared. */

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_socketPollBlocking (int microseconds)
{
    interface_pollSockets (microseconds);
}

void interface_socketPollNonBlocking (void)
{
    interface_pollSockets (0);
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

void interface_release (void)
{
    receiver_free (interface_inGuiReceiver);
    
    PD_MEMORY_FREE (interface_inPollers);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_quit (void *dummy)
{
    scheduler_needToExit();
}

void interface_ping (void *dummy)
{
    interface_outIsWaitingForPing = 0;
}

#if PD_WITH_WATCHDOG

void interface_watchdog (void *dummy)
{
    if (write (interface_outWatchdogPipe, "\n", 1) < 1) { scheduler_needToExitWithError(); PD_BUG; }
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void sys_trytogetmoreguibuf(int newsize)
{
    char *newbuf = realloc(interface_outGuiBuffer, newsize);
#if 0
    static int sizewas;
    if (newsize > 70000 && sizewas < 70000)
    {
        int i;
        for (i = interface_outGuiBufferTail; i < interface_outGuiBufferHead; i++)
            fputc(interface_outGuiBuffer[i], stderr);
    }
    sizewas = newsize;
#endif
#if 0
    fprintf(stderr, "new size %d (head %d, tail %d)\n",
        newsize, interface_outGuiBufferHead, interface_outGuiBufferTail);
#endif

        /* if realloc fails, make a last-ditch attempt to stay alive by
        synchronously writing out the existing contents.  LATER test
        this by intentionally setting newbuf to zero */
    if (!newbuf)
    {
        int bytestowrite = interface_outGuiBufferTail - interface_outGuiBufferHead;
        int written = 0;
        while (1)
        {
            int res = send(interface_inGuiSocket,
                interface_outGuiBuffer + interface_outGuiBufferTail + written, bytestowrite, 0);
            if (res < 0)
            {
                perror("pd output pipe");
                scheduler_needToExitWithError();
            }
            else
            {
                written += res;
                if (written >= bytestowrite)
                    break;
            }
        }
        interface_outGuiBufferHead = interface_outGuiBufferTail = 0;
    }
    else
    {
        interface_outGuiBufferSize = newsize;
        interface_outGuiBuffer = newbuf;
    }
}

void sys_vgui(char *fmt, ...)
{
    int msglen, bytesleft, headwas, nwrote;
    va_list ap;

    if (PD_WITH_NOGUI)
        return;
    if (!interface_outGuiBuffer)
    {
        if (!(interface_outGuiBuffer = malloc(INTERFACE_GUI_BUFFER_SIZE)))
        {
            fprintf(stderr, "Pd: couldn't allocate GUI buffer\n");
            scheduler_needToExitWithError();
        }
        interface_outGuiBufferSize = INTERFACE_GUI_BUFFER_SIZE;
        interface_outGuiBufferHead = interface_outGuiBufferTail = 0;
    }
    if (interface_outGuiBufferHead > interface_outGuiBufferSize - (INTERFACE_GUI_BUFFER_SIZE/2))
        sys_trytogetmoreguibuf(interface_outGuiBufferSize + INTERFACE_GUI_BUFFER_SIZE);
    va_start(ap, fmt);
    msglen = vsnprintf(interface_outGuiBuffer + interface_outGuiBufferHead,
        interface_outGuiBufferSize - interface_outGuiBufferHead, fmt, ap);
    va_end(ap);
    if(msglen < 0) 
    {
        fprintf(stderr, "Pd: buffer space wasn't sufficient for long GUI string\n");
        return;
    }
    if (msglen >= interface_outGuiBufferSize - interface_outGuiBufferHead)
    {
        int msglen2, newsize = interface_outGuiBufferSize + 1 +
            (msglen > INTERFACE_GUI_BUFFER_SIZE ? msglen : INTERFACE_GUI_BUFFER_SIZE);
        sys_trytogetmoreguibuf(newsize);

        va_start(ap, fmt);
        msglen2 = vsnprintf(interface_outGuiBuffer + interface_outGuiBufferHead,
            interface_outGuiBufferSize - interface_outGuiBufferHead, fmt, ap);
        va_end(ap);
        if (msglen2 != msglen) { PD_BUG; }
        if (msglen >= interface_outGuiBufferSize - interface_outGuiBufferHead)
            msglen = interface_outGuiBufferSize - interface_outGuiBufferHead;
    }
    //if (0 /*sys_debuglevel*/ & DEBUG_MESSUP)
    //    fprintf(stderr, "%s",  interface_outGuiBuffer + interface_outGuiBufferHead);
    interface_outGuiBufferHead += msglen;
    interface_outBytesSinceLastPing += msglen;
}

void sys_gui(char *s)
{
    sys_vgui("%s", s);
}

static int sys_flushtogui( void)
{
    int writesize = interface_outGuiBufferHead - interface_outGuiBufferTail, nwrote = 0;
    if (writesize > 0)
        nwrote = send(interface_inGuiSocket, interface_outGuiBuffer + interface_outGuiBufferTail, writesize, 0);

#if 0   
    if (writesize)
        fprintf(stderr, "wrote %d of %d\n", nwrote, writesize);
#endif

    if (nwrote < 0)
    {
        perror("pd-to-gui socket");
        scheduler_needToExitWithError();
    }
    else if (!nwrote)
        return (0);
    else if (nwrote >= interface_outGuiBufferHead - interface_outGuiBufferTail)
         interface_outGuiBufferHead = interface_outGuiBufferTail = 0;
    else if (nwrote)
    {
        interface_outGuiBufferTail += nwrote;
        if (interface_outGuiBufferTail > (interface_outGuiBufferSize >> 2))
        {
            memmove(interface_outGuiBuffer, interface_outGuiBuffer + interface_outGuiBufferTail,
                interface_outGuiBufferHead - interface_outGuiBufferTail);
            interface_outGuiBufferHead = interface_outGuiBufferHead - interface_outGuiBufferTail;
            interface_outGuiBufferTail = 0;
        }
    }
    return (1);
}

static int sys_flushqueue(void )
{
    const int INTERFACE_GUI_SLICE = 512;
    const int INTERFACE_GUI_BYTES = 1024;
    int wherestop = interface_outBytesSinceLastPing + INTERFACE_GUI_SLICE;
    if (wherestop + (INTERFACE_GUI_SLICE >> 1) > INTERFACE_GUI_BYTES)
        wherestop = 0x7fffffff;
    if (interface_outIsWaitingForPing)
        return (0);
    if (!interface_outGuiQueueHead)
        return (0);
    while (1)
    {
        if (interface_outBytesSinceLastPing >= INTERFACE_GUI_BYTES)
        {
            sys_gui("::ping\n");
            interface_outBytesSinceLastPing = 0;
            interface_outIsWaitingForPing = 1;
            return (1);
        }
        if (interface_outGuiQueueHead)
        {
            t_guiqueue *headwas = interface_outGuiQueueHead;
            interface_outGuiQueueHead = headwas->gq_next;
            (*headwas->gq_fn)(headwas->gq_owner, headwas->gq_glist);
            PD_MEMORY_FREE(headwas);
            if (interface_outBytesSinceLastPing >= wherestop)
                break;
        }
        else break;
    }
    sys_flushtogui();
    return (1);
}

    /* flush output buffer and update queue to gui in small time slices */
static int sys_poll_togui(void) /* returns 1 if did anything */
{
    if (PD_WITH_NOGUI)
        return (0);
        /* in case there is stuff still in the buffer, try to flush it. */
    sys_flushtogui();
        /* if the flush wasn't complete, wait. */
    if (interface_outGuiBufferHead > interface_outGuiBufferTail)
        return (0);
    
        /* check for queued updates */
    if (sys_flushqueue())
        return (1);
    
    return (0);
}

void sys_queuegui(void *client, t_glist *glist, t_guifn f)
{
    t_guiqueue **gqnextptr, *gq;
    if (!interface_outGuiQueueHead)
        gqnextptr = &interface_outGuiQueueHead;
    else
    {
        for (gq = interface_outGuiQueueHead; gq->gq_next; gq = gq->gq_next)
            if (gq->gq_owner == client)
                return;
        if (gq->gq_owner == client)
            return;
        gqnextptr = &gq->gq_next;
    }
    gq = PD_MEMORY_GET(sizeof(*gq));
    gq->gq_next = 0;
    gq->gq_owner = client;
    gq->gq_glist = glist;
    gq->gq_fn = f;
    gq->gq_next = 0;
    *gqnextptr = gq;
}

void sys_unqueuegui(void *client)
{
    t_guiqueue *gq, *gq2;
    while (interface_outGuiQueueHead && interface_outGuiQueueHead->gq_owner == client)
    {
        gq = interface_outGuiQueueHead;
        interface_outGuiQueueHead = interface_outGuiQueueHead->gq_next;
        PD_MEMORY_FREE(gq);
    }
    if (!interface_outGuiQueueHead)
        return;
    for (gq = interface_outGuiQueueHead; gq2 = gq->gq_next; gq = gq2)
        if (gq2->gq_owner == client)
    {
        gq->gq_next = gq2->gq_next;
        PD_MEMORY_FREE(gq2);
        break;
    }
}

int sys_pollgui(void)
{
    return (interface_pollSockets(0) || sys_poll_togui());
}

int sys_startgui(const char *libdir)
{
    char *interface_commandToLaunchGUI;
    char cmdbuf[4*PD_STRING];
    struct sockaddr_in server;
    int msgsock;
    char buf[15];
    int len = sizeof(server);
    int ntry = 0, portno = INTERFACE_PORT;
    int xsock = -1, dumbo = -1;
#ifdef _WIN32
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#else
    int stdinpipe[2];
    pid_t childpid;
#endif /* _WIN32 */

    interface_inPollers = (t_fdpoll *)PD_MEMORY_GET(0);
    interface_inPollersSize = 0;

#ifdef _WIN32
    if (WSAStartup(version, &nobby)) PD_BUG;
#endif /* _WIN32 */

    if (PD_WITH_NOGUI) { }
    else if (main_portNumber)  /* GUI exists and sent us a port number */
    {
        struct sockaddr_in server;
        struct hostent *hp;
#ifdef __APPLE__
            /* interface_inGuiSocket might be 1 or 2, which will have offensive results
            if somebody writes to stdout or stderr - so we just open a few
            files to try to fill fds 0 through 2.  (I tried using dup()
            instead, which would seem the logical way to do this, but couldn't
            get it to work.) */
        int burnfd1 = open("/dev/null", 0), burnfd2 = open("/dev/null", 0),
            burnfd3 = open("/dev/null", 0);
        if (burnfd1 > 2)
            close(burnfd1);
        if (burnfd2 > 2)
            close(burnfd2);
        if (burnfd3 > 2)
            close(burnfd3);
#endif
        /* create a socket */
        interface_inGuiSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (interface_inGuiSocket < 0)
            PD_BUG;
        
        /* connect socket using hostname provided in command line */
        server.sin_family = AF_INET;

        hp = gethostbyname(INTERFACE_LOCALHOST);

        if (hp == 0)
        {
            fprintf(stderr,
                "localhost not found (inet protocol not installed?)\n");
            return (1);
        }
        memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

        /* assign client port number */
        server.sin_port = htons((unsigned short)main_portNumber);

            /* try to connect */
        if (connect(interface_inGuiSocket, (struct sockaddr *) &server, sizeof (server))
            < 0)
        {
            PD_BUG;
            return (1);
        }
    }
    else    /* default behavior: start up the GUI ourselves. */
    {
#ifdef _WIN32
        char scriptbuf[PD_STRING+30], wishbuf[PD_STRING+30], portbuf[80];
        int spawnret;
        char intarg;
#else
        int intarg;
#endif

        /* create a socket */
        xsock = socket(AF_INET, SOCK_STREAM, 0);
        if (xsock < 0) PD_BUG;
        intarg = 1;
        if (setsockopt(xsock, IPPROTO_TCP, TCP_NODELAY,
            &intarg, sizeof(intarg)) < 0)
#ifndef _WIN32
                post("setsockopt (TCP_NODELAY) failed\n")
#endif
                    ;
        
        
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;

        /* assign server port number */
        server.sin_port =  htons((unsigned short)portno);

        /* name the socket */
        while (bind(xsock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
#ifdef _WIN32
            int err = WSAGetLastError();
#else
            int err = errno;
#endif
            if ((ntry++ > 20) || (err != EADDRINUSE))
            {
                perror("bind");
                fprintf(stderr,
                    "Pd needs your machine to be configured with\n");
                fprintf(stderr,
                  "'networking' turned on (see Pd's html doc for details.)\n");
                return (1);
            }
            portno++;
            server.sin_port = htons((unsigned short)(portno));
        }

        if (0) fprintf(stderr, "port %d\n", portno);


#ifndef _WIN32
        if (!interface_commandToLaunchGUI)
        {
#ifdef __APPLE__
            int i;
            struct stat statbuf;
            glob_t glob_buffer;
            char *homedir = getenv("HOME");
            char embed_glob[FILENAME_MAX];
            char home_filename[FILENAME_MAX];
            char *wish_paths[10] = {
                "(did not find a home directory)",
                "/Applications/Utilities/Wish.app/Contents/MacOS/Wish",
                "/Applications/Utilities/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/Applications/Wish.app/Contents/MacOS/Wish",
                "/Applications/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/Library/Frameworks/Tk.framework/Resources/Wish.app/Contents/MacOS/Wish",
                "/Library/Frameworks/Tk.framework/Resources/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/System/Library/Frameworks/Tk.framework/Resources/Wish.app/Contents/MacOS/Wish",
                "/System/Library/Frameworks/Tk.framework/Resources/Wish Shell.app/Contents/MacOS/Wish Shell",
                "/usr/bin/wish"
            };
            /* this glob is needed so the Wish executable can have the same
             * filename as the Pd.app, i.e. 'Pd-0.42-3.app' should have a Wish
             * executable called 'Pd-0.42-3.app/Contents/MacOS/Pd-0.42-3' */
            sprintf(embed_glob, "%s/../MacOS/Pd*", libdir);
            glob_buffer.gl_matchc = 1; /* we only need one match */
            glob(embed_glob, GLOB_LIMIT, NULL, &glob_buffer);
            /* If we are using a copy of Wish embedded in the Pd.app, then it
             * will automatically load ui_main.tcl if that embedded Wish can
             * find ../Resources/Scripts/AppMain.tcl, then Wish doesn't want
             * to receive the ui_main.tcl as an argument.  Otherwise it needs
             * to know how to find ui_main.tcl */
            if (glob_buffer.gl_pathc > 0)
                sprintf(cmdbuf, "\"%s\" %d\n", glob_buffer.gl_pathv[0], portno);
            else
            {
                sprintf(home_filename,
                        "%s/Applications/Wish.app/Contents/MacOS/Wish",homedir);
                wish_paths[0] = home_filename;
                for(i=0; i<10; i++)
                {
                    if (0)
                        fprintf(stderr, "Trying Wish at \"%s\"\n", wish_paths[i]);
                    if (stat(wish_paths[i], &statbuf) >= 0)
                        break;
                }
                sprintf(cmdbuf, "\"%s\" \"%s/%sui_main.tcl\" %d\n", 
                        wish_paths[i], libdir, PD_TCL_DIRECTORY, portno);
            }
#else /* __APPLE__ */
            /* sprintf the wish command with needed environment variables.
            For some reason the wish script fails if HOME isn't defined so
            if necessary we put that in here too. */
            sprintf(cmdbuf,
  "TCL_LIBRARY=\"%s/lib/tcl/library\" TK_LIBRARY=\"%s/lib/tk/library\"%s \
  wish \"%s/" PD_TCL_DIRECTORY "/ui_main.tcl\" %d\n",
                 libdir, libdir, (getenv("HOME") ? "" : " HOME=/tmp"),
                    libdir, portno);
#endif /* __APPLE__ */
            interface_commandToLaunchGUI = cmdbuf;
        }

        childpid = fork();
        if (childpid < 0)
        {
            if (errno) perror("sys_startgui");
            else fprintf(stderr, "sys_startgui failed\n");
            return (1);
        }
        else if (!childpid)                     /* we're the child */
        {
            setuid(getuid());          /* lose setuid priveliges */
#ifndef __APPLE__
// TODO this seems unneeded on any platform hans@eds.org
                /* the wish process in Unix will make a wish shell and
                    read/write standard in and out unless we close the
                    file descriptors.  Somehow this doesn't make the MAC OSX
                        version of Wish happy...*/
            if (pipe(stdinpipe) < 0)
                PD_BUG;
            else
            {
                if (stdinpipe[0] != 0)
                {
                    close (0);
                    dup2(stdinpipe[0], 0);
                    close(stdinpipe[0]);
                }
            }
#endif /* NOT __APPLE__ */
            execl("/bin/sh", "sh", "-c", interface_commandToLaunchGUI, (char*)0);
            perror("pd: exec");
            fprintf(stderr, "Perhaps tcl and tk aren't yet installed?\n");
            _exit(1);
       }
#else /* NOT _WIN32 */
        /* fprintf(stderr, "%s\n", libdir); */
        
        strcpy(scriptbuf, "\"");
        strcat(scriptbuf, libdir);
        strcat(scriptbuf, "/" PD_TCL_DIRECTORY "ui_main.tcl\"");
        sys_bashfilename(scriptbuf, scriptbuf);
        
        sprintf(portbuf, "%d", portno);

        strcpy(wishbuf, libdir);
        strcat(wishbuf, "/" PD_BIN_DIRECTORY PD_EXE_WISH);
        sys_bashfilename(wishbuf, wishbuf);
        
        spawnret = _spawnl(P_NOWAIT, wishbuf, PD_EXE_WISH, scriptbuf, portbuf, 0);
        if (spawnret < 0)
        {
            perror("spawnl");
            fprintf(stderr, "%s: couldn't load TCL\n", wishbuf);
            return (1);
        }

#endif /* NOT _WIN32 */
    }

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

    sprintf(cmdbuf, "%s/bin/pdwatchdog", libdir);
    if (PD_WITH_REALTIME)
    {
        struct stat statbuf;
        if (stat(cmdbuf, &statbuf) < 0)
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
                perror("sys_startgui");
            else fprintf(stderr, "sys_startgui failed\n");
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

            if (0) fprintf(stderr, "%s\n", cmdbuf);
            execl("/bin/sh", "sh", "-c", cmdbuf, (char*)0);
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
            interface_outWatchdogPipe = pipe9[1];
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

        interface_inGuiSocket = accept(xsock, (struct sockaddr *) &server, 
            (socklen_t *)&len);
#ifdef OOPS
        interface_socketClose(xsock);
#endif
        if (interface_inGuiSocket < 0) PD_BUG;
        if (0)
            fprintf(stderr, "... connected\n");
        interface_outGuiBufferHead = interface_outGuiBufferTail = 0;
    }
    if (!PD_WITH_NOGUI)
    {
        char buf[256], buf2[256];
        interface_inGuiReceiver = receiver_new (NULL, interface_inGuiSocket, NULL, NULL, 0);

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
        sys_vgui("::initialize %s %s\n",
                 buf, buf2); 
                 /* */
                 /* */
        sys_vgui("set ::var(apiAudio) %d\n", sys_audioapi);
    }
    return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
